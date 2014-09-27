#include <iostream>
#include <chrono>
#include <functional>
#include <future>

#include <opencv2/opencv.hpp>

using namespace std::chrono;

std::chrono::microseconds measureTime(std::function<void()> func) {
	using std::chrono::duration_cast;
	using std::chrono::microseconds;
	using std::chrono::high_resolution_clock;

	auto timerBegin = high_resolution_clock::now();

	func();

	auto timerEnd = high_resolution_clock::now();

	return duration_cast<microseconds>(timerEnd - timerBegin);
}

int main() {

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

	double dWidth = leftVideoCapture.get(CV_CAP_PROP_FRAME_WIDTH); //get the width of frames of the video
	double dHeight = leftVideoCapture.get(CV_CAP_PROP_FRAME_HEIGHT); //get the height of frames of the video

	std::cout << "Frame size : " << dWidth << " x " << dHeight << std::endl;

	while (1) {
		cv::Mat leftFrame;
		cv::Mat rightFrame;

		auto duration = measureTime([&]() {

			auto rightJob = std::async(std::launch::async, [&]() -> bool {
				return rightVideoCapture.read(rightFrame);
			});

			bool leftSuccess = leftVideoCapture.read(leftFrame);
			bool rightSuccess = rightJob.get();
		});

		std::cout << "Duration frame grabbing: " << duration.count() << " us" << std::endl;

		auto duration2 = measureTime([&]() {
			cv::Size sz1 = leftFrame.size();
			cv::Size sz2 = rightFrame.size();

			cv::Mat im3(sz1.height, sz1.width + sz2.width, CV_8UC3);
			cv::Mat left(im3, cv::Rect(0, 0, sz1.width, sz1.height));
			leftFrame.copyTo(left);
			cv::Mat right(im3, cv::Rect(sz1.width, 0, sz2.width, sz2.height));
			rightFrame.copyTo(right);

			cv::imshow("im3", im3);
		});
		std::cout << "Duration processing: " << duration2.count() << " us" << std::endl;

		if (cv::waitKey(30) == 27) {
			std::cout << "esc key is pressed by user" << std::endl;
			break;
		}
	}

	return 0;

}