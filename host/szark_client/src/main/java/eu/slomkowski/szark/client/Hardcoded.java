package eu.slomkowski.szark.client;

/**
 * Stores the settings for the SZARK-client
 * 
 * @author Michał Słomkowski
 * 
 */
public interface Hardcoded {

	String VERSION = "0.16";

	String[] DEFAULT_HOSTNAMES = { "localhost", "10.0.0.1", "10.0.1.1", "192.168.1.31" };

	int GRIPPER_CAMERA_PORT = 8081;
	int HEAD_CAMERA_PORT = 8080;
	int CAMERA_REFRESH_INTERVAL = 50; // in milliseconds

	int SZARK_SERVER_PORT = 6666;
	int SZARK_REFRESH_INTERVAL = 40; // in milliseconds

	boolean ENABLE_JOYSTICK = true;

	String DEFAULT_LOGO = "/img/logo.jpg";

	// arm initial speeds
	int ELBOW_SPEED = 5;
	int SHOULDER_SPEED = 5;
	int WRIST_SPEED = 3;
	int GRIPPER_SPPED = 7;

	// more low-level settings
	int MAX_DATA_LENGTH = 255;
	int SOCKET_TIMEOUT = 1000; // in milliseconds
}
