#include "NetworkServer.hpp"
#include "Configuration.hpp"

#include <boost/format.hpp>

using namespace std;
using namespace boost;
using namespace camera;

namespace camera {
    const int RECEIVED_DATA_MAX_LENGTH = 10;
}

WALLAROO_REGISTER(NetworkServer);

camera::NetworkServer::NetworkServer()
        : logger(log4cpp::Category::getInstance("NetworkServer")),
          config("config", RegistrationToken()),
          imageSource("imageSource", RegistrationToken()),
          jpegEncoder("jpegEncoder", RegistrationToken()),
          ioService(),
          port(0),
          udpSocket(ioService) {
}

camera::NetworkServer::NetworkServer(int port)
        : logger(log4cpp::Category::getInstance("NetworkServer")),
          config("config", RegistrationToken()),
          imageSource("imageSource", RegistrationToken()),
          jpegEncoder("jpegEncoder", RegistrationToken()),
          ioService(),
          port(port),
          udpSocket(ioService) {
}

void NetworkServer::Init() {
    using boost::asio::ip::udp;

    if (port == 0) {
        port = config->getInt("NetworkServer.port");
    }

    recvBuffer.reset(new char[RECEIVED_DATA_MAX_LENGTH]);

    logger.notice((format("Opening listener socket with port %u.") % port).str());

    system::error_code err;
    udpSocket.open(udp::v4());
    udpSocket.bind(udp::endpoint(udp::v4(), port), err);
    if (err) {
        throw NetworkException("error at binding socket: " + err.message());
    }

    doReceive();

    logger.notice("Instance created.");
}

camera::NetworkServer::~NetworkServer() {
    logger.notice("Instance destroyed.");
}

void camera::NetworkServer::run() {
    logger.notice("Starting UDP listener.");
    ioService.run();
}

void camera::NetworkServer::doReceive() {
    udpSocket.async_receive_from(
            asio::buffer(recvBuffer.get(), RECEIVED_DATA_MAX_LENGTH),
            endpoint,
            [this](boost::system::error_code ec, std::size_t bytesReceived) {
                if (ec) {
                    throw NetworkException(
                            (format("error at receiving request: %s") % ec.message()).str());
                }

                bool drawHud = string(recvBuffer.get(), bytesReceived) == "HUD";

                logger.info((format("Received request %s HUD.") %
                             (drawHud ? "with" : "without")).str());

                auto img = imageSource->getImage(drawHud);

                std::array<unsigned char, 0x200000> buffer;

                auto encodedLength = jpegEncoder->encodeImage(img, buffer.data(), buffer.size());

                auto sentBytes = udpSocket.send_to(asio::buffer(buffer.data(), encodedLength), endpoint);
                if (sentBytes != encodedLength) {
                    throw NetworkException(
                            (format("not whole file sent (%u < %u)") % sentBytes % encodedLength).str());
                }

                logger.info((format("Sent file (%u bytes).") % encodedLength).str());

                doReceive();
            });
}

