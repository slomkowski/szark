#ifndef _NETWORKING_H_
#define _NETWORKING_H_

void *networking_function(void *arg);
//void networking_sockets_close();
int networking_setup(int port);
void networking_shutdown();

#define C_VALUE_OFFSET_CHARACTER 'a'

#define C_MOTORS 'M'
#define C_MOTORS_GET_SPEED 'g'
#define C_MOTORS_GET_DIRECTION 'b'
#define C_MOTORS_SET_SPEED 's'
#define C_MOTORS_SET_DIRECTION 'd'
#define C_MOTORS_LEFT 'l'
#define C_MOTORS_RIGHT 'r'

#define C_BATTERY 'B'
#define C_BATTERY_GET 'g'

#define C_SERVER 'S'
#define C_SERVER_REBOOT 'r'
#define C_SERVER_SHUTDOWN 's'
#define C_SERVER_EXIT 'x'
#define C_SERVER_ENABLE 'e'
#define C_SERVER_DISABLE 'd'

#define C_LIGHTS 'L'
#define C_LIGHTS_ENABLE 'e'
#define C_LIGHTS_DISABLE 'd'
#define C_LIGHTS_GRIPPER 'g'
#define C_LIGHTS_HIGH 'h'
#define C_LIGHTS_LOW 'l'
#define C_LIGHTS_CAMERA 'c'

// TODO
#define C_ARM 'A'
#define C_ARM_GET_SPEED 'g'
#define C_ARM_GET_DIRECTION 'b'
#define C_ARM_SET_SPEED 's'
#define C_ARM_SET_DIRECTION 'd'
#define C_ARM_GET_POSITION 'q'
#define C_ARM_SET_POSITION 'p'
#define C_ARM_GRIPPER 'g'
#define C_ARM_ELBOW 'e'
#define C_ARM_WRIST 'w'
#define C_ARM_SHOULDER 's'


#define C_LCD 'D'
#define C_LCD_GET 'g'
#define C_LCD_SET 's'
#define C_LCD_TERM_CHAR '~'

#define C_BUTTON1 'X'  
#define C_BUTTON2 'Y'  
#define C_BUTTON3 'Z'

#define C_EMERGENCY_STOP 'E'
#define C_EMERGENCY_STOP_DISABLE 'd'
#define C_EMERGENCY_STOP_ENABLE 'e'
#define C_EMERGENCY_STOP_HARDWARE 'h'




#endif
