package eu.slomkowski.szark.client.status;

import com.google.gson.annotations.Expose;
import com.google.gson.annotations.SerializedName;

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

	public void setCurrent(float current) {
		this.current = (float) (Math.round(current * 1000.0) / 1000.0);
	}

	public float getVoltage() {
		return voltage;
	}

	public void setVoltage(float voltage) {
		this.voltage = (float) (Math.round(voltage * 1000.0) / 1000.0);
	}
}
