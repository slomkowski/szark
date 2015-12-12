#include "CameraImageGrabber.hpp"
#include "utils.hpp"
#include "Configuration.hpp"

#include <opencv2/opencv.hpp>
#include <boost/format.hpp>

#include <pthread.h>
#include <cstring>

#include <sys/ioctl.h>
#include <linux/v4l2-common.h>
#include <linux/videodev2.h>
#include <sys/mman.h>

using namespace std;
using namespace boost;
using namespace camera;

const int FRAMERATE_AVG_FRAMES = 5;

WALLAROO_REGISTER(ImageGrabber, string);

camera::ImageGrabber::ImageGrabber(const std::string &prefix) :
        prefix(prefix),
        logger(log4cpp::Category::getInstance("ImageGrabber")),
        config("config", RegistrationToken()),
        captureTimesAvgBuffer(FRAMERATE_AVG_FRAMES),
        currentFrameNo(1) {
}

void ImageGrabber::Init() {
    int videoDevice = config->getInt(getFullConfigPath("device"));

    videoCapture.reset(new cv::VideoCapture(videoDevice));

    if (not videoCapture->isOpened()) {
        throw ImageGrabberException((format("cannot open device %d.") % videoDevice).str());
    }

    setVideoCaptureProperty(CV_CAP_PROP_FRAME_WIDTH, "width");
    setVideoCaptureProperty(CV_CAP_PROP_FRAME_HEIGHT, "height");

    logger.notice((format("Set '%s' frame size to %dx%d.") % prefix
                   % videoCapture->get(CV_CAP_PROP_FRAME_WIDTH)
                   % videoCapture->get(CV_CAP_PROP_FRAME_HEIGHT)).str());

    grabberThread.reset(new std::thread(&ImageGrabber::grabberThreadFunction, this));
    int result = pthread_setname_np(grabberThread->native_handle(), (prefix + "ImgGrab").c_str());
    if (result != 0) {
        logger.error((format("Cannot set thread name: %s.") % strerror(result)).str());
    }

    logger.notice("Instance created.");
}

std::tuple<long, double, cv::Mat>  camera::ImageGrabber::getFrame(bool wait) {
    std::unique_lock<std::mutex> lk(dataMutex);

    if (not wait) {
        cond.wait(lk);
        logger.info("Got frame.");
    } else {
        logger.info("Got frame without waiting.");
    }

    return std::tuple<long, double, cv::Mat>(currentFrameNo, currentFps, currentFrame);
}

camera::ImageGrabber::~ImageGrabber() {
    dataMutex.lock();
    finishThread = true;
    dataMutex.unlock();

    grabberThread->join();

    videoCapture->release();

    logger.notice("Instance destroyed.");
}

void ImageGrabber::grabberThreadFunction() {
    while (!finishThread) {
        cv::Mat frame;
        bool validFrame;
        int elapsedTime = common::utils::measureTime<std::chrono::milliseconds>([&]() {
            validFrame = videoCapture->read(frame);
        });

        if (not validFrame) {
            throw ImageGrabberException("invalid frame");
        }

        captureTimesAvgBuffer.push_back(elapsedTime);

        double fps = captureTimesAvgBuffer.size() * 1000.0 /
                     (std::accumulate(captureTimesAvgBuffer.begin(), captureTimesAvgBuffer.end(), 0));

        logger.info((format("Captured frame no %d in %d ms (%2.1f fps).") % currentFrameNo % elapsedTime % fps).str());

        dataMutex.lock();
        this->currentFrame = frame;
        this->currentFrameNo++;
        this->currentFps = fps;
        dataMutex.unlock();

        cond.notify_all();
    }
}

std::string ImageGrabber::getFullConfigPath(std::string property) {
    return "ImageGrabber." + prefix + "_" + property;
}

void ImageGrabber::setVideoCaptureProperty(int prop, std::string confName) {
    auto path = getFullConfigPath(confName);
    int val;

    try {
        val = config->getInt(path);
    } catch (common::config::ConfigException &e) {
        throw ImageGrabberException((format("failed to set property: %s") % e.what()).str());
    }

    videoCapture->set(prop, val);

//	if (readVal != val) {
//		throw ImageGrabberException((format("failed to set property '%s' to value %d, previous value: %d")
//				% path % val % readVal).str());
//	}

    logger.info((format("Set property '%s' to value %d.") % path % val).str());
}

static int xioctl(int fd, int request, void *arg) {
    int r;
    do r = ioctl(fd, request, arg);
    while (-1 == r && EINTR == errno);
    return r;
}

namespace camera {

    class Video4LinuxImageGrabber : public IImageGrabber, public wallaroo::Device {
    public:
        Video4LinuxImageGrabber(const std::string &prefix)
                : prefix(prefix),
                  logger(log4cpp::Category::getInstance("V4LImageGrabber")),
                  config("config", RegistrationToken()),
                  captureTimesAvgBuffer(FRAMERATE_AVG_FRAMES),
                  currentFrameNo(1) { }

        virtual ~ Video4LinuxImageGrabber() { }

        virtual std::tuple<long, double, cv::Mat> getFrame(bool wait) {
            std::unique_lock<std::mutex> lk(dataMutex);

            if (not wait) {
                cond.wait(lk);
                logger.info("Got frame.");
            } else {
                logger.info("Got frame without waiting.");
            }

            return std::tuple<long, double, cv::Mat>(currentFrameNo, currentFps, currentFrame);
        }

    private:
        std::string prefix;

        log4cpp::Category &logger;

        wallaroo::Plug<common::config::Configuration> config;

        boost::circular_buffer<double> captureTimesAvgBuffer;

        std::unique_ptr<cv::VideoCapture> videoCapture;

        std::unique_ptr<std::thread> grabberThread;

        std::mutex dataMutex;
        std::condition_variable cond;

        cv::Mat currentFrame;
        int currentFrameNo;
        double currentFps;

        vector<uint8_t *> buffers;

        volatile bool finishThread = false;

        int fd;

        void checkedXioctl(int fd, int request, void *arg, string errorMessage) {
            if (-1 == xioctl(fd, request, arg)) {
                throw ImageGrabberException(errorMessage);
            }
        }


        virtual void Init() {
            string videoDevice = "/dev/video" + to_string(config->getInt(getFullConfigPath("device")));

            fd = open(videoDevice.c_str(), O_RDWR);
            if (fd == -1) {
                // couldn't find capture device
                throw ImageGrabberException(string("Opening Video device ") + videoDevice);
            }

            v4l2_capability caps = {0};
            if (-1 == xioctl(fd, VIDIOC_QUERYCAP, &caps)) {
                throw ImageGrabberException("Querying Capabilites");
            }

            if (!(caps.capabilities & V4L2_CAP_STREAMING)) {
                throw ImageGrabberException(videoDevice + " does not support streaming I/O");
            }

            printf("Driver Caps:\n"
                           "  Driver: \"%s\"\n"
                           "  Card: \"%s\"\n"
                           "  Bus: \"%s\"\n"
                           "  Version: %d.%d\n"
                           "  Capabilities: %08x\n",
                   caps.driver,
                   caps.card,
                   caps.bus_info,
                   (caps.version >> 16) && 0xff,
                   (caps.version >> 24) && 0xff,
                   caps.capabilities);


            int support_grbg10 = 0;

            struct v4l2_fmtdesc fmtdesc = {0};
            fmtdesc.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
            char fourcc[5] = {0};
            char c, e;
            printf("  FMT : CE Desc\n--------------------\n");
            while (0 == xioctl(fd, VIDIOC_ENUM_FMT, &fmtdesc)) {
                strncpy(fourcc, (char *) &fmtdesc.pixelformat, 4);
                if (fmtdesc.pixelformat == V4L2_PIX_FMT_SGRBG10)
                    support_grbg10 = 1;
                c = fmtdesc.flags & 1 ? 'C' : ' ';
                e = fmtdesc.flags & 2 ? 'E' : ' ';
                printf("  %s: %c%c %s\n", fourcc, c, e, fmtdesc.description);
                fmtdesc.index++;
            }

            if (!support_grbg10) {
                printf("Doesn't support GRBG10.\n");
            }


            v4l2_format fmt = {0};
            fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
            fmt.fmt.pix.width = getVideoCaptureProperty("width");
            fmt.fmt.pix.height = getVideoCaptureProperty("height");
            fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_BGR24;
            fmt.fmt.pix.field = V4L2_FIELD_NONE;

            if (-1 == xioctl(fd, VIDIOC_S_FMT, &fmt)) {
                throw ImageGrabberException("Setting Pixel Format");
            }

            strncpy(fourcc, (char *) &fmt.fmt.pix.pixelformat, 4);
            printf("Selected Camera Mode:\n"
                           "  Width: %d\n"
                           "  Height: %d\n"
                           "  PixFmt: %s\n"
                           "  Field: %d\n",
                   fmt.fmt.pix.width,
                   fmt.fmt.pix.height,
                   fourcc,
                   fmt.fmt.pix.field);

            v4l2_requestbuffers req = {0};
            req.count = 16; // some high value
            req.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
            req.memory = V4L2_MEMORY_MMAP;
            checkedXioctl(fd, VIDIOC_REQBUFS, &req, "requesting buffer");

            if (req.count < 2) {
                throw ImageGrabberException("Insufficient buffer memory ");
            }

            for (unsigned int i = 0; i < req.count; ++i) {
                v4l2_buffer buf = {0};
                buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
                buf.memory = V4L2_MEMORY_MMAP;
                buf.index = i;

                checkedXioctl(fd, VIDIOC_QUERYBUF, &buf, "query buffer");

                uint8_t *bufferPtr = static_cast<uint8_t *>(
                        mmap(NULL, buf.length, PROT_READ | PROT_WRITE, MAP_SHARED, fd, buf.m.offset));

                printf("Length: %d\nAddress: %p\n", buf.length, bufferPtr);
                printf("Image Length: %d\n", buf.bytesused);

                buffers.push_back(bufferPtr);
            }

            queryAllBuffers();

            uint32_t bufType = V4L2_BUF_TYPE_VIDEO_CAPTURE;
            checkedXioctl(fd, VIDIOC_STREAMON, &bufType, "streamon");

            grabberThread.reset(new std::thread(&Video4LinuxImageGrabber::grabberThreadFunction, this));
            int result = pthread_setname_np(grabberThread->native_handle(), (prefix + "ImgGrab").c_str());
            if (result != 0) {
                logger.error((format("Cannot set thread name: %s.") % strerror(result)).str());
            }

            logger.notice("Instance created.");
        }

        void queryAllBuffers() {
            for (unsigned int i = 0; i < buffers.size(); ++i) {
                v4l2_buffer buf = {0};
                buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
                buf.memory = V4L2_MEMORY_MMAP;
                buf.index = i;

                checkedXioctl(fd, VIDIOC_QBUF, &buf, "error during querying all buffers");
            }
        }

        void grabberThreadFunction() {
            while (!finishThread) {
                cv::Mat frame;
                int elapsedTime = common::utils::measureTime<std::chrono::milliseconds>([&]() {

                    fd_set fds;
                    FD_ZERO(&fds);
                    FD_SET(fd, &fds);
                    timeval tv = {0};
                    tv.tv_sec = 2;
                    int r = select(fd + 1, &fds, NULL, NULL, &tv);
                    if (-1 == r) {
                        throw ImageGrabberException("Waiting for Frame");
                    }

                    v4l2_buffer buf = {0};
                    buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
                    buf.memory = V4L2_MEMORY_MMAP;

                    checkedXioctl(fd, VIDIOC_DQBUF, &buf, "error during dequeing buffer");

                    logger.info("idx: %d, bytes used: %d", buf.index, buf.bytesused);

                    frame = cv::Mat(720, 480, CV_8UC3, (void *) buffers[buf.index]);

                    checkedXioctl(fd, VIDIOC_QBUF, &buf, "error during querying buffer");

                    logger.info("Height: %d, width: %d", frame.rows, frame.cols);
                });


                captureTimesAvgBuffer.push_back(elapsedTime);

                double fps = captureTimesAvgBuffer.size() * 1000.0 /
                             (std::accumulate(captureTimesAvgBuffer.begin(), captureTimesAvgBuffer.end(), 0));

                logger.info((format("Captured frame no %d in %d ms (%2.1f fps).") % currentFrameNo % elapsedTime %
                             fps).str());

                dataMutex.lock();
                this->currentFrame = frame;
                this->currentFrameNo++;
                this->currentFps = fps;
                dataMutex.unlock();

                cond.notify_all();
            }
        }

        std::string getFullConfigPath(std::string property) {
            return "ImageGrabber." + prefix + "_" + property;
        }

        int getVideoCaptureProperty(std::string confName) {
            auto path = getFullConfigPath(confName);
            try {
                return config->getInt(path);
            } catch (common::config::ConfigException &e) {
                throw ImageGrabberException((format("failed to set property: %s") % e.what()).str());
            }

//	if (readVal != val) {
//		throw ImageGrabberException((format("failed to set property '%s' to value %d, previous value: %d")
//				% path % val % readVal).str());
//	}

//            logger.info((format("Set property '%s' to value %d.") % path % val).str());
        }
    };

}

WALLAROO_REGISTER(Video4LinuxImageGrabber, string);