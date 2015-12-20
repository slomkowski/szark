#include "NetServer.hpp"

#include <boost/format.hpp>


constexpr int MAX_PACKET_SIZE = 1200;

using namespace std;
using namespace boost;
using namespace boost::asio;
using namespace processing;
using boost::asio::ip::udp;

WALLAROO_REGISTER(NetServer);

processing::NetServer::NetServer()
        : logger(log4cpp::Category::getInstance("NetServer")),
          config("config", RegistrationToken()),
          reqQueuer("requestQueuer", RegistrationToken()),
          ioServiceProvider("ioServiceProvider", RegistrationToken()),
          udpPort(0) { }

processing::NetServer::NetServer(unsigned int port)
        : logger(log4cpp::Category::getInstance("NetServer")),
          config("config", RegistrationToken()),
          reqQueuer("requestQueuer", RegistrationToken()),
          ioServiceProvider("ioServiceProvider", RegistrationToken()),
          udpPort(port) { }

void processing::NetServer::Init() {
    buff.reset(new char[MAX_PACKET_SIZE]);
    udpSocket.reset(new ip::udp::socket(ioServiceProvider->getIoService()));

    if (udpPort == 0) {
        udpPort = config->getInt("NetServer.port");
    }

    logger.notice("Opening listener socket with port %u.", udpPort);

    system::error_code err;
    udpSocket->open(udp::v4());
    udpSocket->bind(udp::endpoint(udp::v4(), udpPort), err);
    if (err) {
        throw NetException("error at binding socket: " + err.message());
    }

    doReceive();

    logger.notice("Started UDP listener on port %u.", udpPort);

    logger.notice("Instance created.");
}

void processing::NetServer::doReceive() {
    udpSocket->async_receive_from(asio::buffer(buff.get(), MAX_PACKET_SIZE), recvSenderEndpoint,
                                  [this](system::error_code ec, size_t bytes_recvd) {
                                      if (!ec && bytes_recvd > 0) {
                                          logger.info("Received %d bytes from %s.",
                                                      bytes_recvd, recvSenderEndpoint.address().to_string().c_str());

                                          if (recvSenderEndpoint.protocol() != udp::v4()) {
                                              logger.error("Received address is not an IPv4 address: " +
                                                           recvSenderEndpoint.address().to_string());
                                          } else {
                                              auto msg = string(buff.get(), 0, bytes_recvd);
                                              logger.debug(string("Received data: ") + msg);

                                              long id = reqQueuer->addRequest(msg, recvSenderEndpoint.address());

                                              if (id != INVALID_MESSAGE) {
                                                  logger.debug("Adding key %ld to senders map.", id);

                                                  sendersMap[id] = recvSenderEndpoint;

                                                  logger.debug("Senders map contains now %d keys.", sendersMap.size());
                                              }

                                              namespace ph = std::placeholders;
                                              reqQueuer->setResponseSender(
                                                      bind(&NetServer::sendResponse, this, ph::_1, ph::_2, ph::_3));
                                              reqQueuer->setRejectedRequestRemover(
                                                      bind(&NetServer::removeFromRequestMap, this, ph::_1));
                                          }
                                      } else {
                                          logger.error("Error when receiving packet (%u bytes): %s.", bytes_recvd,
                                                       ec.message().c_str());
                                      }
                                      doReceive();
                                  });
}

void processing::NetServer::sendResponse(long id, std::string response, bool transmit) {
    const auto senderEndpoint = sendersMap.find(id);

    //TODO tu się wyrżnęło  senders map doesn't contain key: 22351.
    //TODO wywala się, jak zalewa się pakietami z instrukcją - wywalić wait w doInBackground controlUpdater w cliencie
    if (senderEndpoint == sendersMap.end()) {
        throw NetException((format("senders map doesn't contain key: %ld.") % id).str());
    }

    if (transmit == true) {
        udp::endpoint endpoint = senderEndpoint->second;

        unsigned int responseLength = response.length();

        logger.info("Sending response (length %d) to %s.",
                    responseLength, endpoint.address().to_string().c_str());

        logger.debug(string("Sending data: ") + response);

        udpSocket->async_send_to(asio::buffer(response), endpoint,
                                 [responseLength](system::error_code ec, size_t bytes_sent) {
                                     if (ec) {
                                         throw NetException("error when sending response: " + ec.message());
                                     }
                                     if (bytes_sent != responseLength) {
                                         throw NetException(
                                                 (format("wrong amount of data sent: %u instead of %u") % bytes_sent %
                                                  responseLength).str());
                                     }
                                 });
    }

    removeFromRequestMap(id);
}

void processing::NetServer::removeFromRequestMap(long id) {
    ioServiceProvider->getIoService().post([id, this]() {
        logger.debug("Removing key %ld from senders map.", id);
        sendersMap.erase(id);
        logger.debug("Senders map contains now %d keys.", sendersMap.size());
    });
}

processing::NetServer::~NetServer() {
    logger.notice("Instance destroyed.");
}
