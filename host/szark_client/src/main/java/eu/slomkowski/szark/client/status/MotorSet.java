package eu.slomkowski.szark.client.status;

import com.google.gson.annotations.Expose;

public class MotorSet extends AbstractMotor {

	@Expose
	public Motor left = new Motor();

	@Expose
	public Motor right = new Motor();

	@Override
	public boolean equals(Object obj) {
		if (obj == null) {
			return false;
		}
		if (obj == this) {
			return true;
		}

		if (obj instanceof MotorSet) {
			final MotorSet o = (MotorSet) obj;
			if (left.equals(o.left) && right.equals(o.right)) {
				return true;
			}
		}

		return false;
	}

	@Override
	public synchronized void setSpeedLimit(byte speed) {
		if (speed < 0) {
			speedLimit = 0;
		} else if (speed > 15) {
			speedLimit = 15;
		} else {
			speedLimit = speed;
		}

		left.setSpeedLimit(speedLimit);
		right.setSpeedLimit(speedLimit);
	}

	@Override
	public void stop() {
		left.stop();
		right.stop();
	}

	public class Motor extends AbstractParams {
	}
}
