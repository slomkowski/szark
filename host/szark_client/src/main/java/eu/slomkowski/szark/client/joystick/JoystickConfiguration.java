package eu.slomkowski.szark.client.joystick;

interface JoystickConfiguration {
	String CONTROLLER_NAME = "DragonRise Inc.   Generic   USB  Joystick  ";

	Axis MOV_FORWARD_BACKWARD = new Axis("y", 1.0f);
	Axis MOV_LEFT_RIGHT = new Axis("x", 1.0f);
	Axis ARM_SHOULDER = new Axis("rz", 1.0f);
	Axis ARM_ELBOW = new Axis("rx", 1.0f);

	// WARNING! - wrist and gripper are hardcoded to use POV hat

	float GRIPPER_MOVE_FACTOR = 1.0f;
}
