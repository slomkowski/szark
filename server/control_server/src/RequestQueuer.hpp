#pragma once

#include "IRequestProcessor.hpp"

#include <log4cpp/Category.hh>
#include <wallaroo/registered.h>
#include <json/value.h>
#include <json/reader.h>
#include <json/writer.h>
#include <boost/asio/ip/address.hpp>

#include <string>
#include <list>
#include <vector>
#include <functional>
#include <queue>
#include <mutex>
#include <thread>
#include <condition_variable>

namespace processing {

    typedef std::function<void(long, std::string, bool)> ResponseSender;
    typedef std::function<void(long)> RejectedRequestRemover;

    constexpr long INVALID_MESSAGE = -1;



//    typedef std::tuple<long, boost::asio::ip::address, Json::Value> Request;


    class IRequestQueuer {
    public:
        /**
        * Adds the request to the queue.
        * @param request text of the request in JSON format
        * @param address IP address of the client
        * @return true if the request was added to the queue
        */
        virtual long addRequest(std::string request, boost::asio::ip::address address) = 0;

        virtual int getNumOfMessages() = 0;

        virtual int getNumOfProcessors() = 0;

        virtual void setResponseSender(ResponseSender s) = 0;

        virtual void setRejectedRequestRemover(RejectedRequestRemover r) = 0;

        virtual ~IRequestQueuer() = default;
    };

    class RequestValueComparer {
    public:
        bool operator()(const std::shared_ptr<Request> &r1, const std::shared_ptr<Request> &r2) const {
            //return std::get<2>(r1)["serial"].asInt() > std::get<2>(r2)["serial"].asInt();
            return r1->serial > r2->serial;
        }
    };

//    class RequestValueComparer {
//    public:
//        bool operator()(const Request &r1, const Request &r2) const {
//            //return std::get<2>(r1)["serial"].asInt() > std::get<2>(r2)["serial"].asInt();
//            return r1.serial > r2.serial;
//        }
//    };

    class RequestQueuer : public wallaroo::Part, public IRequestQueuer {
    public:
        RequestQueuer();

        ~RequestQueuer();

        virtual long addRequest(std::string requestString, boost::asio::ip::address address);

        virtual int getNumOfMessages();

        virtual int getNumOfProcessors();

        void setResponseSender(ResponseSender s) {
            responseSender = s;
        }

        void setRejectedRequestRemover(RejectedRequestRemover r) {
            rejectedRequestRemover = r;
        }

    private:
        log4cpp::Category &logger;

        wallaroo::Collaborator<IRequestProcessor, wallaroo::collection> requestProcessors;

        std::mutex requestsMutex;
        std::priority_queue<std::shared_ptr<Request>, std::vector<std::shared_ptr<Request>>, RequestValueComparer> requests;
        volatile long lastSerial = 0;

        std::unique_ptr<std::thread> requestProcessorExecutorThread;
        std::condition_variable cv;
        volatile bool finishCycleThread = false;

        Json::Reader jsonReader;

        ResponseSender responseSender;
        RejectedRequestRemover rejectedRequestRemover;

        void Init();

        void requestProcessorExecutorThreadFunction();

        long nextId();
    };

}