#include "NetworkServer.hpp"
#include "Configuration.hpp"

#include "utils.hpp"

#include <minijson_reader.hpp>
#include <minijson_writer.hpp>

#include <boost/format.hpp>

#include <cstdlib>

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

    if (posix_memalign(reinterpret_cast<void **>(&sendBuffer), 32, SEND_BUFFER_SIZE) != 0
        or posix_memalign(reinterpret_cast<void **>(&sendImgBuffer), 32, SEND_BUFFER_SIZE) != 0) {
        throw NetworkException("cannot allocate buffer");
    }

    doReceive();

    logger.notice("Started UDP listener on port %u%s.", port, ipv6enabled ? " (IPv6 enabled)" : "");

    logger.notice("Instance created.");
}

camera::NetworkServer::~NetworkServer() {
    if (sendBuffer != nullptr) {
        free(sendBuffer);
    }

    if (sendImgBuffer != nullptr) {
        free(sendImgBuffer);
    }

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
                string videoInput;
                string receivedTimestamp = common::utils::getTimestamp();
                string sendTimestamp = common::utils::getTimestamp();

                try {
                    minijson::buffer_context ctx(recvBuffer.get(), RECEIVED_DATA_MAX_LENGTH);
                    minijson::parse_object(ctx, [&](const char *k, minijson::value v) {
                        minijson::dispatch(k)
                        << "serial" >> [&] { serial = v.as_long(); }
                        << "input" >> [&] { videoInput = v.as_string(); }
                        << "drawHud" >> [&] { drawHud = v.as_bool(); }
                        << "quality" >> [&] { quality = v.as_long(); }
                        << "tss" >> [&] { sendTimestamp = v.as_string(); };
                    });

                    quality = std::min(100, quality);
                    quality = std::max(5, quality);

                    logger.info("Received request from %s: serial %d, input: %s, %s HUD, quality: %d.",
                                endpoint.address().to_string().c_str(),
                                serial,
                                videoInput,
                                (drawHud ? "with" : "without"),
                                quality);

                    stringstream headerStream;
                    minijson::object_writer writer(headerStream);
                    writer.write("serial", serial);
                    writer.write("input", videoInput);
                    writer.write("drawHud", drawHud);
                    writer.write("quality", quality);
                    writer.write("tss", sendTimestamp);
                    writer.write("tsr", receivedTimestamp);

                    auto img = imageSource->getImage(videoInput, drawHud);
                    auto encodedLength = jpegEncoder->encodeImage(img, sendImgBuffer, SEND_BUFFER_SIZE, quality);

                    logger.debug("JPEG file length: %d B.", encodedLength);

                    writer.write("tssr", common::utils::getTimestamp());
                    writer.close();

                    string header = headerStream.str();

                    std::memcpy(sendBuffer, header.c_str(), header.size());
                    sendBuffer[header.size()] = 0;
                    std::memcpy(sendBuffer + header.size() + 1, sendImgBuffer, encodedLength);

                    auto packetLength = encodedLength + header.size() + 1;

                    if (packetLength > UDP_MAX_PAYLOAD_SIZE) {
                        logger.warn("Payload size %d B, limiting to %d B.", packetLength, UDP_MAX_PAYLOAD_SIZE);
                        packetLength = min(packetLength, UDP_MAX_PAYLOAD_SIZE);
                    }

                    auto sentBytes = udpSocket->send_to(asio::buffer(sendBuffer, packetLength), endpoint);

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
