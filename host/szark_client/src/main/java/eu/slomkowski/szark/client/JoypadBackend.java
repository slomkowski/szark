package eu.slomkowski.szark.client;
import net.java.games.input.*;
import net.java.games.input.Component.Identifier;

/**
 * This interface stores the configuration of joypad.
 */
interface JoypadConfiguration
{
	
	String COTROLLER_NAME = "DragonRise Inc.   Generic   USB  Joystick  ";
	
	Axis MOV_FORW_BACK    = new Axis("y", 1.0f);
	Axis MOV_LEFT_RIGHT   = new Axis("x", 1.0f);
	Axis ARM_SHOULDER = new Axis("rz", 1.0f);
	Axis ARM_ELBOW    = new Axis("rx", 1.0f);
	
	// WARNING! - wrist and gripper are hardcoded to use POV hat
	
	float GRIPPER_MFACTOR = 1.0f;
	float WRIST_MFACTOR = 1.0f;

	/**
	 * Class to store an axis description.
	 */
	class Axis
	{
		/**
		 * 
		 * @param name String name of specific axis, taken from Component.getName().
		 * @param mFactor Scaling factor of the value read from the axis. Negative values invert the axis.
		 */
		public Axis(String name, float mFactor)
		{
			this.name = name;
			this.mFactor = mFactor;
		}

		private String name;
		private float mFactor;
		
		public float getMFactor()
		{
			if (mFactor < - 1.0f) return - 1.0f;
			else if (mFactor > 1.0f) return 1.0f;
			else return mFactor;
		}
		
		public String getName()
		{
			return name;
		}
	}
}

public class JoypadBackend
{

	public class InvalidJoypadException extends Exception
	{
		public InvalidJoypadException(String string)
		{
			super(string);
		}

		private static final long serialVersionUID = 1L;
	}
	
	public void poll()
	{
		controller.poll();
	}
	
	public float getGripperVal()
	{
		float data = POVComp.getPollData();
		
		if((data == 0.25f) || (data == 0.125f) || (data == 0.375f))
			return JoypadConfiguration.GRIPPER_MFACTOR * 1.0f;
		if((data == 0.75f) || (data == 0.625f) || (data == 0.875f))
			return JoypadConfiguration.GRIPPER_MFACTOR * -1.0f;
		
		return 0.0f;
	}
	
	public float getWristVal()
	{
		float data = POVComp.getPollData();
		
		if((data == 0.625f) || (data == 0.5f) || (data == 0.375f))
			return JoypadConfiguration.WRIST_MFACTOR * 1.0f;
		if((data == 0.125f) || (data == 1.0f) || (data == 0.875f))
			return JoypadConfiguration.WRIST_MFACTOR * -1.0f;
		
		return 0.0f;
	}
	
	public float getForwBackVal()
	{
		return -1.0f * forwBackComp.getPollData() * JoypadConfiguration.MOV_FORW_BACK.getMFactor();
	}
	
	public float getLeftRightVal()
	{
		return -1.0f * leftRightComp.getPollData() * JoypadConfiguration.MOV_LEFT_RIGHT.getMFactor();
	}
	
	public float getShoulderVal()
	{
		return armShoulderComp.getPollData() * JoypadConfiguration.ARM_SHOULDER.getMFactor();
	}
	
	public float getElbowVal()
	{
		return armElbowComp.getPollData() * JoypadConfiguration.ARM_ELBOW.getMFactor();
	}
	
	public JoypadBackend() throws InvalidJoypadException
	{
		ControllerEnvironment ce = ControllerEnvironment.getDefaultEnvironment();
		Controller[] cs = ce.getControllers();

		boolean valid = false;
		
		for (int i = 0; i < cs.length; i++ )
		{
			if ((cs[i].getType() == Controller.Type.GAMEPAD) || (cs[i].getType() == Controller.Type.STICK))
			{
				if(cs[i].getName().equals(JoypadConfiguration.COTROLLER_NAME))
				{
					valid = true;
					controller = cs[i];
					
					break;
				}
			}
		}
		
		if(valid == false)
		{
			throw new InvalidJoypadException("Hardcoded joypad not found.");
		}
		
		Component[] compList = controller.getComponents();
		
		// POV hat
		valid = false;
		for(int i = 0; i < compList.length; i++) if(compList[i].getIdentifier() == Identifier.Axis.POV)
		{
			valid = true;
			POVComp = compList[i];
			break;
		}
		if(valid == false) throw new InvalidJoypadException("Hardcoded joypad has no POV hat.");
		
		// forward - backward axis
		valid = false;
		for (int i = 0; i < compList.length; i++ )
		{
			if (compList[i].getName().equals(JoypadConfiguration.MOV_FORW_BACK.getName()))
			{
				valid = true;
				forwBackComp = compList[i];
				break;
			}
		}
		if (valid == false) throw new InvalidJoypadException("Forward/backward axis: the joystick has no " +
				JoypadConfiguration.MOV_FORW_BACK.getName() + " axis.");
		
		// left-right axis
		valid = false;
		for (int i = 0; i < compList.length; i++ )
		{
			if (compList[i].getName().equals(
					JoypadConfiguration.MOV_LEFT_RIGHT.getName()))
			{
				valid = true;
				leftRightComp = compList[i];
				break;
			}
		}
		if (valid == false)
			throw new InvalidJoypadException(
					"Left/right axis: the joystick has no "
							+ JoypadConfiguration.MOV_FORW_BACK
									.getName() + " axis.");

		// arm shoulder
		valid = false;
		for (int i = 0; i < compList.length; i++ )
		{
			if (compList[i].getName().equals(
					JoypadConfiguration.ARM_SHOULDER.getName()))
			{
				valid = true;
				armShoulderComp = compList[i];
				break;
			}
		}
		if (valid == false)
			throw new InvalidJoypadException(
					"Arm shoudler axis: the joystick has no "
							+ JoypadConfiguration.MOV_FORW_BACK
									.getName() + " axis.");

		// arm elbow axis
		valid = false;
		for (int i = 0; i < compList.length; i++ )
		{
			if (compList[i].getName().equals(
					JoypadConfiguration.ARM_ELBOW.getName()))
			{
				valid = true;
				armElbowComp = compList[i];
				break;
			}
		}
		if (valid == false)
			throw new InvalidJoypadException(
					"Arm elbow axis: the joystick has no "
							+ JoypadConfiguration.MOV_FORW_BACK
									.getName() + " axis.");

	}
	
	private Controller controller;
	
	private Component forwBackComp;
	private Component leftRightComp;
	private Component armShoulderComp;
	private Component armElbowComp;
	private Component POVComp;
	
}