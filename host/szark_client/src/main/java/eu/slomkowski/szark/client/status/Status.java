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

	@Expose(deserialize = true, serialize = false)
	@SerializedName("wifi")
	private int wirelessPower;

	@Expose(deserialize = false, serialize = true)
	@SerializedName("ks_en")
	private boolean killswitchEnable;

	@Expose(deserialize = true, serialize = false)
	@SerializedName("ks_stat")
	private KillSwitchStatus receivedKillSwitchStatus;

	@Expose(serialize = true, deserialize = false)
	@SuppressWarnings("unused")
	private int serial = 0;

	public Status() {
		clean();
	}

	public synchronized void clean() {
		battery = new Battery();
		motors = new MotorSet();
		lights = new LightSet();
		joints = new JointSet();

		wirelessPower = 0;
		killswitchEnable = true;
		receivedKillSwitchStatus = KillSwitchStatus.ACTIVE_SOFTWARE;
		serial = 0;
	}

	public void incrementSerial() {
		serial++;
	}

	public int getWirelessPower() {
		return wirelessPower;
	}

	public synchronized boolean isKillswitchEnable() {
		return killswitchEnable;
	}

	public synchronized void setKillswitchEnable(boolean killswitchEnable) {
		this.killswitchEnable = killswitchEnable;
	}

	public KillSwitchStatus getReceivedKillSwitchStatus() {
		return receivedKillSwitchStatus;
	}
}
