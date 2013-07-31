#ifndef _CONFIG_H_
#define _CONFIG_H_

#define LOGS_DISPLAY_TIME true
#define MESG_COLOR "\033[0m" // standard color
#define IMPORTANT_MESG_COLOR "\033[1;33m" // yellow
#define ERR_COLOR "\033[1;31m" // red

/*
 *  Network server configuration
 *
 */ 

// number of network port used by the server
#define NETWORKING_PORT 6666

// maximal length of string send or received by the server
#define MAX_DATA_LENGTH 500

/*
 *  Bridge configuration
 *
 */ 
#define BRIDGE_TRANSFERER_INTERVAL 16 // in millicesonds

// UART
#define UART_DEFAULT_DEVICE "/dev/ttyS0"
#define UART_BAUDRATE 38400

#define UART_ENABLE 1

// LCD display
#define LCD_CHARACTERS 32 

/*
 *  Watchdog configuration
 *
 */ 

#define WATCHDOG_ENABLE FALSE 
// when there wasn't any network connection to the server, it will shut down the device to prevent it from uncontrolled behaviour (in ms)
#define WATCHDOG_INTERVAL 400

// TRUE when testing, FALSE for real reboot or shutdown
#define DUMMY_SHUTDOWN_REBOOT FALSE
/*
 * External commands called by the server - reboot and shutdown
 */
#define MACHINE_REBOOT_COMMAND "sudo reboot"
#define MACHINE_SHUTDOWN_COMMAND "sudo shutdown -h now"

#endif
