#include "SharedInterfaceProvider.hpp"

#include <boost/interprocess/managed_shared_memory.hpp>

namespace common {
    namespace bridge {
        const std::string SHARED_MEM_SEGMENT_NAME = "SZARK_Interface_shm";
        const int SHARED_MEM_SIZE = 0xffff;

        WALLAROO_REGISTER(InterfaceProvider, bool);

        using namespace boost::interprocess;

        struct _InterfaceProviderImpl {
            _InterfaceProviderImpl(const bool isMaster)
                    : logger(log4cpp::Category::getInstance("InterfaceProvider")),
                      master(isMaster) { }

            void openSharedMemory() {
                const char *OBJECT_NAME = "Interface";
                if (master) {
                    logger.info("Cleaning previous instance of shared segment '%s' from system.",
                                SHARED_MEM_SEGMENT_NAME.c_str());

                    shared_memory_object::remove(SHARED_MEM_SEGMENT_NAME.c_str());

                    logger.info("Allocating %d bytes of shared memory with name '%s'.",
                                SHARED_MEM_SIZE, SHARED_MEM_SEGMENT_NAME.c_str());

                    memorySegment = new managed_shared_memory(create_only, SHARED_MEM_SEGMENT_NAME.c_str(),
                                                              SHARED_MEM_SIZE);

                    logger.info("Creating shared object '%s'.", OBJECT_NAME);
                    interface = memorySegment->construct<Interface>(OBJECT_NAME)();
                } else {
                    try {
                        memorySegment = new managed_shared_memory(open_only, SHARED_MEM_SEGMENT_NAME.c_str());

                        size_t interfaceSize;
                        logger.info("Opening shared object '%s'.", OBJECT_NAME);
                        std::tie(this->interface, interfaceSize) = memorySegment->find<Interface>(OBJECT_NAME);

                    } catch (interprocess_exception &exp) {
                        logger.error("Cannot initialize shared object Interface: %s. getInterface() will return NULL.",
                                     exp.what());
                        interface = nullptr;
                    }
                }
            }

            void closeSharedMemory() {
                if (master) {
                    memorySegment->destroy_ptr(interface);
                }

                delete memorySegment;

                if (master) {
                    logger.notice("Removing shared memory segment '%s' from system.",
                                  SHARED_MEM_SEGMENT_NAME.c_str());
                    shared_memory_object::remove(SHARED_MEM_SEGMENT_NAME.c_str());
                }
            }

            log4cpp::Category &logger;
            const bool master;
            Interface *interface;
            managed_shared_memory *memorySegment;
        };

        InterfaceProvider::InterfaceProvider(const bool &isMaster) {
            impl = new _InterfaceProviderImpl(isMaster);
        }

        void InterfaceProvider::Init() {
            impl->openSharedMemory();
            impl->logger.notice("Instance created.");
        }

        InterfaceProvider::~InterfaceProvider() {
            impl->closeSharedMemory();
            impl->logger.notice("Instance destroyed.");
            delete impl;
        }

        Interface *InterfaceProvider::getInterface() {
            return impl->interface;
        }

        bool InterfaceProvider::isValid() {
            return (impl->interface != nullptr) and (impl->interface->isObjectActive());
        }
    }
}