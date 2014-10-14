package eu.slomkowski.szark.client.status;

import com.google.gson.annotations.Expose;
import com.google.gson.annotations.SerializedName;
import eu.slomkowski.szark.client.HardcodedConfiguration;

public class JointSet extends AbstractMotor {

	@Expose
	public Joint elbow = new Joint();
	@Expose
	public Joint gripper = new Joint();
	@Expose
	public Joint shoulder = new Joint();

	@Expose(deserialize = false, serialize = true)
	@SerializedName("b_cal")
	private boolean beginCalibration = false;

	@Expose(deserialize = true, serialize = false)
	@SerializedName("cal_st")
	private CalibrationStatus calStatus = CalibrationStatus.NONE;

	@Expose(deserialize = true, serialize = false)
	@SerializedName("mode")
	private ArmDriverMode armDriverMode = ArmDriverMode.DIRECTIONAL;

	@Override
	public boolean equals(Object obj) {
		if (obj == null) {
			return false;
		}
		if (obj == this) {
			return true;
		}

		if (obj instanceof JointSet) {
			final JointSet o = (JointSet) obj;
			if (gripper.equals(o.gripper)
					&& shoulder.equals(o.shoulder)
					&& elbow.equals(o.elbow)) {
				return true;
			}
		}

		return false;
	}

	@Override
	public void setSpeedLimit(byte speed) {
		if (speed < 0) {
			speedLimit = 0;
		} else if (speed > HardcodedConfiguration.JOINT_SPEED_MAX) {
			speedLimit = HardcodedConfiguration.JOINT_SPEED_MAX;
		} else {
			speedLimit = speed;
		}

		if (gripper.getSpeedLimit() > speedLimit) {
			gripper.setSpeedLimit(speedLimit);
		}
		if (shoulder.getSpeedLimit() > speedLimit) {
			shoulder.setSpeedLimit(speedLimit);
		}
		if (elbow.getSpeedLimit() > speedLimit) {
			elbow.setSpeedLimit(speedLimit);
		}
	}

	@Override
	public void stop() {
		gripper.stop();
		elbow.stop();
		shoulder.stop();
	}

	public CalibrationStatus getCalStatus() {
		return calStatus;
	}

	public void setCalStatus(CalibrationStatus calStatus) {
		this.calStatus = calStatus;
	}


	public boolean isBeginCalibration() {
		return beginCalibration;
	}

	public void setBeginCalibration(boolean beginCalibration) {
		this.beginCalibration = beginCalibration;
	}

	public ArmDriverMode getArmDriverMode() {
		return armDriverMode;
	}

	public void setArmDriverMode(ArmDriverMode armDriverMode) {
		this.armDriverMode = armDriverMode;
	}

	public class Joint extends AbstractParams {

		private Direction exactDirection;

		@Expose(deserialize = true, serialize = false)
		@SerializedName("pos")
		private int position;

		@Override
		public boolean equals(Object other) {
			if (other == null) {
				return false;
			}
			if (this == other) {
				return true;
			}

			if (other instanceof AbstractParams) {
				final Joint o = (Joint) other;

				return (this.getDirection() == o.getDirection()) && (this.getSpeed() == o.getSpeed())
						&& (this.getPosition() == o.getPosition());
			} else {
				return false;
			}
		}

		public synchronized Direction getExactDirection() {
			return exactDirection;
		}

		public synchronized void setExactDirection(Direction exactDirection) {
			this.exactDirection = exactDirection;
		}

		public synchronized int getPosition() {
			return position;
		}

		public synchronized void setPosition(int position) {
			this.position = position;
		}

		@Override
		public String toString() {
			return "direction: " + getDirection() + ", speed: " + this.getSpeed() + ", pos: " + getPosition();
		}
	}
}
