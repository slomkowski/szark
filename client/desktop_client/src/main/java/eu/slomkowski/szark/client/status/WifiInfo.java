package eu.slomkowski.szark.client.status;

import com.google.gson.annotations.SerializedName;

public class WifiInfo {
	@SerializedName("b")
	private double bitrate;

	@SerializedName("s")
	private double strength;

	public double getBitrate() {
		return bitrate;
	}

	public double getStrength() {
		return strength;
	}
}
