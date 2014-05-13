/*
 * DataHolder.hpp
 *
 *  Created on: 23-08-2013
 *      Author: Michał Słomkowski
 */

#ifndef DATAHOLDER_HPP_
#define DATAHOLDER_HPP_

#include <typeinfo>
#include <cstring>
#include <vector>
#include <memory>
#include <boost/utility/enable_if.hpp>

#include "usb-commands.hpp"

namespace bridge {

const int DATAHOLDER_MAX_DATA_SIZE = 40;

/**
 * Helpful class used by Interface class to hold each USB request. The class is immutable, only data can change.
 */
class DataHolder {
public:
	/**
	 * Creates single-byte request.
	 * @param request
	 * @param pr priority of the request
	 */
	DataHolder(const USBCommands::Request request, const int pr);

	/**
	 * Creates two-byte request.
	 * @param request
	 * @param pr priority of the request
	 * @param d
	 */
	DataHolder(const USBCommands::Request request, const int pr, const uint8_t byte);

	DataHolder(const DataHolder &dh);

	/**
	 * Creates the request from the given structure. The structure content is copied byte by byte.
	 * @param request
	 * @param pr priority of the request
	 * @param structure any data type except std::vector.
	 */
	template<typename TYPE,
			typename = typename std::enable_if<not std::is_same<TYPE, std::vector<uint8_t>>::value>::type>
	DataHolder(const USBCommands::Request request, const int pr, TYPE& structure) {
		static_assert(DATAHOLDER_MAX_DATA_SIZE >= sizeof(TYPE) + 1, "Should be at least of the size of the structure.");

		initData(request, pr, sizeof(TYPE));

		std::memcpy(data + 1, &structure, sizeof(TYPE));
	}

	DataHolder(const USBCommands::Request request, const int pr, void* data, int size);

	/**
	 * Creates the request from the vector of bytes. The vector's data is copied into request.
	 * @param request
	 * @param pr priority of the request
	 * @param vec
	 */
	DataHolder(const USBCommands::Request request, const int pr, const std::vector<uint8_t>& vec);

	DataHolder& operator=(const DataHolder &dh);

	/**
	 * Helper method, creates the \ref{DataHolder} instance and passes the arguments to the appropriate constructor.
	 * @param args
	 * @return shared_ptr to the \ref{DataHolder} instance.
	 */
	template<typename ... ARGS> static std::shared_ptr<DataHolder> create(ARGS ... args) {
		return std::shared_ptr<DataHolder>(new DataHolder(args...));
	}

	/**
	 * Returns the contained data along with the request number.
	 * @return pointer to the stored data.
	 */
	const uint8_t* getPlainData() const {
		return data;
	}

	/**
	 * Returns the pointer to the payload (all data except USB request number) casted on the type given.
	 * @return
	 */
	template<typename TYPE> TYPE* getPayload() const {
		return reinterpret_cast<TYPE*>(data + 1);
	}

	const USBCommands::Request getRequest() const {
		return static_cast<const USBCommands::Request>(data[0]);
	}

	unsigned int getSize() const {
		return length;
	}

	int getPriority() const {
		return priority;
	}

	/**
	 * Appends data to the given vector instance.
	 * @param vec existing std::vector<uint8_t> instance.
	 */
	void appendTo(std::vector<uint8_t>& vec) const {
		vec.insert(vec.end(), data, data + length);
	}

	/**
	 * Checks if the object is equal to the given one.
	 * @param right
	 * @return
	 */
	bool equals(const DataHolder &right);

private:
	mutable uint8_t data[DATAHOLDER_MAX_DATA_SIZE];
	unsigned int length = 0;
	int priority = 0;

	static_assert(DATAHOLDER_MAX_DATA_SIZE >= 2, "Should be at least one byte.");

	void initData(USBCommands::Request request, const int pr, unsigned int dataSize);
};

}

#endif /* DATAHOLDER_HPP_ */
