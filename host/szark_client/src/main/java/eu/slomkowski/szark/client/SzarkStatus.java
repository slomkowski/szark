package eu.slomkowski.szark.client;

import com.google.gson.annotations.Expose;
import com.google.gson.annotations.SerializedName;

public class SzarkStatus {

	public class Arm extends Motorized {

		protected class ArmParams extends Params {

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

				if (other instanceof Params) {
					final ArmParams o = (ArmParams) other;

					if ((this.getDirection() == o.getDirection()) && (this.getSpeed() == o.getSpeed())
							&& (this.getPosition() == o.getPosition())) {
						return true;
					} else {
						return false;
					}
				} else {
					return false;
				}
			}

			public synchronized Direction getExactDirection() {
				return exactDirection;
			}

			public synchronized int getPosition() {
				return position;
			}

			public synchronized void setExactDirection(Direction exactDirection) {
				this.exactDirection = exactDirection;
			}

			public synchronized void setPosition(int position) {
				this.position = position;
			}

			@Override
			public String toString() {
				return "direction: " + getDirection() + ", speed: " + this.getSpeed() + ", pos: " + getPosition();
			}
		}

		private CalStatus calStatus = CalStatus.READY;

		@Expose
		public ArmParams elbow = new ArmParams();

		@Expose
		public ArmParams gripper = new ArmParams();

		@Expose
		public ArmParams shoulder = new ArmParams();

		@Expose
		public ArmParams wrist = new ArmParams();

		@Override
		public boolean equals(Object obj) {
			if (obj == null) {
				return false;
			}
			if (obj == this) {
				return true;
			}

			if (obj instanceof Motors) {
				final Arm o = (Arm) obj;
				if (gripper.equals(o.gripper) && wrist.equals(o.wrist) && shoulder.equals(o.shoulder)
						&& elbow.equals(o.elbow)) {
					return true;
				}
			}

			return false;
		}

		protected CalStatus getCalStatus() {
			return calStatus;
		}

		protected void setCalStatus(CalStatus calStatus) {
			this.calStatus = calStatus;
		}

		@Override
		public void setSpeedLimit(byte speed) {
			if (speed < 0) {
				speedLimit = 0;
			} else if (speed > 15) {
				speedLimit = 15;
			} else {
				speedLimit = speed;
			}

			if (wrist.getSpeedLimit() > speedLimit) {
				wrist.setSpeedLimit(speedLimit);
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
			wrist.stop();
			elbow.stop();
			shoulder.stop();
		}
	}

	public class Battery {

		@Expose(deserialize = true, serialize = false)
		@SerializedName("curr")
		private float current;

		@Expose(deserialize = true, serialize = false)
		@SerializedName("volt")
		private float voltage;

		public float getCurrent() {
			return current;
		}

		public float getVoltage() {
			return voltage;
		}

		public void setCurrent(float current) {
			this.current = (float) (Math.round(current * 1000.0) / 1000.0);
		}

		public void setVoltage(float voltage) {
			this.voltage = (float) (Math.round(voltage * 1000.0) / 1000.0);
		}
	}

	public enum CalStatus {
		READY, REQ_SENT, REQUESTED
	}

	// definition of direction
	public enum Direction {
		@SerializedName("backward")
		BACKWARD("backward"),

		@SerializedName("forward")
		FORWARD("forward"),

		@SerializedName("stop")
		STOP("stop");

		private String name;

		private Direction(String name) {
			this.name = name;
		}

		@Override
		public String toString() {
			return name;
		}
	}

	public class Lights {

		private boolean camera;

		@Expose
		@SerializedName("camera")
		private boolean gripper;

		@Expose
		@SerializedName("left")
		private boolean high;

		@Expose
		@SerializedName("right")
		private boolean low;

		public synchronized boolean isCamera() {
			return camera;
		}

		public synchronized boolean isGripper() {
			return gripper;
		}

		public synchronized boolean isHigh() {
			return high;
		}

		public synchronized boolean isLow() {
			return low;
		}

		public synchronized void setCamera(boolean camera) {
			this.camera = camera;
		}

		public synchronized void setGripper(boolean gripper) {
			this.gripper = gripper;
		}

		public synchronized void setHigh(boolean high) {
			this.high = high;
		}

		public synchronized void setLow(boolean low) {
			this.low = low;
		}
	}

	// classes definitions
	abstract protected class Motorized {

		protected class Params {

			@Expose
			@SerializedName("dir")
			private Direction direction;

			@Expose
			@SerializedName("speed")
			private byte speed;

			protected byte speedLimit = 0;

			Params() {
				stop();

				speed = 0; // initial speed
			}

			Params(Direction dir, byte sp) {
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

				if (other instanceof Params) {
					final Params o = (Params) other;

					if ((direction == o.direction) && (speed == o.speed)) {
						return true;
					} else {
						return false;
					}
				} else {
					return false;
				}
			}

			public synchronized Direction getDirection() {
				return direction;
			}

			public synchronized byte getSpeed() {
				return speed;
			}

			public byte getSpeedLimit() {
				return speedLimit;
			}

			public synchronized void setDirection(Direction dir) {
				direction = dir;
			}

			public synchronized void setSpeed(byte sp) {
				if (sp > speedLimit) {
					speed = speedLimit;
				} else {
					speed = sp;
				}
			}

			public void setSpeed(int speed) {
				setSpeed((byte) speed);
			}

			public void setSpeedLimit(byte speed) {
				if (speed < 0) {
					speedLimit = 0;
				} else if (speed > 15) {
					speedLimit = 15;
				} else {
					speedLimit = speed;
				}
			}

			public void setSpeedLimit(int speed) {
				setSpeedLimit((byte) speed);
			}

			public synchronized void stop() {
				speed = 0;
				direction = Direction.STOP;
			}

			@Override
			public String toString() {
				return "direction: " + direction + ", speed: " + speed;
			}
		}

		protected byte speedLimit = 15;

		public synchronized byte getSpeedLimit() {
			return speedLimit;
		}

		public abstract void setSpeedLimit(byte speed);

		public void setSpeedLimit(int speed) {
			setSpeedLimit((byte) speed);
		}

		public abstract void stop();
	}

	public class Motors extends Motorized {

		@Expose
		public Params left = new Params();

		@Expose
		public Params right = new Params();

		@Override
		public boolean equals(Object obj) {
			if (obj == null) {
				return false;
			}
			if (obj == this) {
				return true;
			}

			if (obj instanceof Motors) {
				final Motors o = (Motors) obj;
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

	};

	public class Server {

		private boolean do_exit = false;
		private boolean rebootSystem = false;
		private boolean shutdownSystem = false;

		@Override
		public boolean equals(Object obj) {
			if (obj == null) {
				return false;
			}
			if (obj == this) {
				return true;
			}

			if (obj instanceof Server) {
				final Server o = (Server) obj;

				if ((rebootSystem == o.rebootSystem) && (shutdownSystem == o.shutdownSystem)
						&& (do_exit == o.do_exit)) {
					return true;
				}
			}
			return false;
		}

		public synchronized void exit() {
			do_exit = true;
		}

		public synchronized boolean mustExit() {
			return do_exit;
		}

		public synchronized boolean mustReboot() {
			return rebootSystem;
		}

		public synchronized boolean mustShutdown() {
			return shutdownSystem;
		}

		public synchronized void reboot() {
			rebootSystem = true;
		}

		public synchronized void shutdown() {
			shutdownSystem = true;
		}
	};

	@Expose
	public Arm arm;

	@Expose(deserialize = true, serialize = false)
	@SerializedName("batt")
	public Battery battery;

	@Expose(deserialize = false, serialize = true)
	@SerializedName("killswitch")
	private boolean emergencyStopped;

	@Expose(deserialize = false, serialize = true)
	@SerializedName("light")
	public Lights lights;

	@Expose
	@SerializedName("motor")
	public Motors motors;

	@Expose(serialize = true, deserialize = false)
	int serial = 0;

	// implementation

	public Server server;

	@Expose(deserialize = true, serialize = false)
	@SerializedName("wifi")
	private int wirelessPower;

	SzarkStatus() {
		clean();
	}

	// standard initial routines
	public synchronized void clean() {
		battery = new Battery();
		motors = new Motors();
		server = new Server();
		lights = new Lights();
		arm = new Arm();

		wirelessPower = 0;
		emergencyStopped = true;
		serial = 0;
	}

	public void incrementSerial() {
		serial++;
	}

	public int getWirelessPower() {
		return wirelessPower;
	}

	public synchronized boolean isEmergencyStopped() {
		return emergencyStopped;
	}

	public synchronized void setEmergencyStopped(boolean emergencyStopped) {
		this.emergencyStopped = emergencyStopped;
	}

	public void setWirelessPower(int wirelessPower) {
		this.wirelessPower = wirelessPower;
	}
}
