#include "BridgeProcessor.hpp"

#include <boost/test/unit_test.hpp>
#include <wallaroo/catalog.h>
#include <json/value.h>
#include <minijson_writer.hpp>

#include <thread>
#include <chrono>

using namespace std;
using namespace std::chrono;

static void execTest(const shared_ptr<bridge::BridgeProcessor> &proc) {
    this_thread::sleep_for(milliseconds(500));
    Json::Value req;
    std::stringstream respStream;
    minijson::object_writer resp(respStream);
    proc->process(req, boost::asio::ip::address(), resp);
    BOOST_TEST_MESSAGE(respStream.str());
    for (int i = 0; i < 30; i++) {
        proc->process(req, boost::asio::ip::address(), resp);
        this_thread::sleep_for(milliseconds(40));
    }
    BOOST_CHECK_EQUAL(1, 1);
}

BOOST_AUTO_TEST_CASE(BridgeProcessorTest_Run) {
    wallaroo::Catalog catalog;

    catalog.Create("comm", "USBCommunicator");
    catalog.Create("bp", "BridgeProcessor");

    wallaroo::use(catalog["comm"]).as("communicator").of(catalog["bp"]);

    catalog.CheckWiring();

    shared_ptr<bridge::BridgeProcessor> proc = catalog["bp"];

    execTest(proc);
}

