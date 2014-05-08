package eu.slomkowski.szark.client;

public class JoypadDataUpdater {

	JoypadDataUpdater(SzarkStatus status, JoypadBackend joypadBackend) {
		joy = joypadBackend;
		this.status = status;
	}

	private final JoypadBackend joy;
	private final SzarkStatus status;

	private static float ROTATE_SPEED_FACTOR = 0.5f;

	public void update() {
		// movement
		joy.poll();
		final float fowBack = joy.getForwBackVal();
		final float leftRight = joy.getLeftRightVal();

		// arm
		final float armShoulder = joy.getShoulderVal();
		final float armElbow = joy.getElbowVal();
		final float armWrist = joy.getWristVal();
		final float armGripper = joy.getGripperVal();

		final byte motorSpeedLimit = status.motors.getSpeedLimit();

		// float v = Math.abs((float)( motorSpeedLimit * fowBack * (1.0f -
		// Math.abs(leftRight)) ));

		if (fowBack == 0) // rotate or stop
		{
			if (leftRight == 0) {
				status.motors.left.setDirection(SzarkStatus.Direction.STOP);
				status.motors.right.setDirection(SzarkStatus.Direction.STOP);
				status.motors.left.setSpeed((byte) 0);
				status.motors.right.setSpeed((byte) 0);
			} else if (leftRight < 0) {
				status.motors.left.setDirection(SzarkStatus.Direction.FORWARD);
				status.motors.right.setDirection(SzarkStatus.Direction.BACKWARD);
				status.motors.left.setSpeed((byte) (motorSpeedLimit * ROTATE_SPEED_FACTOR * Math.abs(leftRight)));
				status.motors.right.setSpeed((byte) (motorSpeedLimit * ROTATE_SPEED_FACTOR * Math.abs(leftRight)));
			} else if (leftRight > 0) {
				status.motors.left.setDirection(SzarkStatus.Direction.BACKWARD);
				status.motors.right.setDirection(SzarkStatus.Direction.FORWARD);
				status.motors.left.setSpeed((byte) (motorSpeedLimit * ROTATE_SPEED_FACTOR * Math.abs(leftRight)));
				status.motors.right.setSpeed((byte) (motorSpeedLimit * ROTATE_SPEED_FACTOR * Math.abs(leftRight)));
			}
		} else if (fowBack > 0) {
			status.motors.left.setDirection(SzarkStatus.Direction.FORWARD);
			status.motors.right.setDirection(SzarkStatus.Direction.FORWARD);

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
			status.motors.left.setDirection(SzarkStatus.Direction.BACKWARD);
			status.motors.right.setDirection(SzarkStatus.Direction.BACKWARD);

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

		// arm
		if (armShoulder == 0) {
			status.arm.shoulder.setDirection(SzarkStatus.Direction.STOP);
			status.arm.shoulder.setSpeed(0);
		} else if (armShoulder > 0) {
			status.arm.shoulder.setDirection(SzarkStatus.Direction.FORWARD);
			status.arm.shoulder.setSpeed((byte) (status.arm.shoulder.getSpeedLimit() * Math.abs(armShoulder)));
		} else if (armShoulder < 0) {
			status.arm.shoulder.setDirection(SzarkStatus.Direction.BACKWARD);
			status.arm.shoulder.setSpeed((byte) (status.arm.shoulder.getSpeedLimit() * Math.abs(armShoulder)));
		}

		if (armElbow == 0) {
			status.arm.elbow.setDirection(SzarkStatus.Direction.STOP);
			status.arm.elbow.setSpeed(0);
		} else if (armElbow > 0) {
			status.arm.elbow.setDirection(SzarkStatus.Direction.FORWARD);
			status.arm.elbow.setSpeed((byte) (status.arm.elbow.getSpeedLimit() * Math.abs(armElbow)));
		} else if (armElbow < 0) {
			status.arm.elbow.setDirection(SzarkStatus.Direction.BACKWARD);
			status.arm.elbow.setSpeed((byte) (status.arm.elbow.getSpeedLimit() * Math.abs(armElbow)));
		}

		if (armGripper == 0) {
			status.arm.gripper.setDirection(SzarkStatus.Direction.STOP);
			status.arm.gripper.setSpeed(0);
		} else if (armGripper > 0) {
			status.arm.gripper.setDirection(SzarkStatus.Direction.FORWARD);
			status.arm.gripper.setSpeed((byte) (status.arm.gripper.getSpeedLimit() * Math.abs(armGripper)));
		} else if (armGripper < 0) {
			status.arm.gripper.setDirection(SzarkStatus.Direction.BACKWARD);
			status.arm.gripper.setSpeed((byte) (status.arm.gripper.getSpeedLimit() * Math.abs(armGripper)));
		}

		if (armWrist == 0) {
			status.arm.wrist.setDirection(SzarkStatus.Direction.STOP);
			status.arm.wrist.setSpeed(0);
		} else if (armWrist > 0) {
			status.arm.wrist.setDirection(SzarkStatus.Direction.FORWARD);
			status.arm.wrist.setSpeed((byte) (status.arm.wrist.getSpeedLimit() * Math.abs(armWrist)));
		} else if (armWrist < 0) {
			status.arm.wrist.setDirection(SzarkStatus.Direction.BACKWARD);
			status.arm.wrist.setSpeed((byte) (status.arm.wrist.getSpeedLimit() * Math.abs(armWrist)));
		}
	}

}
