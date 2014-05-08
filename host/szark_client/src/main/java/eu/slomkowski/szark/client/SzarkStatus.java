package eu.slomkowski.szark.client;
public class SzarkStatus 
{
	// wifi
	public int getWirelessPower()
	{
		return wirelessPower;
	}

	public void setWirelessPower(int wirelessPower)
	{
		this.wirelessPower = wirelessPower;
	}

	// emergency stop
	public synchronized boolean isEmergencyStopped()
	{
		return emergencyStopped;
	}

	public synchronized void setEmergencyStopped(boolean emergencyStopped)
	{
		this.emergencyStopped = emergencyStopped;
	}
	
	// standard initial routines 
	public synchronized void clean()
	{
		battery = new Battery();
		motors = new Motors();
		server = new Server();
		lights = new Lights();
		arm = new Arm();
		
		wirelessPower = 0;
		emergencyStopped = true;
	}
	
	SzarkStatus()
	{
		clean();
	}
	
	// definition of direction
	public enum Direction { STOP, FORWARD, BACKWARD };
	public enum CalStatus { READY, REQUESTED, REQ_SENT };

	public Battery battery;
	public Motors motors;
	public Server server;
	public Lights lights;
	public Arm arm;
	
	//implementation
	
	private int wirelessPower;
	private boolean emergencyStopped;
	
	// classes definitions
	abstract protected class Motorized
	{
		protected byte speedLimit = 15;
		
		public abstract void stop();
		
		public void setSpeedLimit(int speed)
		{
			setSpeedLimit((byte)speed);
		}
		
		public abstract void setSpeedLimit(byte speed);
		
		public synchronized byte getSpeedLimit()
		{
			return speedLimit;
		}
		
		protected class Params 
		{
			private Direction direction;
			private byte speed;
			
			protected byte speedLimit = 0;
			
			public void setSpeedLimit(byte speed)
			{
				if(speed < 0) speedLimit = 0;
				else if(speed > 15) speedLimit = 15;
				else speedLimit = speed;
			}
			
			public void setSpeedLimit(int speed)
			{
				setSpeedLimit((byte)speed);
			}

			public byte getSpeedLimit()
			{
				return speedLimit;
			}
			
			public synchronized void setDirection(Direction dir)
			{
				direction = dir;
			}
			
			public synchronized String getDirectionText()
			{
				if(direction == Direction.FORWARD) return "forward";
				else if(direction == Direction.BACKWARD) return "backward";
				else return "stopped";
			}
			
			public synchronized Direction getDirection()
			{
				return direction;
			}
			
			public void setSpeed(int speed)
			{
				setSpeed((byte) speed);
			}
			
			public synchronized void setSpeed(byte sp)
			{
				if(sp > speedLimit) speed = speedLimit;
				else speed = sp;
			}
			
			public synchronized byte getSpeed()
			{
				return speed;
			}
			
			public synchronized void stop()
			{
				speed = 0;
				direction = Direction.STOP;
			}
			
			Params()
			{
				stop();
				
				speed = 0; // initial speed 
			}
			
			Params(Direction dir, byte sp)
			{
				direction = dir;
				speed = sp;
			}
			
			public boolean equals(Object other)
			{
				if(other == null) return false;
				if(this == other) return true;
				
				if(other instanceof Params)
				{
					Params o = (Params)other;
					
					if((this.direction == o.direction) && (this.speed == o.speed)) return true;
					else return false;
				}
				else return false;
			}
			
			public String toString()
			{
				String dir;
				
				switch(this.direction)
				{
					case FORWARD:
						dir = "forward";
						break;
					case BACKWARD:
						dir = "backward";
						break;
					default:
						dir = "stopped";
				};
				
				return "direction: " + dir + ", speed: " + this.speed;
			}
		}
	}
	
	public class Arm extends Motorized
	{
		protected CalStatus getCalStatus()
		{
			return calStatus;
		}


		protected void setCalStatus(CalStatus calStatus)
		{
			this.calStatus = calStatus;
		}

		public ArmParams gripper = new ArmParams();
		public ArmParams wrist = new ArmParams();
		public ArmParams shoulder = new ArmParams();
		public ArmParams elbow = new ArmParams();

		private CalStatus calStatus = CalStatus.READY;
		
		public void stop()
		{
			gripper.stop();
			wrist.stop();
			elbow.stop();
			shoulder.stop();
		}
		
		
		public void setSpeedLimit(byte speed)
		{
			if(speed < 0) speedLimit = 0;
			else if(speed > 15) speedLimit = 15;
			else speedLimit = speed;
			
			if(wrist.getSpeedLimit() > speedLimit) wrist.setSpeedLimit(speedLimit);
			if(gripper.getSpeedLimit() > speedLimit) gripper.setSpeedLimit(speedLimit);
			if(shoulder.getSpeedLimit() > speedLimit) shoulder.setSpeedLimit(speedLimit);
			if(elbow.getSpeedLimit() > speedLimit) elbow.setSpeedLimit(speedLimit);
		}
		
		protected class ArmParams extends Params
		{
			private int position;
			private Direction exactDirection;

			public synchronized int getPosition()
			{
				return position;
			}

			public synchronized void setPosition(int position)
			{
				this.position = position;
			}
			
			public synchronized void setExactDirection(Direction exactDirection)
			{
				this.exactDirection = exactDirection;
			}
			
			public synchronized Direction getExactDirection()
			{
				return exactDirection;
			}
			
			public synchronized String getExactDirectionText()
			{
				if(exactDirection == Direction.FORWARD) return "forward";
				else if(exactDirection == Direction.BACKWARD) return "backward";
				else return "stopped";
			}
			
			public boolean equals(Object other)
			{
				if(other == null) return false;
				if(this == other) return true;
				
				if(other instanceof Params)
				{
					ArmParams o = (ArmParams)other;
					
					if((this.getDirection() == o.getDirection()) &&
							(this.getSpeed() == o.getSpeed()) &&
							(this.getPosition() == o.getPosition())) return true;
					else return false;
				}
				else return false;
			}
			
			public String toString()
			{
				String dir;
				
				switch(this.getDirection())
				{
					case FORWARD:
						dir = "forward";
						break;
					case BACKWARD:
						dir = "backward";
						break;
					default:
						dir = "stopped";
				};
				
				return "direction: " + dir + ", speed: " + this.getSpeed() +
						", pos: " + getPosition();
			}
		}
		
		public boolean equals(Object obj)
		{
			if(obj == null) return false;
			if(obj == this) return true;
			
			if(obj instanceof Motors)
			{
				Arm o = (Arm)obj;
				if(gripper.equals(o.gripper) &&
						wrist.equals(o.wrist) &&
						shoulder.equals(o.shoulder) &&
						elbow.equals(o.elbow)) return true;
			}
			
			return false;
		}
	}
	
	public class Motors extends Motorized
	{
		public Params left = new Params();
		public Params right = new Params();
		
		public void stop()
		{
			left.stop();
			right.stop();
		}
		
		public synchronized void setSpeedLimit(byte speed)
		{
			if(speed < 0) speedLimit = 0;
			else if(speed > 15) speedLimit = 15;
			else speedLimit = speed;
			
			left.setSpeedLimit(speedLimit);
			right.setSpeedLimit(speedLimit);
		}
		
		public boolean equals(Object obj)
		{
			if(obj == null) return false;
			if(obj == this) return true;
			
			if(obj instanceof Motors)
			{
				Motors o = (Motors)obj;
				if(left.equals(o.left) && right.equals(o.right)) return true;
			}
			
			return false;
		}
		
	}
	
	public class Battery 
	{
		public float getVoltage()
		{
			return voltage;
		}
		public void setVoltage(float voltage)
		{
			this.voltage = (float)(Math.round(voltage * 1000.0) / 1000.0);
		}
		public float getCurrent()
		{
			return current;
		}
		public void setCurrent(float current)
		{
			this.current = (float)(Math.round(current * 1000.0) / 1000.0);
		}
		private float voltage;
		private float current;
	}

	
	public class Server
	{
		
		private boolean rebootSystem = false;
		private boolean shutdownSystem = false;
		private boolean do_exit = false;
		
		public synchronized boolean mustExit()
		{
			return do_exit;
		}
		
		public synchronized boolean mustReboot()
		{
			return rebootSystem;
		}
		
		public synchronized boolean mustShutdown()
		{
			return shutdownSystem;
		}
		
		public synchronized void exit()
		{
			do_exit = true;
		}
		
		public synchronized void shutdown()
		{
			shutdownSystem = true;
		}
		
		public synchronized void reboot()
		{
			rebootSystem = true;
		}
		
		public boolean equals(Object obj)
		{
			if(obj == null) return false;
			if(obj == this) return true;
			
			if(obj instanceof Server)
			{
				Server o = (Server)obj;
				
				if((rebootSystem == o.rebootSystem) && (shutdownSystem == o.shutdownSystem) && (do_exit == o.do_exit)) return true;
			}
			return false;
		}
	}
	
	public class Lights
	{
		public synchronized boolean isLow()
		{
			return low;
		}
		public synchronized void setLow(boolean low)
		{
			this.low = low;
		}
		public synchronized boolean isHigh()
		{
			return high;
		}
		public synchronized void setHigh(boolean high)
		{
			this.high = high;
		}
		public synchronized boolean isGripper()
		{
			return gripper;
		}
		public synchronized void setGripper(boolean gripper)
		{
			this.gripper = gripper;
		}
		public synchronized boolean isCamera()
		{
			return camera;
		}
		public synchronized void setCamera(boolean camera)
		{
			this.camera = camera;
		}
		private boolean low;
		private boolean high;
		private boolean gripper;
		private boolean camera;
	}
}
