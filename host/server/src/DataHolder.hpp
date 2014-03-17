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
#include <boost/utility/enable_if.hpp>

#include "usb-commands.hpp"

namespace bridge {

	/**
	 * Helpful class used by Interface class to hold each USB request. The class is immutable.
	 */
	class DataHolder {
	private:
		uint8_t* data = nullptr;
		unsigned int length = 0;

		void initData(USBCommands::Request request, unsigned int dataSize = 0) {
			length = dataSize + 1;
			data = new uint8_t[length];
			data[0] = request;
		}
	public:
		/**
		 * Creates single-byte request.
		 * @param request
		 */
		DataHolder(USBCommands::Request request) {
			initData(request);
		}

		~DataHolder() {
			delete[] data;
		}

		/**
		 * Creates the request from the given structure. The structure content is copied byte by byte.
		 * @param request
		 * @param structure any data type except std::vector.
		 */
		template<typename TYPE,
			typename = typename std::enable_if<not std::is_same<TYPE, std::vector<uint8_t>>::value>::type>
		DataHolder(USBCommands::Request request, TYPE& structure) {
			initData(request, sizeof(TYPE));

			std::memcpy(data + 1, &structure, sizeof(TYPE));
		}

		/**
		 * Creates the request from the vector of bytes. The vector's data is copied into request.
		 * @param request
		 * @param vec
		 */
		DataHolder(USBCommands::Request request, const std::vector<uint8_t>& vec) {
			initData(request, vec.size());

			unsigned int idx = 1;
			for (auto& val : vec) {
				data[idx] = val;
				idx++;
			}
		}

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
		uint8_t* getPlainData() {
			return data;
		}

		/**
		 * Returns the pointer to the payload (all data except USB request number) casted on the type given.
		 * @return
		 */
		template<typename TYPE> TYPE* getPayload() {
			return reinterpret_cast<TYPE*>(data + 1);
		}

		USBCommands::Request getRequest() {
			return static_cast<USBCommands::Request>(data[0]);
		}

		unsigned int getSize() {
			return length;
		}

		/**
		 * Appends data to the given vector instance.
		 * @param vec existing std::vector<uint8_t> instance.
		 */
		void appendTo(std::vector<uint8_t>& vec) {
			vec.insert(vec.end(), data, data + length);
		}
	};
}

#endif /* DATAHOLDER_HPP_ */
