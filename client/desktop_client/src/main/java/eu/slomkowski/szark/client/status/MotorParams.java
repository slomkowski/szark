package eu.slomkowski.szark.client.status;

import eu.slomkowski.szark.client.HardcodedConfiguration;

public class MotorParams {

	private Direction direction;
	private byte speed;

	public static enum Direction {
		STOP, FORWARD, BACKWARD
	}

	public void setDirection(Direction dir) {
		direction = dir;
	}

	public Direction getDirection() {
		return direction;
	}

	public void setSpeed(byte sp) {
		if (sp > HardcodedConfiguration.MOTOR_SPEED_MAX) {
			speed = HardcodedConfiguration.MOTOR_SPEED_MAX;
		} else {
			speed = sp;
		}
	}

	public byte getSpeed() {
		return speed;
	}

	public void stop() {
		speed = 0;
		direction = Direction.STOP;
	}

	MotorParams() {
		stop();

		speed = 0; // initial speed
	}

	MotorParams(Direction dir, byte sp) {
		direction = dir;
		speed = sp;
	}

	@Override
	public boolean equals(Object other) {
		if (other == null) {
			return false;
		}
		if (this == other) {
			return true;
		}

		if (other instanceof MotorParams) {
			final MotorParams o = (MotorParams) other;

			return (direction == o.direction) && (speed == o.speed);
		} else {
			return false;
		}
	}

	@Override
	public String toString() {
		String dir;

		switch (direction) {
			case FORWARD:
				dir = "forward";
				break;
			case BACKWARD:
				dir = "backward";
				break;
			default:
				dir = "stopped";
		}

		return "direction: " + dir + ", speed: " + speed;
	}
}
