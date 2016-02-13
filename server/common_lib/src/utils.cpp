#include "utils.hpp"

#include <pthread.h>

#ifdef __FreeBSD__
#include <pthread_np.h>
#endif

#include <cstring>

std::string common::utils::getTimestamp() {

    std::stringstream now;

    auto tp = std::chrono::system_clock::now();
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(tp.time_since_epoch());
    size_t modulo = ms.count() % 1000;

    time_t seconds = std::chrono::duration_cast<std::chrono::seconds>(ms).count();

    char buffer[25]; // holds "2013-12-01 21:31:42"

    // note: localtime() is not threadsafe, lock with a mutex if necessary
    if (strftime(buffer, 25, "%H:%M:%S.", localtime(&seconds))) {
        now << buffer;
    }

    // ms
    now.fill('0');
    now.width(3);
    now << modulo;

    return now.str();
}

void common::utils::setThreadName(log4cpp::Category &logger, std::thread *thr, std::string name) {
#ifdef __FreeBSD__
    pthread_set_name_np(thr->native_handle(), name.c_str());
#else
    int result = pthread_setname_np(thr->native_handle(), name.c_str());
    if (result != 0) {
        logger.error("Cannot set thread name: %s.", std::strerror(result));
    }
#endif
}
