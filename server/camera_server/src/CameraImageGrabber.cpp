#include "CameraImageGrabber.hpp"
#include "utils.hpp"
#include "Configuration.hpp"

#include <opencv2/opencv.hpp>

#include <pixfc-sse.h>

#include <sys/ioctl.h>
#include <linux/v4l2-common.h>
#include <linux/videodev2.h>
#include <sys/mman.h>

#include <boost/format.hpp>
#include <pthread.h>
#include <cstring>
#include <cstdlib>

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

    logger.notice("Set '%s' frame size to %dx%d.", prefix.c_str(), videoCapture->get(CV_CAP_PROP_FRAME_WIDTH),
                  videoCapture->get(CV_CAP_PROP_FRAME_HEIGHT));

    grabberThread.reset(new std::thread(&ImageGrabber::grabberThreadFunction, this));
    int result = pthread_setname_np(grabberThread->native_handle(), (prefix + "ImgGrab").c_str());
    if (result != 0) {
        logger.error("Cannot set thread name: %s.", strerror(result));
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

        logger.info("Captured frame no %d in %d ms (%2.1f fps).", currentFrameNo, elapsedTime, fps);

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

    logger.info("Set property '%s' to value %d.", path.c_str(), val);
}

static int xioctl(int fd, unsigned long request, void *arg) {
    int r;
    do r = ioctl(fd, request, arg);
    while (-1 == r && EINTR == errno);
    return r;
}

static void checkedXioctl(int fd, unsigned long request, void *arg, string errorMessage) {
    int status;
    do {
        status = ioctl(fd, request, arg);
    } while (status == -1 && EINTR == errno);

    if (status == -1) {
        const char *errorDescription = std::strerror(errno);
        throw ImageGrabberException((boost::format("%s: %s") % errorMessage % errorDescription).str());
    }
}

namespace camera {

    struct VideoBuffer {
        uint8_t *video4linuxBuffer;
        uint8_t *rgbBuffer;
    };

    class Video4LinuxImageGrabber : public IImageGrabber, public wallaroo::Part {
    public:
        Video4LinuxImageGrabber(const std::string &prefix)
                : prefix(prefix),
                  logger(log4cpp::Category::getInstance("V4LImageGrabber")),
                  config("config", RegistrationToken()),
                  captureTimesAvgBuffer(FRAMERATE_AVG_FRAMES),
                  currentFrameNo(1) { }

        virtual ~ Video4LinuxImageGrabber() {
            finishThread = true;

            if (grabberThread.get() != nullptr) {
                grabberThread->join();
            }

            //todo close v4l

            destroy_pixfc(pixfc);

            for (auto vb : buffers) {
                free(vb.rgbBuffer);
            }
        }

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

        void setInput(int inputNo) {
            logger.info("Setting video input to %d.", inputNo);
            checkedXioctl(fd, VIDIOC_S_INPUT, &inputNo, "cannot set input to " + to_string(inputNo));
        }

    private:
        std::string prefix;

        log4cpp::Category &logger;

        wallaroo::Collaborator<common::config::Configuration> config;

        boost::circular_buffer<double> captureTimesAvgBuffer;

        std::unique_ptr<std::thread> grabberThread;

        std::mutex dataMutex;
        std::condition_variable cond;

        cv::Mat currentFrame;
        int currentFrameNo;
        double currentFps;

        vector<VideoBuffer> buffers;

        volatile bool finishThread = false;

        int fd;

        unsigned int width;
        unsigned int height;

        PixFcSSE *pixfc;

        virtual void Init() {
            string videoDevice = "/dev/video" + to_string(config->getInt(getFullConfigPath("device")));

            fd = open(videoDevice.c_str(), O_RDWR);
            if (fd == -1) {
                throw ImageGrabberException(string("cannot open device ") + videoDevice);
            }

            v4l2_capability caps = {};
            checkedXioctl(fd, VIDIOC_QUERYCAP, &caps, "cannot query capabilities");

            if (not (caps.capabilities & V4L2_CAP_STREAMING)) {
                throw ImageGrabberException("device doesn't support streaming I/O");
            }

            logger.info("Capabilities: driver: %s, card: %s, bus: %s, version: %d.%d, caps: %08x.",
                        caps.driver,
                        caps.card,
                        caps.bus_info,
                        (caps.version >> 16) & 0xff, (caps.version >> 24) & 0xff,
                        caps.capabilities);

            v4l2_fmtdesc fmtdesc = {};
            fmtdesc.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

            char fourcc[5] = {0};
            logger.info("Supported formats:");
            while (0 == xioctl(fd, VIDIOC_ENUM_FMT, &fmtdesc)) {
                memcpy(fourcc, &fmtdesc.pixelformat, 4);
                char c = fmtdesc.flags & 1 ? 'C' : ' ';
                char e = fmtdesc.flags & 2 ? 'E' : ' ';
                logger.info(">>  %s: %c%c %s.", fourcc, c, e, fmtdesc.description);
                fmtdesc.index++;
            }

            width = getVideoCaptureProperty("width");
            height = getVideoCaptureProperty("height");
            v4l2_format fmt = {};
            fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
            fmt.fmt.pix.width = width;
            fmt.fmt.pix.height = height;
            fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_BGR24;
            fmt.fmt.pix.field = V4L2_FIELD_NONE;

            checkedXioctl(fd, VIDIOC_S_FMT, &fmt, "cannot set pixel format");

            memcpy(fourcc, &fmt.fmt.pix.pixelformat, 4);
            logger.notice("Camera mode: width: %d, height: %d, pixel format: %s, field: %d.",
                          fmt.fmt.pix.width,
                          fmt.fmt.pix.height,
                          fourcc,
                          fmt.fmt.pix.field);

            logger.info("Available inputs:");
            v4l2_input input = {};
            while (0 == xioctl(fd, VIDIOC_ENUMINPUT, &input)) {
                logger.info(">> %d: %s.", input.index, input.name);
                input.index++;
            }

            setInput(getVideoCaptureProperty("input"));

            v4l2_requestbuffers req = {};
            req.count = 16; // some high value
            req.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
            req.memory = V4L2_MEMORY_MMAP;
            checkedXioctl(fd, VIDIOC_REQBUFS, &req, "error when requesting memory buffers");

            if (req.count < 2) {
                throw ImageGrabberException("insufficient buffer memory");
            }

            for (unsigned int i = 0; i < req.count; ++i) {
                v4l2_buffer buf = {};
                buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
                buf.memory = V4L2_MEMORY_MMAP;
                buf.index = i;

                checkedXioctl(fd, VIDIOC_QUERYBUF, &buf, "error when querying buffer");

                VideoBuffer buffer;
                buffer.video4linuxBuffer = static_cast<uint8_t *>(
                        mmap(nullptr, buf.length, PROT_READ | PROT_WRITE, MAP_SHARED, fd, buf.m.offset));

                logger.debug("Initialized buffer %d: address: %p, length: %d.", i,
                             buffer.video4linuxBuffer, buf.length);

                unsigned int rgbBufferSize = fmt.fmt.pix.width * fmt.fmt.pix.height * 3;

                if (posix_memalign(reinterpret_cast<void **>(&buffer.rgbBuffer), 16, rgbBufferSize) != 0) {
                    throw ImageGrabberException("cannot allocate buffer " + to_string(i));
                }

                logger.debug("Allocated %d B for RGB buffer %d (%p).", rgbBufferSize, i, buffer.rgbBuffer);

                buffers.push_back(buffer);
            }

            queryAllBuffers();

            uint32_t bufType = V4L2_BUF_TYPE_VIDEO_CAPTURE;
            checkedXioctl(fd, VIDIOC_STREAMON, &bufType, "streamon");

            int status = create_pixfc(&pixfc, PixFcUYVY, PixFcBGR24,
                                      width, height, width * 2, width * 3, PixFcFlag_Default);

            if (status != 0) {
                throw ImageGrabberException("cannot create struct for pixfc: " + to_string(status));
            }

            grabberThread.reset(new std::thread(&Video4LinuxImageGrabber::grabberThreadFunction, this));
            int result = pthread_setname_np(grabberThread->native_handle(), (prefix + "ImgGrab").c_str());
            if (result != 0) {
                logger.error("Cannot set thread name: %s.", strerror(result));
            }

            logger.notice("Instance created.");
        }

        void queryAllBuffers() {
            for (unsigned int i = 0; i < buffers.size(); ++i) {
                v4l2_buffer buf = {};
                buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
                buf.memory = V4L2_MEMORY_MMAP;
                buf.index = i;

                checkedXioctl(fd, VIDIOC_QBUF, &buf, "error during querying all buffers");
            }
        }

        void grabberThreadFunction() {
            while (!finishThread) {
                v4l2_buffer v4l2_buf = {};
                v4l2_buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
                v4l2_buf.memory = V4L2_MEMORY_MMAP;

                int elapsedTime = common::utils::measureTime<std::chrono::milliseconds>([&]() {

                    fd_set fds;
                    FD_ZERO(&fds);
                    FD_SET(fd, &fds);
                    timeval tv = {};
                    tv.tv_sec = 2;
                    int r = select(fd + 1, &fds, nullptr, nullptr, &tv);
                    if (-1 == r) {
                        throw ImageGrabberException("Waiting for Frame");
                    }

                    checkedXioctl(fd, VIDIOC_DQBUF, &v4l2_buf, "error during dequeing buffer");

                    auto *buffer = &buffers[v4l2_buf.index];

                    logger.debug("Buffer %d (%p), bytes used: %d.", v4l2_buf.index, buffer->video4linuxBuffer,
                                 v4l2_buf.bytesused);
                });


                captureTimesAvgBuffer.push_back(elapsedTime);

                double fps = captureTimesAvgBuffer.size() * 1000.0 /
                             (std::accumulate(captureTimesAvgBuffer.begin(), captureTimesAvgBuffer.end(), 0));

                logger.info("Captured frame no %d in %d ms (%2.1f fps).", currentFrameNo, elapsedTime, fps);

                cv::Mat frame;

                elapsedTime = common::utils::measureTime<std::chrono::microseconds>([&]() {
                    auto *buffer = &buffers[v4l2_buf.index];

                    pixfc->convert(pixfc, buffer->video4linuxBuffer, buffer->rgbBuffer);
                    // todo timecode can be used
                    frame = cv::Mat(height, width, CV_8UC3, buffer->rgbBuffer);

                    logger.debug("Mat: height: %d, width: %d.", frame.rows, frame.cols);
                });

                logger.info("Converted frame %d from UYUV to RGB in %d us.", currentFrameNo, elapsedTime);

                checkedXioctl(fd, VIDIOC_QBUF, &v4l2_buf, "error during querying buffer");

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
                int val = config->getInt(path);
                logger.info("Trying to set property '%s' to %d.", confName.c_str(), val);
                return val;
            } catch (common::config::ConfigException &e) {
                throw ImageGrabberException((format("failed to set property: %s") % e.what()).str());
            }
        }
    };
}

WALLAROO_REGISTER(Video4LinuxImageGrabber, string);