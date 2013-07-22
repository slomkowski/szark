/*
 * controller.h
 *
 *  Created on: 17-07-2013
 *      Author: michal
 */

#ifndef CONTROLLER_H_
#define CONTROLLER_H_

namespace motor {
	// PD controller constants:
	// proportional
	const int16_t CONTROLLER_P_REG = 60;
	// derivative
	const int16_t CONTROLLER_D_REG = 20;

	uint8_t regulate(Motor *motor, uint8_t actualPwmReg);
}

#endif /* CONTROLLER_H_ */
