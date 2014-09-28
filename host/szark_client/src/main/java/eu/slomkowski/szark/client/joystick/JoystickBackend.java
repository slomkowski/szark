package eu.slomkowski.szark.client.joystick;

import net.java.games.input.Component;
import net.java.games.input.Component.Identifier;
import net.java.games.input.Controller;
import net.java.games.input.ControllerEnvironment;

public class JoystickBackend {

	private Controller controller;

	private Component forwardBackComp;
	private Component leftRightComp;
	private Component armShoulderComp;
	private Component armElbowComp;
	private Component POVComp;

	public JoystickBackend() throws InvalidJoystickException {
		final ControllerEnvironment ce = ControllerEnvironment.getDefaultEnvironment();
		final Controller[] cs = ce.getControllers();

		boolean valid = false;

		for (final Controller element : cs) {
			if ((element.getType() == Controller.Type.GAMEPAD) || (element.getType() == Controller.Type.STICK)) {
				if (element.getName().equals(JoystickConfiguration.COTROLLER_NAME)) {
					valid = true;
					controller = element;

					break;
				}
			}
		}

		if (!valid) {
			throw new InvalidJoystickException("Hardcoded joystick not found.");
		}

		final Component[] compList = controller.getComponents();

		// POV hat
		valid = false;
		for (final Component element : compList) {
			if (element.getIdentifier() == Identifier.Axis.POV) {
				valid = true;
				POVComp = element;
				break;
			}
		}
		if (!valid) {
			throw new InvalidJoystickException("Hardcoded joystick has no POV hat.");
		}

		// forward - backward axis
		valid = false;
		for (final Component element : compList) {
			if (element.getName().equals(JoystickConfiguration.MOV_FORW_BACK.getName())) {
				valid = true;
				forwardBackComp = element;
				break;
			}
		}
		if (!valid) {
			throw new InvalidJoystickException("Forward/backward axis: the joystick has no "
					+ JoystickConfiguration.MOV_FORW_BACK.getName() + " axis.");
		}

		// left-right axis
		valid = false;
		for (final Component element : compList) {
			if (element.getName().equals(JoystickConfiguration.MOV_LEFT_RIGHT.getName())) {
				valid = true;
				leftRightComp = element;
				break;
			}
		}
		if (!valid) {
			throw new InvalidJoystickException("Left/right axis: the joystick has no "
					+ JoystickConfiguration.MOV_FORW_BACK.getName() + " axis.");
		}

		// joints shoulder
		valid = false;
		for (final Component element : compList) {
			if (element.getName().equals(JoystickConfiguration.ARM_SHOULDER.getName())) {
				valid = true;
				armShoulderComp = element;
				break;
			}
		}
		if (!valid) {
			throw new InvalidJoystickException("Arm shoulder axis: the joystick has no "
					+ JoystickConfiguration.MOV_FORW_BACK.getName() + " axis.");
		}

		// joints elbow axis
		valid = false;
		for (final Component element : compList) {
			if (element.getName().equals(JoystickConfiguration.ARM_ELBOW.getName())) {
				valid = true;
				armElbowComp = element;
				break;
			}
		}
		if (!valid) {
			throw new InvalidJoystickException("Arm elbow axis: the joystick has no "
					+ JoystickConfiguration.MOV_FORW_BACK.getName() + " axis.");
		}

	}

	public void poll() {
		controller.poll();
	}

	public float getGripperVal() {
		final float data = POVComp.getPollData();

		if ((data == 0.25f) || (data == 0.125f) || (data == 0.375f)) {
			return JoystickConfiguration.GRIPPER_MFACTOR * 1.0f;
		}
		if ((data == 0.75f) || (data == 0.625f) || (data == 0.875f)) {
			return JoystickConfiguration.GRIPPER_MFACTOR * -1.0f;
		}

		return 0.0f;
	}

	public float getForwardBackwardVal() {
		return -1.0f * forwardBackComp.getPollData() * JoystickConfiguration.MOV_FORW_BACK.getMFactor();
	}

	public float getLeftRightVal() {
		return -1.0f * leftRightComp.getPollData() * JoystickConfiguration.MOV_LEFT_RIGHT.getMFactor();
	}

	public float getShoulderVal() {
		return armShoulderComp.getPollData() * JoystickConfiguration.ARM_SHOULDER.getMFactor();
	}

	public float getElbowVal() {
		return armElbowComp.getPollData() * JoystickConfiguration.ARM_ELBOW.getMFactor();
	}

}
