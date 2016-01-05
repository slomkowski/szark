#include "SharedInterfaceProvider.hpp"

#include <log4cpp/Category.hh>
#include <boost/test/unit_test.hpp>
#include <wallaroo/catalog.h>

#include <thread>

using namespace std;
using namespace common::bridge;

BOOST_AUTO_TEST_CASE(SharedInterfaceProviderTest_Slave) {
    auto &logger = log4cpp::Category::getInstance("SharedInterfaceProviderTest");
    wallaroo::Catalog c;
    c.Create("ifaceProvider", "InterfaceProvider", false);

    c.CheckWiring();
    c.Init();

    auto interfaceProvider = std::shared_ptr<common::bridge::InterfaceProvider>(c["ifaceProvider"]);
    auto *iface = interfaceProvider->getInterface();

    BOOST_CHECK(interfaceProvider != nullptr);

    {
        common::bridge::SharedScopedMutex lk(iface->mutex);

        logger.info("Initial speed: %d.", iface->motor.left.getSpeed());
        iface->setKillSwitch(false);
        iface->motor.left.setSpeed(5);
        iface->motor.left.setDirection(Direction::FORWARD);
    }

    this_thread::sleep_for(chrono::milliseconds(70));

    {
        common::bridge::SharedScopedMutex lk(iface->mutex);
        iface->setKillSwitch(true);
    }

    BOOST_CHECK_EQUAL(1, 1);
}