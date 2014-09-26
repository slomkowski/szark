package eu.slomkowski.szark.client.status;

import com.google.gson.annotations.Expose;
import com.google.gson.annotations.SerializedName;

public class Status {

	@Expose
	@SerializedName("arm")
	public JointSet joints;

	@Expose
	@SerializedName("motor")
	public MotorSet motors;

	@Expose(deserialize = false, serialize = true)
	@SerializedName("light")
	public LightSet lights;

	@Expose(deserialize = true, serialize = false)
	@SerializedName("batt")
	public Battery battery;

	@Expose(serialize = true, deserialize = false)
	private int serial = 0;

	@Expose(deserialize = false, serialize = true)
	@SerializedName("killswitch")
	private boolean emergencyStopped;

	@Expose(deserialize = true, serialize = false)
	@SerializedName("wifi")
	private int wirelessPower;

	public Status() {
		clean();
	}

	public synchronized void clean() {
		battery = new Battery();
		motors = new MotorSet();
		lights = new LightSet();
		joints = new JointSet();

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

	public void setWirelessPower(int wirelessPower) {
		this.wirelessPower = wirelessPower;
	}

	public synchronized boolean isEmergencyStopped() {
		return emergencyStopped;
	}

	public synchronized void setEmergencyStopped(boolean emergencyStopped) {
		this.emergencyStopped = emergencyStopped;
	}
}
