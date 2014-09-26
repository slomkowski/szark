package eu.slomkowski.szark.client.joystick;

public interface JoystickConfiguration {
	String COTROLLER_NAME = "DragonRise Inc.   Generic   USB  Joystick  ";

	Axis MOV_FORW_BACK = new Axis("y", 1.0f);
	Axis MOV_LEFT_RIGHT = new Axis("x", 1.0f);
	Axis ARM_SHOULDER = new Axis("rz", 1.0f);
	Axis ARM_ELBOW = new Axis("rx", 1.0f);

	// WARNING! - wrist and gripper are hardcoded to use POV hat

	float GRIPPER_MFACTOR = 1.0f;
	float WRIST_MFACTOR = 1.0f;
}
