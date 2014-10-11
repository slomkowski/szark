#include <boost/format.hpp>
#include "NetworkServer.hpp"
#include "Configuration.hpp"

using namespace std;
using namespace boost;
using namespace camera;

namespace camera {
	const int RECEIVED_DATA_MAX_LENGTH = 10;
}

WALLAROO_REGISTER(NetworkServer);

camera::NetworkServer::NetworkServer()
		: logger(log4cpp::Category::getInstance("NetworkServer")),
		  imageSource("imageSource", RegistrationToken()),
		  ioService(),
		  udpSocket(ioService) {
	initializeServer(config::getInt("szark.server.camera.NetworkServer.port"));
}

camera::NetworkServer::NetworkServer(int port)
		: logger(log4cpp::Category::getInstance("NetworkServer")),
		  imageSource("imageSource", RegistrationToken()),
		  ioService(),
		  udpSocket(ioService) {
	initializeServer(port);
}

void NetworkServer::initializeServer(int port) {
	using boost::asio::ip::udp;

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
	udpSocket.async_receive_from(asio::buffer(recvBuffer.get(), RECEIVED_DATA_MAX_LENGTH), endpoint,
			[this](boost::system::error_code ec, std::size_t bytesReceived) {
				if (ec) {
					throw NetworkException((format("error at receiving request: %s") % ec.message()).str());
				}

				bool drawHud = string(recvBuffer.get(), bytesReceived) == "HUD";

				logger.info((format("Received request %s HUD.") % (drawHud ? "with" : "without")).str());

				imageSource->getEncodedImage(drawHud, [this](void *buff, size_t length) {
					auto sentBytes = udpSocket.send_to(asio::buffer(buff, length), endpoint);
					if (sentBytes != length) {
						throw NetworkException((format("not whole file sent (%u < %u)") % sentBytes % length).str());
					}
					logger.info((format("Sent file (%u bytes).") % length).str());
				});

				doReceive();
			});
}

