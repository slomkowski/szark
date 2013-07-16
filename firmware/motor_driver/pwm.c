#include "global.h"

#include <avr/io.h>
#include <inttypes.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <avr/cpufunc.h>
#include <avr/pgmspace.h>

#include <stdlib.h>

#include "pwm.h"

#include "uart.h"

#define pwm1_start_generating() { TCCR0A |= (1<<COM0A1); motor1_counter = 0; GIMSK |= (1<<INT0);}
#define pwm1_stop_generating()  { TCCR0A &= ~(1<<COM0A1); GIMSK &= ~(1<<INT0); }

#define pwm2_start_generating() { TCCR0A |= (1<<COM0B1); motor2_counter = 0; GIMSK |= (1<<INT1); }
#define pwm2_stop_generating()  { TCCR0A &= ~(1<<COM0B1); GIMSK &= ~(1<<INT1); }

#define PWM1_REG OCR0A
#define PWM2_REG OCR0B

// interrupt interval - the base is (f_clk/1024) ~= 0.25ms
// current setting - one interrupt per 125ms
#define TIMER_INTERVAL 500

/*
 * in my application, the maximal speed was limited to 70 impulses per interrupt
 * this is the table of valid speeds (number of impulses in TIMER_INTERVAL time)
 */
//const uint8_t s_vals[] PROGMEM = { 0, 5, 10, 14, 18, 23, 28, 33, 37, 42, 46, 51, 56, 61, 65, 70 };
const uint16_t s_vals[] PROGMEM = { 0, 10, 20, 28, 36, 46, 54, 66, 74, 84, 92, 102, 112, 122, 130, 140 };



static uint8_t motor1_direction;
static uint8_t motor2_direction;

static uint8_t motor1_ref;
static uint8_t motor2_ref;

static volatile uint16_t motor1_counter;
static volatile uint16_t motor2_counter;

void motor_init()
{
	// set outputs
	DDR(MOTOR1_PORT) |= (1<<MOTOR1_FORWARD_PIN) | (1<<MOTOR1_BACKWARD_PIN) | (1<<MOTOR1_PWM_PIN);
	DDR(MOTOR2_PORT) |= (1<<MOTOR2_FORWARD_PIN) | (1<<MOTOR2_BACKWARD_PIN) | (1<<MOTOR2_PWM_PIN);

	//set encoders inputs & pullups
	DDR(ENCODER_PORT) &= ~(1<<ENCODER1_PIN) & ~(1<<ENCODER2_PIN);
	PORT(ENCODER_PORT) |= (1<<ENCODER1_PIN) | (1<<ENCODER2_PIN);
	
	// brake motors
	motor_set_direction(MOTOR1_IDENTIFIER, MOTOR_STOP);
	motor_set_direction(MOTOR2_IDENTIFIER, MOTOR_STOP);

	// pwm setup
	TCCR0A = (1<<WGM00); // phase-correct
	TCCR0B = (1<<CS01) | (1<<CS00); // prescaler clkio/64

	// external interrupts setup - for encoders support
	MCUCR |= (1<<ISC10) | (1<<ISC00); // enable interrupts at both rising and falling edges 

	// setup of the interrupt timer, 
	// Timer1 -- CTC mode, enable overflow interrupt, prescaler f_clk/1024 
	TCCR1A = 0;
	TCCR1B = (1<<WGM12) | (1<<CS12) | (1<<CS10); 


	TIMSK |= (1<<OCIE1A);
     OCR1A = TIMER_INTERVAL;
}

// returned value is the actual PWM setting, which changes by encoder feedback, not the one set by the user
uint8_t motor_get_speed(uint8_t motor)
{
	if(motor == MOTOR1_IDENTIFIER) return PWM1_REG;
		else return PWM2_REG;
}

uint8_t motor_get_direction(uint8_t motor)
{
	if(motor == MOTOR1_IDENTIFIER) return motor1_direction;
		else return motor2_direction;
}

void motor_set_speed(uint8_t motor, uint8_t speed)
{
	if(motor == MOTOR1_IDENTIFIER)
	{
#if DEBUG
		if(speed == '0') motor1_ref = pgm_read_byte(&(s_vals[0]));
		else if(speed == 'm') motor1_ref = pgm_read_byte(&(s_vals[15]));
		else 
#endif
		motor1_ref = pgm_read_byte(&(s_vals[speed % 16]));
	}
	else
	{
#if DEBUG
		if(speed == '0') motor2_ref = pgm_read_byte(&(s_vals[0]));
		else if(speed == 'm') motor2_ref =pgm_read_byte(&(s_vals[15]));
		else
#endif
		motor2_ref = pgm_read_byte(&(s_vals[speed % 16]));
	}
}

void motor_set_direction(uint8_t motor, uint8_t direction)
{
	// motor 1
	if(motor == MOTOR1_IDENTIFIER)
	{
		if(direction == MOTOR_FORWARD)
		{
			PORT(MOTOR1_PORT) |= (1<<MOTOR1_FORWARD_PIN);
			PORT(MOTOR1_PORT) &= ~(1<<MOTOR1_BACKWARD_PIN);

			pwm1_start_generating();
			motor1_direction = MOTOR_FORWARD;
		}
		else if(direction == MOTOR_BACKWARD)
		{
			PORT(MOTOR1_PORT) &= ~(1<<MOTOR1_FORWARD_PIN);
			PORT(MOTOR1_PORT) |= (1<<MOTOR1_BACKWARD_PIN);

			pwm1_start_generating();
			motor1_direction = MOTOR_BACKWARD;
		}
		else
		{
			// braking
			PORT(MOTOR1_PORT) |= (1<<MOTOR1_FORWARD_PIN);
			PORT(MOTOR1_PORT) |= (1<<MOTOR1_BACKWARD_PIN);

			pwm1_stop_generating();
			motor1_direction = MOTOR_STOP;
		}
	}
	else // motor 2
	{
		if(direction == MOTOR_FORWARD)
		{
			PORT(MOTOR2_PORT) |= (1<<MOTOR2_FORWARD_PIN);
			PORT(MOTOR2_PORT) &= ~(1<<MOTOR2_BACKWARD_PIN);

			pwm2_start_generating();
			motor2_direction = MOTOR_FORWARD;
		}
		else if(direction == MOTOR_BACKWARD)
		{
			PORT(MOTOR2_PORT) &= ~(1<<MOTOR2_FORWARD_PIN);
			PORT(MOTOR2_PORT) |= (1<<MOTOR2_BACKWARD_PIN);

			pwm2_start_generating();
			motor2_direction = MOTOR_BACKWARD;
		}
		else
		{
			// braking
			PORT(MOTOR2_PORT) |= (1<<MOTOR2_FORWARD_PIN);
			PORT(MOTOR2_PORT) |= (1<<MOTOR2_BACKWARD_PIN);

			pwm2_stop_generating();
			motor2_direction = MOTOR_STOP;
		}
	}
}

/* regulator support code */

ISR(ENCODER1_INTERRUPT)	
{
	motor1_counter++;
}

ISR(ENCODER2_INTERRUPT)
{
	motor2_counter++;
}

#define P_REG 60
#define D_REG 30
// REGULATOR PD

ISR(TIMER1_COMPA_vect)
{
	int16_t err1, err2, out1, out2;
	static int16_t prevErr1, prevErr2;

	if(motor1_direction != MOTOR_STOP)
	{
		err1 = motor1_counter - motor1_ref;
		
		out1 = PWM1_REG - ( (P_REG *  err1 / 100) + (D_REG * (err1 - prevErr1) / 100));

		prevErr1 = err1;

		if(out1 < 0) PWM1_REG = 0;
		else if(out1 > 0xff) PWM1_REG = 0xff;
		else PWM1_REG = out1;
	}
	else prevErr1 = 0;

	if(motor2_direction != MOTOR_STOP)
	{
		err2 = motor2_counter - motor2_ref;
		
		out2 = PWM2_REG - ( (P_REG *  err2 / 100) + (D_REG * (err2 - prevErr2) / 100));

		prevErr2 = err2;

		if(out1 < 0) PWM1_REG = 0;
		else if(out1 > 0xff) PWM1_REG = 0xff;
		else PWM1_REG = out1;
		
		if(out2 < 0) PWM2_REG = 0;
		else if(out2 > 0xff) PWM2_REG = 0xff;
		else PWM2_REG = out2;
	}
	else prevErr2 = 0;

#if DEBUG
	uart_putc(motor2_counter);
#endif

	// clear the counters
	motor1_counter = 0;
	motor2_counter = 0;

	//debug_down();
}




