package eu.slomkowski.szark.client.status;

import com.google.gson.annotations.Expose;
import com.google.gson.annotations.SerializedName;

public class WifiInfo {
	@Expose
	@SerializedName("s")
	private double strength;

	public double getStrength() {
		return strength;
	}
}
