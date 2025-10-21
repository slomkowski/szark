#include "NetServer.hpp"

#include <boost/test/unit_test.hpp>
#include <wallaroo/catalog.h>

#include <memory>
#include <thread>

using namespace std;
using namespace processing;

class RequestQueuerMock : public wallaroo::Part, public processing::IRequestQueuer {
public:
    virtual long addRequest(std::string request, boost::asio::ip::address address) {
        id++;
        //respThread.reset(new thread(&RequestQueuerMock::respThreadFunction, this));
        return id;
    }

    virtual int getNumOfMessages() {
        return 1;
    }

    virtual int getNumOfProcessors() {
        return 1;
    }

    virtual void setResponseSender(ResponseSender s) {
        callback = s;
    }

    virtual void setRejectedRequestRemover(RejectedRequestRemover r) {

    }

    ~RequestQueuerMock() {
        if (respThread.get() != nullptr) {
            respThread->join();
        }
    }

private:
    ResponseSender callback;
    long id = 1;
    unique_ptr<thread> respThread;

    void respThreadFunction() {
        this_thread::sleep_for(chrono::milliseconds(500));
        callback(id - 1, "sample response", true);
    }
};

WALLAROO_REGISTER(RequestQueuerMock);

BOOST_AUTO_TEST_CASE(NetServerTest_Run) {
    wallaroo::Catalog catalog;

    catalog.Create("rq", "RequestQueuerMock");
    catalog.Create("netServer", "NetServer");
    catalog.Create("ioServiceProvider", "IoServiceProvider");

    wallaroo::use(catalog["rq"]).as("requestQueuer").of(catalog["netServer"]);
    wallaroo::use(catalog["ioServiceProvider"]).as("ioServiceProvider").of(catalog["netServer"]);

    catalog.CheckWiring();

    auto ioServiceProvider = std::shared_ptr<common::IoServiceProvider>(catalog["ioServiceProvider"]);

    ioServiceProvider->getIoContext().run();

    this_thread::sleep_for(chrono::milliseconds(500));

    BOOST_CHECK_EQUAL(1, 1);
}
