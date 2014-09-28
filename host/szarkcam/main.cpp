#include <iostream>
#include <chrono>
#include <functional>
#include <future>

#include <boost/asio.hpp>
#include <opencv2/opencv.hpp>

using namespace boost;
using boost::asio::ip::udp;

using namespace std::chrono;
const int MAX_PACKET_SIZE = 1200;

std::chrono::microseconds measureTime(std::function<void()> func) {
	using std::chrono::duration_cast;
	using std::chrono::microseconds;
	using std::chrono::high_resolution_clock;

	auto timerBegin = high_resolution_clock::now();

	func();

	auto timerEnd = high_resolution_clock::now();

	return duration_cast<microseconds>(timerEnd - timerBegin);
}

void doReceive(udp::socket &udpSocket, cv::VideoCapture &leftVideoCapture, cv::VideoCapture &rightVideoCapture, std::unique_ptr<char> &buff, udp::endpoint &recvSenderEndpoint) {
	udpSocket.async_receive_from(asio::buffer(buff.get(), MAX_PACKET_SIZE), recvSenderEndpoint,
			[&](system::error_code ec, size_t bytes_recvd) {
				cv::Mat leftFrame;
				cv::Mat rightFrame;

				auto duration = measureTime([&]() {

					auto rightJob = std::async(std::launch::async, [&]() -> bool {
						return rightVideoCapture.read(rightFrame);
					});

					bool leftSuccess = leftVideoCapture.read(leftFrame);
					bool rightSuccess = rightJob.get();
				});

				//std::cout << "Duration frame grabbing: " << duration.count() << " us" << std::endl;

				cv::vector<unsigned char> buf;

				auto duration2 = measureTime([&]() {

					cv::flip(leftFrame, leftFrame, 0);
					cv::transpose(leftFrame, leftFrame);
					cv::flip(leftFrame, leftFrame, 0);

					cv::transpose(rightFrame, rightFrame);

					cv::Size sizeLeft = leftFrame.size();
					cv::Size sizeRight = rightFrame.size();

					cv::Mat im3(sizeLeft.height, sizeLeft.width + sizeRight.width, CV_8UC3);
					cv::Mat left(im3, cv::Rect(0, 0, sizeLeft.width, sizeLeft.height));
					leftFrame.copyTo(left);
					cv::Mat right(im3, cv::Rect(sizeLeft.width, 0, sizeRight.width, sizeRight.height));
					rightFrame.copyTo(right);

					//cv::imshow("im3", im3);
					std::vector<int> compression_params; //vector that stores the compression parameters of the image
					compression_params.push_back(CV_IMWRITE_JPEG_QUALITY); //specify the compression technique

					compression_params.push_back(45);
					//cv::imwrite("out.jpg", im3, compression_params);

					cv::imencode(".jpg", im3, buf, compression_params);
				});
				//std::cout << "Duration processing: " << duration2.count() << " us" << std::endl;

				udpSocket.async_send_to(boost::asio::buffer(&buf[0], buf.size()), recvSenderEndpoint,
						[&buf](system::error_code ec, size_t bytes_sent) {
							//std::cout << "buf size " << buf.size() << ", sent " << bytes_sent << std::endl;
						});

				doReceive(udpSocket, leftVideoCapture, rightVideoCapture, buff, recvSenderEndpoint);
			});
}


int main() {

	boost::asio::io_service ioService;
	boost::asio::ip::udp::socket udpSocket(ioService);

	boost::system::error_code err;
	udpSocket.open(udp::v4());
	udpSocket.bind(udp::endpoint(udp::v4(), 10192), err);

	cv::VideoCapture leftVideoCapture(0);
	cv::VideoCapture rightVideoCapture(1);

	if (!leftVideoCapture.isOpened())  // if not success, exit program
	{
		std::cout << "Cannot open the video cam" << std::endl;
		return -1;
	}

	if (!rightVideoCapture.isOpened())  // if not success, exit program
	{
		std::cout << "Cannot open the video cam" << std::endl;
		return -1;
	}

	leftVideoCapture.set(CV_CAP_PROP_FRAME_WIDTH, 352);
	leftVideoCapture.set(CV_CAP_PROP_FRAME_HEIGHT, 288);

	rightVideoCapture.set(CV_CAP_PROP_FRAME_WIDTH, 352);
	rightVideoCapture.set(CV_CAP_PROP_FRAME_HEIGHT, 288);


	std::unique_ptr<char> buff;
	buff.reset(new char[MAX_PACKET_SIZE]);
	boost::asio::ip::udp::endpoint recvSenderEndpoint;

	doReceive(udpSocket, leftVideoCapture, rightVideoCapture, buff, recvSenderEndpoint);

	ioService.run();
}

