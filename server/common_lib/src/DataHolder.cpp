#include "DataHolder.hpp"

using namespace common::bridge;

common::bridge::DataHolder::DataHolder() {
    this->_length = 0;
    this->_priority = 0;
    this->_killswitchDependent = false;
}

common::bridge::DataHolder::DataHolder(const USBCommands::Request request, const int pr, bool killswitchDependent) {
    initData(request, pr, killswitchDependent, 0);
}

common::bridge::DataHolder::DataHolder(const USBCommands::Request request, const int pr, bool killswitchDependent,
                                       const uint8_t byte) {
    initData(request, pr, killswitchDependent, sizeof(char));

    _data[1] = byte;
}

common::bridge::DataHolder::DataHolder(const USBCommands::Request request, const int pr, bool killswitchDependent,
                                       const std::vector<uint8_t> &vec) {
    initData(request, pr, killswitchDependent, vec.size());

    unsigned int idx = 1;
    for (auto &val : vec) {
        _data[idx] = val;
        idx++;
    }
}

common::bridge::DataHolder::DataHolder(const DataHolder &dh) {
    this->_length = dh._length;
    this->_priority = dh._priority;
    this->_killswitchDependent = dh._killswitchDependent;
    std::memcpy(this->_data, dh._data, _length + 1);
}

common::bridge::DataHolder::DataHolder(const USBCommands::Request request, const int pr, bool killswitchDependent,
                                       void *data,
                                       int size) {
    initData(request, pr, killswitchDependent, size);
    std::memcpy(this->_data + 1, data, _length);
}

DataHolder &common::bridge::DataHolder::operator=(const DataHolder &dh) {
    this->_length = dh._length;
    this->_priority = dh._priority;
    this->_killswitchDependent = dh._killswitchDependent;
    std::memcpy(this->_data, dh._data, _length + 1);

    return *this;
}

bool common::bridge::DataHolder::equals(const DataHolder &right) {

    if (this->_priority != right._priority) {
        return false;
    }

    if (this->_length != right._length) {
        return false;
    }

    if (this->_killswitchDependent != right._killswitchDependent) {
        return false;
    }

    if (std::memcmp(this->_data, right._data, this->_length + 1) != 0) {
        return false;
    }

    return true;
}

void common::bridge::DataHolder::initData(USBCommands::Request request, const int pr, bool killswitchDependent,
                                          unsigned int dataSize) {
    _priority = pr;
    _length = dataSize + 1;
    _data[0] = request;
    this->_killswitchDependent = killswitchDependent;
}
