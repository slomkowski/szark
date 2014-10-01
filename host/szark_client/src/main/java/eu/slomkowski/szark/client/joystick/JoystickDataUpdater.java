package eu.slomkowski.szark.client.joystick;

import eu.slomkowski.szark.client.status.Direction;
import eu.slomkowski.szark.client.status.Status;

public class JoystickDataUpdater {

	private static final float ROTATE_SPEED_FACTOR = 0.5f;

	private final JoystickBackend joy;
	private final Status status;

	public JoystickDataUpdater(Status status, JoystickBackend joystickBackend) {
		this.joy = joystickBackend;
		this.status = status;
	}

	public void update() {
		// movement
		joy.poll();
		final float fowBack = joy.getForwardBackwardVal();
		final float leftRight = joy.getLeftRightVal();

		// joints
		final float armShoulder = joy.getShoulderVal();
		final float armElbow = joy.getElbowVal();
		final float armGripper = joy.getGripperVal();

		final byte motorSpeedLimit = status.motors.getSpeedLimit();

		// float v = Math.abs((float)( motorSpeedLimit * fowBack * (1.0f -
		// Math.abs(leftRight)) ));

		if (fowBack == 0) // rotate or stop
		{
			if (leftRight == 0) {
				status.motors.left.setDirection(Direction.STOP);
				status.motors.right.setDirection(Direction.STOP);
				status.motors.left.setSpeed((byte) 0);
				status.motors.right.setSpeed((byte) 0);
			} else if (leftRight < 0) {
				status.motors.left.setDirection(Direction.FORWARD);
				status.motors.right.setDirection(Direction.BACKWARD);
				status.motors.left.setSpeed((byte) (motorSpeedLimit * ROTATE_SPEED_FACTOR * Math.abs(leftRight)));
				status.motors.right.setSpeed((byte) (motorSpeedLimit * ROTATE_SPEED_FACTOR * Math.abs(leftRight)));
			} else if (leftRight > 0) {
				status.motors.left.setDirection(Direction.BACKWARD);
				status.motors.right.setDirection(Direction.FORWARD);
				status.motors.left.setSpeed((byte) (motorSpeedLimit * ROTATE_SPEED_FACTOR * Math.abs(leftRight)));
				status.motors.right.setSpeed((byte) (motorSpeedLimit * ROTATE_SPEED_FACTOR * Math.abs(leftRight)));
			}
		} else if (fowBack > 0) {
			status.motors.left.setDirection(Direction.FORWARD);
			status.motors.right.setDirection(Direction.FORWARD);

			if (leftRight < 0) {
				status.motors.left.setSpeed((byte) (motorSpeedLimit * Math.abs(fowBack)));
				status.motors.right
						.setSpeed((byte) (motorSpeedLimit * (1 - Math.abs(leftRight) * Math.abs(fowBack))));
			} else if (leftRight > 0) {
				status.motors.right.setSpeed((byte) (motorSpeedLimit * Math.abs(fowBack)));
				status.motors.left
						.setSpeed((byte) (motorSpeedLimit * (1 - Math.abs(leftRight) * Math.abs(fowBack))));
			} else {
				status.motors.left.setSpeed((byte) (motorSpeedLimit * Math.abs(fowBack)));
				status.motors.right.setSpeed((byte) (motorSpeedLimit * Math.abs(fowBack)));
			}
		}
		if (fowBack < 0) {
			status.motors.left.setDirection(Direction.BACKWARD);
			status.motors.right.setDirection(Direction.BACKWARD);

			if (leftRight < 0) {
				status.motors.left.setSpeed((byte) (motorSpeedLimit * Math.abs(fowBack)));
				status.motors.right
						.setSpeed((byte) (motorSpeedLimit * (1 - Math.abs(leftRight) * Math.abs(fowBack))));
			} else if (leftRight > 0) {
				status.motors.right.setSpeed((byte) (motorSpeedLimit * Math.abs(fowBack)));
				status.motors.left
						.setSpeed((byte) (motorSpeedLimit * (1 - Math.abs(leftRight) * Math.abs(fowBack))));
			} else {
				status.motors.left.setSpeed((byte) (motorSpeedLimit * Math.abs(fowBack)));
				status.motors.right.setSpeed((byte) (motorSpeedLimit * Math.abs(fowBack)));
			}
		}

		// joints
		if (armShoulder == 0) {
			status.joints.shoulder.setDirection(Direction.STOP);
			status.joints.shoulder.setSpeed(0);
		} else if (armShoulder > 0) {
			status.joints.shoulder.setDirection(Direction.FORWARD);
			status.joints.shoulder.setSpeed((byte) (status.joints.shoulder.getSpeedLimit() * Math.abs(armShoulder)));
		} else if (armShoulder < 0) {
			status.joints.shoulder.setDirection(Direction.BACKWARD);
			status.joints.shoulder.setSpeed((byte) (status.joints.shoulder.getSpeedLimit() * Math.abs(armShoulder)));
		}

		if (armElbow == 0) {
			status.joints.elbow.setDirection(Direction.STOP);
			status.joints.elbow.setSpeed(0);
		} else if (armElbow > 0) {
			status.joints.elbow.setDirection(Direction.FORWARD);
			status.joints.elbow.setSpeed((byte) (status.joints.elbow.getSpeedLimit() * Math.abs(armElbow)));
		} else if (armElbow < 0) {
			status.joints.elbow.setDirection(Direction.BACKWARD);
			status.joints.elbow.setSpeed((byte) (status.joints.elbow.getSpeedLimit() * Math.abs(armElbow)));
		}

		if (armGripper == 0) {
			status.joints.gripper.setDirection(Direction.STOP);
			status.joints.gripper.setSpeed(0);
		} else if (armGripper > 0) {
			status.joints.gripper.setDirection(Direction.FORWARD);
			status.joints.gripper.setSpeed((byte) (status.joints.gripper.getSpeedLimit() * Math.abs(armGripper)));
		} else if (armGripper < 0) {
			status.joints.gripper.setDirection(Direction.BACKWARD);
			status.joints.gripper.setSpeed((byte) (status.joints.gripper.getSpeedLimit() * Math.abs(armGripper)));
		}
	}
}
