#include <memory>
#include <thread>
#include <boost/test/unit_test.hpp>
#include <wallaroo/catalog.h>

#include "NetServer.hpp"

using namespace std;
using namespace processing;

class RequestQueuerMock : public wallaroo::Device, public processing::IRequestQueuer {
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
	wallaroo::use(catalog["rq"]).as("requestQueuer").of(catalog["netServer"]);

	catalog.CheckWiring();

	auto netServer = shared_ptr<INetServer>(catalog["netServer"]);

	netServer->run();

	this_thread::sleep_for(chrono::milliseconds(500));

	BOOST_CHECK_EQUAL(1, 1);
}
