package eu.slomkowski.szark.client;
public class Motors 
{
	public MotorParams left = new MotorParams();
	public MotorParams right = new MotorParams();
	
	public void stop()
	{
		left.stop();
		right.stop();
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