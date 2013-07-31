#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <math.h>
#include <stdarg.h>

#include "global.h"
#include "networking.h"
#include "extra_func.h"
#include "structures.h"
#include "watchdog.h"
#include "bridge.h"

// show detailed log information
#define DEBUG 1

// number of clients connected at the same time
#define MAXPENDING 5

static int server_fd = -1, connectio_fd = -1;
static bool terminateFlag = false;

static void parseServerCommands(const char *received, char *toSend);

int networking_setup(int port)
{
	struct sockaddr_in serv_addr;

	server_fd = socket(AF_INET, SOCK_STREAM, 0);
	
	if (server_fd < 0)
	{
		errf("networking: Failed to create a socket.");
		return ERROR;
	}

	bzero((char*)&serv_addr, sizeof(serv_addr)); // filling with zeros

	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = INADDR_ANY;
	serv_addr.sin_port = htons(port); // set the port

	if(bind(server_fd, (struct sockaddr*) &serv_addr, sizeof(serv_addr)) < 0) 
     {
		errf("networking: Failed binding socket.");
		close(server_fd);
		return ERROR;
	}
	
	listen(server_fd, MAXPENDING);

	return SUCCESS;
}

void networking_shutdown()
{
	imsgf("networking: shutting down.");
	terminateFlag = true;
	close(connectio_fd);
	close(server_fd);
}

void *networking_function(void *arg)
{
	char d_recv[MAX_DATA_LENGTH + 5]; // additional +5 just for safety
	char d_send[MAX_DATA_LENGTH + 5];
	
	socklen_t clilen;
	struct sockaddr_in cli_addr;

	int len;

	while(!terminateFlag)
	{
		clilen = sizeof(cli_addr);
		connectio_fd = accept(server_fd, (struct sockaddr *) &cli_addr, &clilen);

		if (connectio_fd < 0) 
	    	{
			errf("networking: Failed at ACCEPT.");
			terminateFlag = true;
			break;
		}
		
		bzero(d_send, MAX_DATA_LENGTH); // erase the buffers
		bzero(d_recv, MAX_DATA_LENGTH);

		// read commands from the client
		len = read(connectio_fd, d_recv, MAX_DATA_LENGTH);
		if(len < 0)
		{
			errf("networking: Error reading from socket.");
			close(connectio_fd);
			continue;
		}

		// limit the the valid data to printable characters (from 32 to 127)
		for(unsigned int r_idx = 0; r_idx < strlen(d_recv); r_idx++) if(((unsigned char)d_recv[r_idx] < 32) ||
				((unsigned char)d_recv[r_idx] > 127)) d_recv[r_idx] = '\0';

#if DEBUG
		msgf("networking: Command string received, length %d: \"%s\".", len, d_recv);
#endif
		// reset watchdog - every connection 
		watchdog_reset();

		// MOST IMPORTANT - COMMAND PARSER
		parseServerCommands(d_recv, d_send);

		len = write(connectio_fd, d_send, strlen(d_send));
		if (len < 0)
		{
			errf("networking: Error writing socket.");
			close(connectio_fd);
			continue;
		}
#if DEBUG
		msgf("networking: Data string sent: \"%s\".", d_send);
#endif
		
		close(connectio_fd);

		// SERVER COMMANDS - they are put after the connection finishes because
		// they don't give back any feedback and some of them are stopping the whole OS
#define setPerformExit() { main_command.lock(); main_command.performExit = true; main_command.unlock(); }

		for(unsigned int r_idx = 0; r_idx < strlen(d_recv); r_idx++) if(d_recv[r_idx] == C_SERVER)
		{
			switch(d_recv[r_idx + 1])
			{
				case C_SERVER_SHUTDOWN:
					imsgf("networking: Server shutdown called.");
					setPerformExit();				
		
					shutdown_machine();

					nexit(EXIT_SUCCESS);
					break;
				case C_SERVER_REBOOT:
					imsgf("networking: Server reboot called.");
					setPerformExit();

					reboot();

					nexit(EXIT_SUCCESS);
					break;
				case C_SERVER_EXIT:
					imsgf("networking: Exit called by the client.");
					setPerformExit();

					nexit(EXIT_SUCCESS);
					break;
			};
		}
	}

	return NULL;
}

// functions to help parse the commands

static void lightParse(char enDis, bool *lcommandSwitch, const char *text)
{
	if(enDis == C_LIGHTS_ENABLE)
	{
		*lcommandSwitch = true;
#if DEBUG
		msgf("networking: %s light enabled.", text);
#endif
	}
	else if(enDis == C_LIGHTS_DISABLE)
	{
		*lcommandSwitch = false;
#if DEBUG
		msgf("networking: %s light disabled.", text);
#endif
	}
	else errf("networking: Wrong light enable/disable character: %c.", enDis);
}

static void motorSetDirection(char dirChar, DIRECTION *lcommandDir, const char *text)
{
	switch(dirChar)
	{
	case D_FORWARD:  *lcommandDir = D_FORWARD;  break;
	case D_BACKWARD: *lcommandDir = D_BACKWARD; break; 
	case D_STOP:     *lcommandDir = D_STOP;     break; 
	default:
		*lcommandDir = D_STOP;
		errf("networking: Wrong direction command to %s, stopping it.", text);
	}
#if DEBUG
	msgf("networking: Set %s direction: %s.", text, direction_to_text(*lcommandDir)); 
#endif
}

static void motorSetSpeed(char speedChar, SPEED *lcommandSpeed, const char *text)
{
	if((speedChar >= C_VALUE_OFFSET_CHARACTER) && (speedChar < C_VALUE_OFFSET_CHARACTER + 16))
		*lcommandSpeed = speedChar - C_VALUE_OFFSET_CHARACTER;
	else errf("networking: Client tried to deliver wrong speed to %s: %c.", text, speedChar);

#if DEBUG
	msgf("networking: Set %s speed to %d.", text, *lcommandSpeed);
#endif
}

static void addToBuff(char *dest, int count, ...)
{
	va_list ap;
	char tab[15];

	va_start(ap, count);	

	for(int i = 0; i < count; i++) tab[i] = va_arg(ap, int);

	va_end(ap);

	strncat(dest, tab, count);
}

// macros
#define arm_wrong_joint_code() errf("networking: Client delivered wrong arm joint code: %c.", received[idx])
#define motor_wrong_code() errf("networking: Client deliverd wrong motor code: %c.", received[idx])

static void parseServerCommands(const char *received, char *toSend)
{
	// local copies of the structures
	SZARKCommand lcommand;
	SZARKStatus lstatus;

	int idx, rLen;

	rLen = strlen(received);

	lcommand.copy(main_command);

	// TODO możliwe, że trzeba to przerobić, by nie ustawiał status
	// EMERGENCY STOP 
	for(idx = 0; idx < rLen; idx++) if(received[idx] == C_EMERGENCY_STOP)
	{
		switch(received[idx + 1])
		{
		case C_EMERGENCY_STOP_ENABLE:
#if DEBUG
			msgf("networking: Activating the device.");
#endif
			lcommand.performEmergencyStop = false;

			main_status.lock(); // this has to be forced
			main_status.runlevel = OPERATIONAL;
			main_status.unlock();
			break;
		case C_EMERGENCY_STOP_DISABLE:
#if DEBUG
			msgf("networking: Deactivating the device.");
#endif
			lcommand.performEmergencyStop = true;

			main_status.lock();
			Runlevel runlevel = main_status.runlevel;
			main_status.init();
			if(runlevel == OPERATIONAL) main_status.runlevel = STOPPED_SOFT;
			else main_status.runlevel = runlevel;
			main_status.unlock();
			
			break;
		};

		break; // the for loop
	}

	// MOTORS
	for(idx = 0; idx < rLen; idx++) if(received[idx] == C_MOTORS)
	{
		idx += 2; // set focus on motor agrument
		switch(received[idx - 1]) // here back to command
		{
		case C_MOTORS_SET_SPEED:
			if(received[idx] == C_MOTORS_LEFT)
				motorSetSpeed(received[idx + 1], &lcommand.motors.left.speed, "left motor");
			else if(received[idx] == C_MOTORS_RIGHT)
				motorSetSpeed(received[idx + 1], &lcommand.motors.right.speed, "right motor");
			else motor_wrong_code();
			break;

		case C_MOTORS_SET_DIRECTION:
			if(received[idx] == C_MOTORS_LEFT)
				motorSetDirection(received[idx + 1], &lcommand.motors.left.direction, "left motor");
			else if(received[idx] == C_MOTORS_RIGHT)
				motorSetDirection(received[idx + 1], &lcommand.motors.right.direction, "right motor");
			else motor_wrong_code();
			break;

		case C_MOTORS_GET_DIRECTION:
			if(received[idx] == C_MOTORS_LEFT)
				addToBuff(toSend, 4, C_MOTORS, C_MOTORS_GET_DIRECTION, C_MOTORS_LEFT, lcommand.motors.left.direction);
			else if(received[idx] == C_MOTORS_RIGHT)
				addToBuff(toSend, 4, C_MOTORS, C_MOTORS_GET_DIRECTION, C_MOTORS_RIGHT, lcommand.motors.right.direction);
			else motor_wrong_code();
			break;
		
		case C_MOTORS_GET_SPEED:
			if(received[idx] == C_MOTORS_LEFT)
				addToBuff(toSend, 4, C_MOTORS, C_MOTORS_GET_SPEED, C_MOTORS_LEFT, lcommand.motors.left.speed + C_VALUE_OFFSET_CHARACTER);
			else if(received[idx] == C_MOTORS_RIGHT)
				addToBuff(toSend, 4, C_MOTORS, C_MOTORS_GET_SPEED, C_MOTORS_RIGHT, lcommand.motors.right.speed + C_VALUE_OFFSET_CHARACTER);
			else motor_wrong_code();
			break;
		};
	}

	// LIGHTS
	for(idx = 0; idx < rLen; idx++) if(received[idx] == C_LIGHTS)
	{
		switch(received[idx + 1])
		{
		case C_LIGHTS_GRIPPER:
			lightParse(received[idx + 2], &lcommand.lights.gripper, "Gripper");
			continue;
		case C_LIGHTS_LOW:
			lightParse(received[idx + 2], &lcommand.lights.low, "Low");
			continue;
		case C_LIGHTS_HIGH:
			lightParse(received[idx + 2], &lcommand.lights.high, "High");
			continue;
		case C_LIGHTS_CAMERA:
			lightParse(received[idx + 2], &lcommand.lights.camera, "Camera");
			continue;
		default:
			errf("networking: Client delivered wrong light symbol.");
		};
		idx += 2;
	}

	// LCD 
	for(idx = 0; idx < rLen - 1; idx++) if(received[idx] == C_LCD)
	{
		
		if(received[idx + 1] == C_LCD_SET) // if that was set lcd text command
		{
			idx += 2;
			// copying to the structure
			bzero(lcommand.lcdText, LCD_CHARACTERS);
			memccpy(lcommand.lcdText, &(received[idx]), C_LCD_TERM_CHAR, LCD_CHARACTERS);
			// deleting the END_CHARACTER
			int len = strlen(lcommand.lcdText);
			if(lcommand.lcdText[len - 1] == C_LCD_TERM_CHAR) lcommand.lcdText[len - 1] = '\0';
#if DEBUG
			msgf("networking: LCD text set to %s.", lcommand.lcdText);
#endif
			break;
		}
		else if(received[idx + 1] == C_LCD_GET)
		{
			addToBuff(toSend, 1, C_LCD);
			strncat(toSend, lcommand.lcdText, LCD_CHARACTERS);
			addToBuff(toSend, 1, C_LCD_TERM_CHAR);
			break;
		}
		else continue;
	}

	// copy status, because it will be needed for battery, atm position etc.
	lstatus.copy(main_status);

	// ARM
	for(idx = 0; idx < rLen; idx++) if(received[idx] == C_ARM)
	{
		idx += 2; // set focus on motor argument
		switch(received[idx - 1])
		{
		case C_ARM_SET_SPEED:
			switch(received[idx])
			{
			case C_ARM_GRIPPER:
				motorSetSpeed(received[idx + 1], &lcommand.arm.gripper.speed, "arm gripper");
				break;
			case C_ARM_WRIST:
				motorSetSpeed(received[idx + 1], &lcommand.arm.wrist.speed, "arm wrist");
				break;	
			case C_ARM_ELBOW:
				motorSetSpeed(received[idx + 1], &lcommand.arm.elbow.speed, "arm elbow");
				break;
			case C_ARM_SHOULDER:
				motorSetSpeed(received[idx + 1], &lcommand.arm.shoulder.speed, "arm shoulder");
				break;
			default:
				arm_wrong_joint_code();	
			};
		break;

		case C_ARM_GET_SPEED:
			switch(received[idx])
			{
			case C_ARM_GRIPPER:
				addToBuff(toSend, 4, C_ARM, C_ARM_GET_SPEED, C_ARM_GRIPPER, lcommand.arm.gripper.speed + C_VALUE_OFFSET_CHARACTER);
				break;
			case C_ARM_ELBOW:
				addToBuff(toSend, 4, C_ARM, C_ARM_GET_SPEED, C_ARM_ELBOW, lcommand.arm.elbow.speed + C_VALUE_OFFSET_CHARACTER);
				break;
			case C_ARM_SHOULDER:
				addToBuff(toSend, 4, C_ARM, C_ARM_GET_SPEED, C_ARM_SHOULDER, lcommand.arm.shoulder.speed + C_VALUE_OFFSET_CHARACTER);
				break;
			case C_ARM_WRIST:
				addToBuff(toSend, 4, C_ARM, C_ARM_GET_SPEED, C_ARM_WRIST, lcommand.arm.wrist.speed + C_VALUE_OFFSET_CHARACTER);
				break;
			default:
				arm_wrong_joint_code();	
			}
			break;
			
		case C_ARM_GET_DIRECTION:
			switch(received[idx])
			{
			case C_ARM_GRIPPER:
				addToBuff(toSend, 4, C_ARM, C_ARM_GET_DIRECTION, C_ARM_GRIPPER, lstatus.armDirections.gripper);
				break;
			case C_ARM_ELBOW:
				addToBuff(toSend, 4, C_ARM, C_ARM_GET_DIRECTION, C_ARM_ELBOW, lstatus.armDirections.elbow);
				break;
			case C_ARM_SHOULDER:
				addToBuff(toSend, 4, C_ARM, C_ARM_GET_DIRECTION, C_ARM_SHOULDER, lstatus.armDirections.shoulder);
				break;
			case C_ARM_WRIST:
				addToBuff(toSend, 4, C_ARM, C_ARM_GET_DIRECTION, C_ARM_WRIST, lstatus.armDirections.wrist);
				break;
			default:
				arm_wrong_joint_code();	
			}
			break;
		
		case C_ARM_SET_DIRECTION:
			switch(received[idx])
			{
			case C_ARM_GRIPPER:
				motorSetDirection(received[idx + 1], &lcommand.arm.gripper.direction, "arm gripper");
				break;
			case C_ARM_WRIST:
				motorSetDirection(received[idx + 1], &lcommand.arm.wrist.direction, "arm wrist");
				break;	
			case C_ARM_ELBOW:
				motorSetDirection(received[idx + 1], &lcommand.arm.elbow.direction, "arm elbow");
				break;
			case C_ARM_SHOULDER:
				motorSetDirection(received[idx + 1], &lcommand.arm.shoulder.direction, "arm shoulder");
				break;
			default:
				arm_wrong_joint_code();	
			}
			break;
		


		// TODO pozycję narazie zostawiamy
		case C_ARM_SET_POSITION:
			// big black hole here
			break;

		case C_ARM_GET_POSITION: // TODO tutaj jest problem, bo status nie jest blokowany czy cuś
			switch(received[idx])
			{
			case C_ARM_GRIPPER:
				addToBuff(toSend, 5, C_ARM, C_ARM_GET_POSITION, C_ARM_GRIPPER,
						(lstatus.armPositions.gripper / 16) + C_VALUE_OFFSET_CHARACTER,
						(lstatus.armPositions.gripper % 16) + C_VALUE_OFFSET_CHARACTER);
				break;
			case C_ARM_ELBOW:
				addToBuff(toSend, 5, C_ARM, C_ARM_GET_POSITION, C_ARM_ELBOW,
						(lstatus.armPositions.elbow / 16) + C_VALUE_OFFSET_CHARACTER,
						(lstatus.armPositions.elbow % 16) + C_VALUE_OFFSET_CHARACTER);
				break;
			case C_ARM_SHOULDER:
				addToBuff(toSend, 5, C_ARM, C_ARM_GET_POSITION, C_ARM_SHOULDER,
						(lstatus.armPositions.shoulder / 16) + C_VALUE_OFFSET_CHARACTER,
						(lstatus.armPositions.shoulder % 16) + C_VALUE_OFFSET_CHARACTER);
				break;
			case C_ARM_WRIST:
				addToBuff(toSend, 5, C_ARM, C_ARM_GET_POSITION, C_ARM_WRIST,
						(lstatus.armPositions.wrist / 16) + C_VALUE_OFFSET_CHARACTER,
						(lstatus.armPositions.wrist % 16) + C_VALUE_OFFSET_CHARACTER);
				break;
			default:
				arm_wrong_joint_code();	
			}
			break;
		};
	}

	// BATTERY
	for(idx = 0; idx < rLen - 1; idx++) if((received[idx] == C_BATTERY) && (received[idx + 1] == C_BATTERY_GET))
	{
		// place values into output
		addToBuff(toSend, 6, C_BATTERY, C_BATTERY_GET,
		(char)(lstatus.battery.voltage) + C_VALUE_OFFSET_CHARACTER,
		(char)roundf(10 * fmodf(lstatus.battery.voltage, 1.0)) + C_VALUE_OFFSET_CHARACTER,
		(char)(lstatus.battery.current) + C_VALUE_OFFSET_CHARACTER,
		(char)roundf(10 * fmodf(lstatus.battery.current, 1.0)) + C_VALUE_OFFSET_CHARACTER);
#if DEBUG
		msgf("networking: Placed battery data: %2.2fV, %2.2fA.", lstatus.battery.voltage, lstatus.battery.current); 
#endif
		break;
	}

	// fill the output with the additional information
	
	if(lstatus.runlevel == STOPPED_HARD) addToBuff(toSend, 2, C_EMERGENCY_STOP, C_EMERGENCY_STOP_HARDWARE);
	else if(lstatus.runlevel == STOPPED_SOFT)  addToBuff(toSend, 2, C_EMERGENCY_STOP, C_EMERGENCY_STOP_DISABLE);

	main_command.lock();
	main_command.copy(lcommand, true);
	main_command.unlock();
}

