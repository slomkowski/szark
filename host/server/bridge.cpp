#include <string.h>
#include <errno.h>
#include <SerialStream.h>

#include "extra_func.h"
#include "global.h"
#include "bridge.h"

#define DEBUG 1

using namespace LibSerial;

// uart file descriptor
static SerialStream ser;

static char i2c_expander_byte;

static void putc_direction(DIRECTION d);
static void send_expander(char data);
static void getDirectionFromData(DIRECTION *joint, unsigned char *data);

static bool terminateFlag = false;

int b_setup(const char *device, int baudrate)
{
	// *** 1. setting up serial port
	
	imsgf("bridge: Opening UART port %s.", device);

	ser.Open(device);

	if(!ser.IsOpen())
	{
		errf("bridge: Could not open the device: %s!", device);
		return ERROR;
	}
	
	switch(baudrate)
	{
		case 300:    ser.SetBaudRate(SerialStreamBuf::BAUD_300); break;
		case 600:    ser.SetBaudRate(SerialStreamBuf::BAUD_600); break;
		case 1200:   ser.SetBaudRate(SerialStreamBuf::BAUD_1200); break;
		case 2400:   ser.SetBaudRate(SerialStreamBuf::BAUD_2400); break;
		case 4800:   ser.SetBaudRate(SerialStreamBuf::BAUD_4800); break;
		case 9600:   ser.SetBaudRate(SerialStreamBuf::BAUD_9600); break;
		case 19200:  ser.SetBaudRate(SerialStreamBuf::BAUD_19200); break;
		case 38400:  ser.SetBaudRate(SerialStreamBuf::BAUD_38400); break;
		case 57600:  ser.SetBaudRate(SerialStreamBuf::BAUD_57600); break;
		case 115200: ser.SetBaudRate(SerialStreamBuf::BAUD_115200); break;
#if DEBUG
		default:
			errf("bridge: Illegal baudrate set!");
			return ERROR;
#endif
	}

	// *** 2. initializing device
	b_enable();

	main_status.lock();
	main_status.runlevel = OPERATIONAL;
	main_status.unlock();


	return SUCCESS;
}


#define MEAN_ELEMENTS 6

void *b_data_listener_function(void *arg)
{
	// used to receive data
	unsigned char rec; // single received character
	unsigned char inc_data[5];

	// used to calculate mean of measurements 
	unsigned int volt_raw, curr_raw, index = 1, index_prev = 0;
	unsigned int volt_array[MEAN_ELEMENTS], curr_array[MEAN_ELEMENTS];

	bzero(volt_array, sizeof(volt_array));
	bzero(curr_array, sizeof(curr_array));

	/* in this loop there should not be any stdio-related commands - they cause latency, which hangs the listener and data is received improperly */
	while(!terminateFlag)
	{
		ser >> rec;
	
		if(rec == RS_BATTERY) // battery receive
		{
			// get battery measurements - 4 bytes (two for voltage and two for current)
			//ser.read(inc_data, 4);
			for(int i = 0; i < 4; i++) ser >> inc_data[i];

			if(index == MEAN_ELEMENTS) index = 0;
			if(index_prev == MEAN_ELEMENTS) index_prev = 0;

			volt_array[index] = (inc_data[1] << 8) | inc_data[0];
			curr_array[index] = (inc_data[3] << 8) | inc_data[2];
			
			// elimination of wrong measurements - noise protection
			if(volt_array[index] > (16 * BATTERY_VOLTAGE_DIVIDING_FACTOR)) volt_array[index] = volt_array[index_prev];
			if(curr_array[index] > (16 * BATTERY_CURRENT_DIVIDING_FACTOR)) curr_array[index] = curr_array[index_prev];

			index++;
			index_prev++;

			// calculate mean
			volt_raw = 0;
			curr_raw = 0;
			for(int i = 0; i < MEAN_ELEMENTS; i++)
			{
				volt_raw += volt_array[i];
				curr_raw += curr_array[i];
			}
			volt_raw /= MEAN_ELEMENTS;
			curr_raw /= MEAN_ELEMENTS;

			main_status.lock();
			main_status.battery.voltage = (float)volt_raw / BATTERY_VOLTAGE_DIVIDING_FACTOR;
			main_status.battery.current = (float)curr_raw / BATTERY_CURRENT_DIVIDING_FACTOR;
			main_status.unlock();

		}
		else if(rec == RS_HARDWARE_STOP)
		{
			main_status.lock();
			Runlevel runlevel = main_status.runlevel;
			main_status.unlock();

			if(runlevel == OPERATIONAL)
			{
				int i;
				// still 3 to get
				for(i = 0; i < 2; i++)
				{
					ser >> inc_data[i];
					if(inc_data[i] != RS_HARDWARE_STOP) break;
				}	
				if(i == 2) // if loop above has all iterations
				{
					main_command.lock();
					main_command.performEmergencyStop = true;
					main_command.unlock();

					main_status.lock();
					main_status.runlevel = STOPPED_HARD;
					main_status.unlock();
				
					imsgf("bridge: BRIDGE STOPPED BY EMERGENCY BUTTON!");
				}
			}
		}
		else if(rec == RS_ARM)
		{
			ser >> rec;

			if(rec == RS_ARM_GET_ALL_POSITIONS)
			{
				for(int i = 0; i < 4; i++) ser >> inc_data[i];

				main_status.lock();
				main_status.armPositions.shoulder = inc_data[0];
				main_status.armPositions.elbow = inc_data[1];
				main_status.armPositions.wrist = inc_data[2];
				main_status.armPositions.gripper = inc_data[3];
				main_status.unlock();

			}
			else if(rec == RS_ARM_GET_ALL_DIRECTIONS)
			{
				for(int i = 0; i < 4; i++) ser >> inc_data[i];

				main_status.lock();
				getDirectionFromData(&main_status.armDirections.shoulder, &inc_data[0]);
				getDirectionFromData(&main_status.armDirections.elbow, &inc_data[1]);
				getDirectionFromData(&main_status.armDirections.wrist, &inc_data[2]);
				getDirectionFromData(&main_status.armDirections.gripper, &inc_data[3]);
				main_status.unlock();
			}
		}
	}

	return NULL;
}

static void getDirectionFromData(DIRECTION *joint, unsigned char *data)
{
	if(*data == RS_A_FORWARD) *joint = D_FORWARD;
	else if(*data == RS_A_BACKWARD) *joint = D_BACKWARD;
	else if(*data == RS_A_STOP) *joint = D_STOP;
}

void *b_data_sender_function(void *arg)
{
	SZARKCommand lcommand, lcommand_old; // local copy of the global one

	//Runlevel runlevel;

	while(!terminateFlag)
	{
		usleep(BRIDGE_TRANSFERER_INTERVAL * 1000); // BRIDGE_TRANSFERER_INTERVAL is in ms

		//firstly, copy the global structure to the local one to minimize the time, when the global one is locked

		// XXX USING MAIN_COMMAND
		main_command.lock();
		
		lcommand.copy(main_command, true); // bez blokowania
		
		if(main_command.performExit)
		{
			main_command.unlock();
			b_shutdown();
			return NULL;
		}
		if(lcommand.performEmergencyStop)
		{
			// clear command table
			main_command.init();
			main_command.performEmergencyStop = true;	
			memcpy(&main_command.lcdText, &lcommand.lcdText, LCD_CHARACTERS + 1); // lcd display stays the same
		}
		main_command.unlock();

		// XXX END USING MAIN_COMMAND
		
		if(lcommand.performEmergencyStop && !lcommand_old.performEmergencyStop) // when deactivating
		{
			b_disable(true);

			/*pthread_mutex_lock(&main_status_mutex);
			runlevel = main_status.runlevel;
			SZARKStatus_init(&main_status);
			if(runlevel == OPERATIONAL) main_status.runlevel = STOPPED_SOFT;
			else main_status.runlevel = STOPPED_HARD;
			pthread_mutex_unlock(&main_status_mutex);*/
			// TODO to ^ najprawdopodobniej trzeba będzie włączyć

			lcommand_old.copy(lcommand, true);

			continue;
		}

		if(!lcommand.performEmergencyStop && lcommand_old.performEmergencyStop) // when activating
		{
			b_enable();

			/*pthread_mutex_lock(&main_status_mutex);
			main_status.runlevel = OPERATIONAL;
			pthread_mutex_unlock(&main_status_mutex);*/
		}

		//b_battery_measure(); 
		//usleep(6 * 1000); // 10 ms

		// LCD DISPLAY
		if(strcmp(lcommand.lcdText, lcommand_old.lcdText) != 0) b_setLCDText(lcommand.lcdText);

		if(lcommand.performEmergencyStop)
		{
			b_battery_measure();
			continue;
		}

		// MOTORS - analyzing speed and direction separately, because speed changes a lot more often than direction - less data to sent via uart

		// left motor
		if(lcommand.motors.left.direction != lcommand_old.motors.left.direction) b_setLeftMotorDirection(lcommand.motors.left.direction);
		if(lcommand.motors.left.speed != lcommand_old.motors.left.speed) b_setLeftMotorSpeed(lcommand.motors.left.speed);
	
		// right motor
		if(lcommand.motors.right.direction != lcommand_old.motors.right.direction) b_setRightMotorDirection(lcommand.motors.right.direction);
		if(lcommand.motors.right.speed != lcommand_old.motors.right.speed) b_setRightMotorSpeed(lcommand.motors.right.speed);


		// ARM
		// speed and direction for now
		if(!lcommand.armGoToPos)
		{
			if(lcommand.arm.gripper.direction != lcommand_old.arm.gripper.direction) b_setArmGripperDirection(lcommand.arm.gripper.direction);
			if(lcommand.arm.wrist.direction != lcommand_old.arm.wrist.direction) b_setArmWristDirection(lcommand.arm.wrist.direction);
			if(lcommand.arm.elbow.direction != lcommand_old.arm.elbow.direction) b_setArmElbowDirection(lcommand.arm.elbow.direction);
			if(lcommand.arm.shoulder.direction != lcommand_old.arm.shoulder.direction) b_setArmShoulderDirection(lcommand.arm.shoulder.direction);
		}
		else
		{
			if(lcommand.armPositions.gripper != lcommand_old.armPositions.gripper) b_setArmGripperPosition(lcommand.armPositions.gripper);
			if(lcommand.armPositions.wrist != lcommand_old.armPositions.wrist) b_setArmWristPosition(lcommand.armPositions.wrist);
			if(lcommand.armPositions.shoulder != lcommand_old.armPositions.shoulder) b_setArmShoulderPosition(lcommand.armPositions.shoulder);
			if(lcommand.armPositions.elbow != lcommand_old.armPositions.elbow) b_setArmElbowPosition(lcommand.armPositions.elbow);
		}

		if(lcommand.arm.gripper.speed != lcommand_old.arm.gripper.speed) b_setArmGripperSpeed(lcommand.arm.gripper.speed);
		if(lcommand.arm.wrist.speed != lcommand_old.arm.wrist.speed) b_setArmWristSpeed(lcommand.arm.wrist.speed);
		if(lcommand.arm.elbow.speed != lcommand_old.arm.elbow.speed) b_setArmElbowSpeed(lcommand.arm.elbow.speed);
		if(lcommand.arm.shoulder.speed != lcommand_old.arm.shoulder.speed) b_setArmShoulderSpeed(lcommand.arm.shoulder.speed);

		// LIGHTS
		if(lcommand.lights.low != lcommand_old.lights.low) b_setLightLow(lcommand.lights.low);
		if(lcommand.lights.high != lcommand_old.lights.high) b_setLightHigh(lcommand.lights.high);
		if(lcommand.lights.gripper != lcommand_old.lights.gripper) b_setLightGripper(lcommand.lights.gripper);
		if(lcommand.lights.camera != lcommand_old.lights.camera) b_setLightCamera(lcommand.lights.camera);


		// copy the current one to the old
		//memcpy(&lcommand_old, &lcommand, sizeof(SZARKCommand));
		lcommand_old.copy(lcommand);

		//b_getArmAllPositions();
		//b_getArmAllDirections();
		//usleep(6 * 1000); // 10 ms
		//b_getArmAllPositions();
		b_getAllData();
		//usleep(10 * 1000); // 10 ms
	}

	return NULL;
}

void b_shutdown()
{
	terminateFlag = true;

	imsgf("bridge: shutting down.\n");

	if(ser.IsOpen())
	{
		b_disable(true);	
#if DEBUG
		imsgf("bridge: Closing UART port.");
#endif
		ser.Close();
	}
#if DEBUG
	else errf("bridge: UART isn't open.");
#endif
}

//  low-level commands for bridge
void b_setLightLow(bool enable)
{
	if(enable) i2c_expander_byte |= (1 << RS_LIGHT_LOW);
	else i2c_expander_byte &= ~(1 << RS_LIGHT_LOW);	
	send_expander(i2c_expander_byte);
}

void b_setLightHigh(bool enable)
{
	if(enable) i2c_expander_byte |= (1 << RS_LIGHT_HIGH);
	else i2c_expander_byte &= ~(1 << RS_LIGHT_HIGH);
	send_expander(i2c_expander_byte);
}

void b_setLightGripper(bool enable)
{
	if(enable) i2c_expander_byte |= (1 << RS_LIGHT_GRIPPER);
	else i2c_expander_byte &= ~(1 << RS_LIGHT_GRIPPER);	
	send_expander(i2c_expander_byte);
}

void b_setLightCamera(bool enable)
{
	if(enable) i2c_expander_byte |= (1 << RS_LIGHT_CAMERA);
	else i2c_expander_byte &= ~(1 << RS_LIGHT_CAMERA);	
	send_expander(i2c_expander_byte);
}

void b_battery_measure()
{
	ser << RS_BATTERY << "\n";
}

void b_enable()
{
	b_disable();

	imsgf("bridge: Activating the bridge.");

	ser << RS_INIT_CHAR << "\n";
	
	i2c_expander_byte = 0;
	send_expander(i2c_expander_byte);

	usleep(100 * 1000);

	b_setLCDText(MESSAGE_READY_TO_COMMAND);
}

void b_disable(bool msg)
{

	ser << RS_EMERGENCY_STOP_STRING << "\n";
	ser << 'S' << '\0';

	if(msg) imsgf("bridge: Deactivating the bridge.");

	usleep(100 * 1000);
}

// due to my convenience, the characters are hardcoded
void b_setLeftMotorDirection(DIRECTION d)
{
#if DEBUG
	msgf("bridge: Left motor set to direction %s.", direction_to_text(d));
#endif 

	ser << "Mdl";
	putc_direction(d);
	ser << "\n";
}

void b_setRightMotorDirection(DIRECTION d)
{
#if DEBUG
	msgf("bridge: Right motor set to direction %s.", direction_to_text(d));
#endif

	ser << "Mdr";
	putc_direction(d);
	ser << "\n";

}

void b_setLeftMotorSpeed(SPEED speed)
{
#if DEBUG
	msgf("bridge: Left motor speed: %d.", speed);
#endif

	ser << "Msl";
	ser << speed;
	ser << "\n";

}

void b_setRightMotorSpeed(SPEED speed)
{
#if DEBUG
	msgf("bridge: Right motor speed: %d.", speed);
#endif

	ser << "Msr";
	ser << speed;
	ser << "\n";

}

void b_setLeftMotor(MotorParams m)
{
	b_setLeftMotorDirection(m.direction);
	b_setLeftMotorSpeed(m.speed);
}

void b_setRightMotor(MotorParams m)
{
	b_setRightMotorDirection(m.direction);
	b_setRightMotorSpeed(m.speed);
}

void b_setLCDText(const char *text)
{
	int i;
#if DEBUG
	msgf("bridge: LCD text set to: \"%s\".", text);
#endif
	ser << RS_LCD_WRITE;
	for(i = 0; i < LCD_CHARACTERS; i++)
	{
		if(text[i] == '\0') break;
		else if(text[i] == RS_LCD_NEWLINE) ser << '\n';
		else ser << text[i];
	}
	ser << RS_LCD_EOF << '\n';
	//ser << "              \n'"; // dummy string

	usleep(50 * 1000);
}

static void send_expander(char data)
{
	ser << RS_EXPANDER << RS_EXPANDER_SET << data << "\n";
}

static void putc_direction(DIRECTION d)
{
	switch(d)
	{
		case D_FORWARD:
			ser << RS_M_FORWARD;
			break;
		case D_BACKWARD:
			ser << RS_M_BACKWARD;
			break;
		case D_STOP:
			ser << RS_M_STOP;
			break;
#if DEBUG
		default:
			errf("bridge: putc_direction: Warning! Character \'%d\' is the wrong direction!", d);
			ser << RS_M_STOP;
			break;
#endif
	};
}


// ARM

void b_setArmShoulderDirection(DIRECTION d)
{
#if DEBUG
	msgf("bridge: Arm shoulder set to direction %s.", direction_to_text(d));
#endif

	ser << "Ads";
	putc_direction(d);
	ser << "\n";
}

void b_setArmShoulderSpeed(SPEED speed)
{
#if DEBUG
	msgf("bridge: Arm shoulder speed: %d.", speed);
#endif

	ser << "Ass";
	ser << speed;
	ser << "\n";
}

void b_setArmShoulder(MotorParams m)
{
	b_setArmShoulderDirection(m.direction);
	b_setArmShoulderSpeed(m.speed);
}
//
void b_setArmElbowDirection(DIRECTION d)
{
#if DEBUG
	msgf("bridge: Arm elbow set to direction %s.", direction_to_text(d));
#endif

	ser << "Ade";
	putc_direction(d);
	ser << "\n";
}

void b_setArmElbowSpeed(SPEED speed)
{
#if DEBUG
	msgf("bridge: Arm elbow speed: %d.", speed);
#endif

	ser << "Ase";
	ser << speed;
	ser << "\n";
}

void b_setArmElbow(MotorParams m)
{
	b_setArmElbowDirection(m.direction);
	b_setArmElbowSpeed(m.speed);
}
//
void b_setArmWristDirection(DIRECTION d)
{
#if DEBUG
	msgf("bridge: Arm wrist set to direction %s.", direction_to_text(d));
#endif

	ser << "Adw";
	putc_direction(d);
	ser << "\n";
}

void b_setArmWristSpeed(SPEED speed)
{
#if DEBUG
	msgf("bridge: Arm wrist speed: %d.", speed);
#endif

	ser << "Asw";
	ser << speed;
	ser << "\n";
}

void b_setArmWrist(MotorParams m)
{
	b_setArmWristDirection(m.direction);
	b_setArmWristSpeed(m.speed);
}
//
void b_setArmGripperDirection(DIRECTION d)
{
#if DEBUG
	msgf("bridge: Arm gripper set to direction %s.", direction_to_text(d));
#endif

	ser << "Adg";
	putc_direction(d);
	ser << "\n";
}

void b_setArmGripperSpeed(SPEED speed)
{
#if DEBUG
	msgf("bridge: Arm gripper speed: %d.", speed);
#endif

	ser << "Asg";
	ser << speed;
	ser << "\n";
}

void b_setArmGripper(MotorParams m)
{
	b_setArmGripperDirection(m.direction);
	b_setArmGripperSpeed(m.speed);
}
//
void b_setArmShoulderPosition(Position pos)
{
#if DEBUG
	msgf("bridge: Arm shoulder set to position: %d.", pos);
#endif

	ser << "Aps";
	ser << pos;
	ser << "\n";
}

void b_setArmGripperPosition(Position pos)
{
#if DEBUG
	msgf("bridge: Arm gripper set to position: %d.", pos);
#endif

	ser << "Apg";
	ser << pos;
	ser << "\n";
}

void b_setArmElbowPosition(Position pos)
{
#if DEBUG
	msgf("bridge: Arm elbow set to position: %d.", pos);
#endif
	ser << "Ape";
	ser << pos;
	ser << "\n";
}

void b_setArmWristPosition(Position pos)
{
#if DEBUG
	msgf("bridge: Arm wrist set to position: %d.", pos);
#endif

	ser << "Apw";
	ser << pos;
	ser << "\n";
}

void b_getArmAllPositions()
{
	ser << "AP" << '\n';
}

void b_getArmAllDirections()
{
	ser << "AD" << '\n';
}

void b_getAllData()
{
	ser << "QQ" << '\n';
}
