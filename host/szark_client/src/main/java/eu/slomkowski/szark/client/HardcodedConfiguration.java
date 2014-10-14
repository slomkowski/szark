package eu.slomkowski.szark.client;

public interface HardcodedConfiguration {

	String PROGRAM_VERSION = "0.17";

	String[] DEFAULT_HOST_NAMES = {"localhost", "szark.local"};

	int CAMERA_TIMEOUT = 500; // in milliseconds
	int CAMERA_PORT_GRIPPER = 10192;
	int CAMERA_PORT_HEAD = 10192;

	int CONTROL_SERVER_PORT = 10191;
	int CONTROL_SERVER_REFRESH_INTERVAL = 30; // in milliseconds
	int CONTROL_SERVER_TIMEOUT = 100;

	boolean JOYSTICK_ENABLE = true;

	String DEFAULT_LOGO = "/img/logo.jpg";

	int JOINT_SPEED_INIT_SHOULDER = 7;
	int JOINT_SPEED_INIT_ELBOW = 5;
	int JOINT_SPEED_INIT_GRIPPER = 7;

	int JOINT_SPEED_MAX = 15;
	int MOTOR_SPEED_MAX = 12;
}
