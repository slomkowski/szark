package eu.slomkowski.szark.client;

/**
 * Stores the settings for the SZARK-client
 *
 * @author Michał Słomkowski
 */
public interface HardcodedConfiguration {

	String VERSION = "0.17";

	String[] DEFAULT_HOSTNAMES = {"localhost", "szark.local"};

	int GRIPPER_CAMERA_PORT = 8081;
	int HEAD_CAMERA_PORT = 8080;
	int CAMERA_REFRESH_INTERVAL = 50; // in milliseconds

	int SZARK_SERVER_PORT = 10191;
	int SZARK_REFRESH_INTERVAL = 50; // in milliseconds

	boolean ENABLE_JOYSTICK = true;

	String DEFAULT_LOGO = "/img/logo.jpg";

	// joints initial speeds
	int ELBOW_SPEED = 5;
	int SHOULDER_SPEED = 5;
	int WRIST_SPEED = 3;
	int GRIPPER_SPPED = 7;
}
