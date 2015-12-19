/*
 * utils.hpp
 *
 *  Project: server
 *  Created on: 22 mar 2014
 *
 *  Copyright 2014 Michał Słomkowski m.slomkowski@gmail.com
 *
 *	This program is free software; you can redistribute it and/or modify it
 *	under the terms of the GNU General Public License version 3 as
 *	published by the Free Software Foundation.
 */

#pragma once

#include <functional>
#include <chrono>
#include <vector>
#include <string>
#include <sstream>

namespace common {
    namespace utils {

/**
* Measure the execution time of the function.
* @param func function object (pointer or lambda expression) matching the format void function()
* @return number of microseconds
*/
        template<typename TimeUnit>
        int measureTime(std::function<void()> func) {
            using std::chrono::duration_cast;
            using std::chrono::high_resolution_clock;

            auto timerBegin = high_resolution_clock::now();

            func();

            auto timerEnd = high_resolution_clock::now();

            return duration_cast<TimeUnit>(timerEnd - timerBegin).count();
        }

/**
* Gets the formatted date&time string with milliseconds.
* @return timestamp in format: 21:31:42.150
*/
        std::string getTimestamp();

/**
* Takes the vector of type given and returns the string with given format:
* {elem1,elem2,elem3}
* @param data vector of type T
* @return
*/
        template<typename T>
        std::string toString(std::vector<T> &data) {
            std::stringstream out;

            out << "{";

            for (T &elem : data) {
                out << std::dec << (int) elem << ",";
            }

            out << "}";

            return out.str();
        }

    }
}
