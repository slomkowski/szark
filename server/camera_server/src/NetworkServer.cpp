#include "NetworkServer.hpp"
#include "Configuration.hpp"

#include <minijson_reader.hpp>
#include <minijson_writer.hpp>

#include <boost/format.hpp>

using namespace std;
using namespace boost;
using namespace camera;

namespace camera {
    const int RECEIVED_DATA_MAX_LENGTH = 256;
}

WALLAROO_REGISTER(NetworkServer);

camera::NetworkServer::NetworkServer()
        : logger(log4cpp::Category::getInstance("NetworkServer")),
          config("config", RegistrationToken()),
          imageSource("imageSource", RegistrationToken()),
          jpegEncoder("jpegEncoder", RegistrationToken()),
          ioServiceProvider("ioServiceProvider", RegistrationToken()),
          port(0) { }

camera::NetworkServer::NetworkServer(int port)
        : logger(log4cpp::Category::getInstance("NetworkServer")),
          config("config", RegistrationToken()),
          imageSource("imageSource", RegistrationToken()),
          jpegEncoder("jpegEncoder", RegistrationToken()),
          ioServiceProvider("ioServiceProvider", RegistrationToken()),
          port(port) { }

void NetworkServer::Init() {
    using boost::asio::ip::udp;
    udpSocket.reset(new udp::socket(ioServiceProvider->getIoService()));

    if (port == 0) {
        port = config->getInt("NetworkServer.port");
    }

    recvBuffer.reset(new char[RECEIVED_DATA_MAX_LENGTH]);

    logger.notice("Opening listener socket with port %u.", port);

    system::error_code err;
    bool ipv6enabled = config->getBool("NetworkServer.enable_ipv6");

    if (ipv6enabled) {
        udpSocket->open(udp::v6());
        udpSocket->bind(udp::endpoint(udp::v6(), port), err);
    } else {
        udpSocket->open(udp::v4());
        udpSocket->bind(udp::endpoint(udp::v4(), port), err);
    }

    if (err) {
        throw NetworkException("error at binding socket: " + err.message());
    }

    doReceive();

    logger.notice("Started UDP listener on port %u%s.", port, ipv6enabled ? " (IPv6 enabled)" : "");

    logger.notice("Instance created.");
}

camera::NetworkServer::~NetworkServer() {
    logger.notice("Instance destroyed.");
}

void camera::NetworkServer::doReceive() {
    udpSocket->async_receive_from(
            asio::buffer(recvBuffer.get(), RECEIVED_DATA_MAX_LENGTH),
            endpoint,
            [this](boost::system::error_code ec, std::size_t bytesReceived) {
                if (ec) {
                    throw NetworkException(
                            (format("error at receiving request: %s") % ec.message()).str());
                }

                long serial = 0;
                bool drawHud = false;
                int quality = camera::DEFAULT_JPEG_QUALITY;

                try {
                    minijson::buffer_context ctx(recvBuffer.get(), RECEIVED_DATA_MAX_LENGTH);
                    minijson::parse_object(ctx, [&](const char *k, minijson::value v) {
                        minijson::dispatch(k)
                        << "serial" >> [&] { serial = v.as_long(); }
                        << "drawHud" >> [&] { drawHud = v.as_bool(); }
                        << "quality" >> [&] { quality = v.as_long(); };
                    });

                    quality = std::min(100, quality);
                    quality = std::max(5, quality);

                    logger.info("Received request from %s: serial %d, %s HUD, quality: %d.",
                                endpoint.address().to_string().c_str(),
                                serial,
                                (drawHud ? "with" : "without"),
                                quality);

                    auto img = imageSource->getImage(drawHud);

                    std::array<unsigned char, 0x400000> buffer;

                    std::stringstream headerStream;
                    minijson::object_writer writer(headerStream);
                    writer.write("serial", serial);
                    writer.write("drawHud", drawHud);
                    writer.write("quality", quality);
                    writer.close();

                    std::string header = headerStream.str();

                    std::memcpy(buffer.data(), header.c_str(), header.size());

                    buffer[header.size()] = 0;

                    auto encodedLength = jpegEncoder->encodeImage(img, buffer.data() + header.size() + 1,
                                                                  buffer.size() - header.size() - 1, quality);

                    logger.debug("JPEG file length: %d B.", encodedLength);
                    auto packetLength = header.size() + 1 + encodedLength;

                    auto sentBytes = udpSocket->send_to(asio::buffer(buffer.data(), packetLength), endpoint);

                    if (sentBytes != packetLength) {
                        logger.error("Not whole packet sent (%u < %u).", sentBytes, packetLength);
                    } else {
                        logger.info("Sent packet (%u B).", packetLength);
                    }

                } catch (boost::system::system_error &err) {
                    logger.error("send_to error: %s", err.what());
                } catch (minijson::parse_error &exp) {
                    logger.error("Malformed request error: %s", exp.what());
                }

                doReceive();
            });
}
