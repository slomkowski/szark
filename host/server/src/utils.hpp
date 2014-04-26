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
#ifndef UTILS_HPP_
#define UTILS_HPP_

#include <functional>
#include <chrono>

namespace utils {

/**
 * Measure the execution time of the function.
 * @param func function object (pointer or lambda expression) matching the format void function()
 * @return number of microseconds
 */
std::chrono::microseconds measureTime(std::function<void()> func);

} /* namespace utils */

#endif /* UTILS_HPP_ */
