#include "SharedInterfaceProvider.hpp"

#include <log4cpp/Category.hh>
#include <boost/test/unit_test.hpp>
#include <wallaroo/catalog.h>

using namespace std;

BOOST_AUTO_TEST_CASE(SharedInterfaceProviderTest_Slave) {
    auto &logger = log4cpp::Category::getInstance("SharedInterfaceProviderTest");
    wallaroo::Catalog c;
    c.Create("ifaceProvider", "InterfaceProvider", false);

    c.CheckWiring();
    c.Init();

    auto interfaceProvider = std::shared_ptr<common::bridge::InterfaceProvider>(c["ifaceProvider"]);
    auto *iface = interfaceProvider->getInterface();
    common::bridge::SharedScopedMutex lk(iface->mutex);

    BOOST_CHECK(interfaceProvider != nullptr);

    logger.info("Initial speed: %d.", iface->motor.left.getSpeed());

    iface->motor.left.setSpeed(5);

    BOOST_CHECK_EQUAL(1, 1);
}