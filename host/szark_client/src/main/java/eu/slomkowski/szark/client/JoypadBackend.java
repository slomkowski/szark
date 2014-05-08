package eu.slomkowski.szark.client;

import net.java.games.input.Component;
import net.java.games.input.Component.Identifier;
import net.java.games.input.Controller;
import net.java.games.input.ControllerEnvironment;

/**
 * This interface stores the configuration of joypad.
 */
interface JoypadConfiguration {

	String COTROLLER_NAME = "DragonRise Inc.   Generic   USB  Joystick  ";

	Axis MOV_FORW_BACK = new Axis("y", 1.0f);
	Axis MOV_LEFT_RIGHT = new Axis("x", 1.0f);
	Axis ARM_SHOULDER = new Axis("rz", 1.0f);
	Axis ARM_ELBOW = new Axis("rx", 1.0f);

	// WARNING! - wrist and gripper are hardcoded to use POV hat

	float GRIPPER_MFACTOR = 1.0f;
	float WRIST_MFACTOR = 1.0f;

	/**
	 * Class to store an axis description.
	 */
	class Axis {

		/**
		 * 
		 * @param name
		 *             String name of specific axis, taken from
		 *             Component.getName().
		 * @param mFactor
		 *             Scaling factor of the value read from the axis.
		 *             Negative values invert the axis.
		 */
		public Axis(String name, float mFactor) {
			this.name = name;
			this.mFactor = mFactor;
		}

		private final String name;
		private final float mFactor;

		public float getMFactor() {
			if (mFactor < -1.0f) {
				return -1.0f;
			} else if (mFactor > 1.0f) {
				return 1.0f;
			} else {
				return mFactor;
			}
		}

		public String getName() {
			return name;
		}
	}
}

public class JoypadBackend {

	public class InvalidJoypadException extends Exception {

		public InvalidJoypadException(String string) {
			super(string);
		}

		private static final long serialVersionUID = 1L;
	}

	public void poll() {
		controller.poll();
	}

	public float getGripperVal() {
		final float data = POVComp.getPollData();

		if ((data == 0.25f) || (data == 0.125f) || (data == 0.375f)) {
			return JoypadConfiguration.GRIPPER_MFACTOR * 1.0f;
		}
		if ((data == 0.75f) || (data == 0.625f) || (data == 0.875f)) {
			return JoypadConfiguration.GRIPPER_MFACTOR * -1.0f;
		}

		return 0.0f;
	}

	public float getWristVal() {
		final float data = POVComp.getPollData();

		if ((data == 0.625f) || (data == 0.5f) || (data == 0.375f)) {
			return JoypadConfiguration.WRIST_MFACTOR * 1.0f;
		}
		if ((data == 0.125f) || (data == 1.0f) || (data == 0.875f)) {
			return JoypadConfiguration.WRIST_MFACTOR * -1.0f;
		}

		return 0.0f;
	}

	public float getForwBackVal() {
		return -1.0f * forwBackComp.getPollData() * JoypadConfiguration.MOV_FORW_BACK.getMFactor();
	}

	public float getLeftRightVal() {
		return -1.0f * leftRightComp.getPollData() * JoypadConfiguration.MOV_LEFT_RIGHT.getMFactor();
	}

	public float getShoulderVal() {
		return armShoulderComp.getPollData() * JoypadConfiguration.ARM_SHOULDER.getMFactor();
	}

	public float getElbowVal() {
		return armElbowComp.getPollData() * JoypadConfiguration.ARM_ELBOW.getMFactor();
	}

	public JoypadBackend() throws InvalidJoypadException {
		final ControllerEnvironment ce = ControllerEnvironment.getDefaultEnvironment();
		final Controller[] cs = ce.getControllers();

		boolean valid = false;

		for (final Controller element : cs) {
			if ((element.getType() == Controller.Type.GAMEPAD) || (element.getType() == Controller.Type.STICK)) {
				if (element.getName().equals(JoypadConfiguration.COTROLLER_NAME)) {
					valid = true;
					controller = element;

					break;
				}
			}
		}

		if (valid == false) {
			throw new InvalidJoypadException("Hardcoded joypad not found.");
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
		if (valid == false) {
			throw new InvalidJoypadException("Hardcoded joypad has no POV hat.");
		}

		// forward - backward axis
		valid = false;
		for (final Component element : compList) {
			if (element.getName().equals(JoypadConfiguration.MOV_FORW_BACK.getName())) {
				valid = true;
				forwBackComp = element;
				break;
			}
		}
		if (valid == false) {
			throw new InvalidJoypadException("Forward/backward axis: the joystick has no "
					+ JoypadConfiguration.MOV_FORW_BACK.getName() + " axis.");
		}

		// left-right axis
		valid = false;
		for (final Component element : compList) {
			if (element.getName().equals(JoypadConfiguration.MOV_LEFT_RIGHT.getName())) {
				valid = true;
				leftRightComp = element;
				break;
			}
		}
		if (valid == false) {
			throw new InvalidJoypadException("Left/right axis: the joystick has no "
					+ JoypadConfiguration.MOV_FORW_BACK.getName() + " axis.");
		}

		// arm shoulder
		valid = false;
		for (final Component element : compList) {
			if (element.getName().equals(JoypadConfiguration.ARM_SHOULDER.getName())) {
				valid = true;
				armShoulderComp = element;
				break;
			}
		}
		if (valid == false) {
			throw new InvalidJoypadException("Arm shoudler axis: the joystick has no "
					+ JoypadConfiguration.MOV_FORW_BACK.getName() + " axis.");
		}

		// arm elbow axis
		valid = false;
		for (final Component element : compList) {
			if (element.getName().equals(JoypadConfiguration.ARM_ELBOW.getName())) {
				valid = true;
				armElbowComp = element;
				break;
			}
		}
		if (valid == false) {
			throw new InvalidJoypadException("Arm elbow axis: the joystick has no "
					+ JoypadConfiguration.MOV_FORW_BACK.getName() + " axis.");
		}

	}

	private Controller controller;

	private Component forwBackComp;
	private Component leftRightComp;
	private Component armShoulderComp;
	private Component armElbowComp;
	private Component POVComp;

}
