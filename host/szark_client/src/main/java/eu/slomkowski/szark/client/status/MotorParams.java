package eu.slomkowski.szark.client.status;

public class MotorParams {

	private Direction direction;
	private byte speed;

	public static enum Direction {
		STOP, FORWARD, BACKWARD
	};

	public void setDirection(Direction dir) {
		direction = dir;
	}

	public Direction getDirection() {
		return direction;
	}

	public void setSpeed(byte sp) {
		if (sp > 15) {
			speed = 15;
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

			if ((direction == o.direction) && (speed == o.speed)) {
				return true;
			} else {
				return false;
			}
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
		;

		return "direction: " + dir + ", speed: " + speed;
	}
}
