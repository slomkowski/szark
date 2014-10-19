#ifndef CONFIGURATION_HPP_
#define CONFIGURATION_HPP_

#include <string>
#include <stdexcept>
#include <map>

#include <boost/noncopyable.hpp>
#include <log4cpp/Category.hh>
#include <wallaroo/registered.h>

namespace log4cpp {

	class Properties : public std::map<std::string, std::string> {

	public:

		Properties();

		virtual ~Properties();

		virtual void load(std::istream &in);

		virtual void save(std::ostream &out);

		virtual int getInt(const std::string &property, int defaultValue);

		virtual bool getBool(const std::string &property, bool defaultValue);

		virtual std::string getString(const std::string &property,

				const char *defaultValue);

	protected:

		virtual void _substituteVariables(std::string &value);

	};

}

namespace common {
	namespace config {

		class ConfigException : public std::runtime_error {
		public:
			ConfigException(const std::string &message)
					: std::runtime_error(message) {
			}
		};

		class Configuration : boost::noncopyable, public wallaroo::Device {
		public:
			Configuration(const std::string fileName);

			Configuration(const std::vector<std::string> fileNames);

			int getInt(const std::string &property);

			bool getBool(const std::string &property);

			std::string getString(const std::string &property);

			log4cpp::Properties prop;
		private:

			void checkKey(const std::string &property);
		};
	}
}

#endif /* CONFIGURATION_HPP_ */
