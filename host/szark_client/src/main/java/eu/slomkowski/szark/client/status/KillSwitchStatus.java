package eu.slomkowski.szark.client.status;

import com.google.gson.annotations.SerializedName;

public enum KillSwitchStatus {
	@SerializedName("inactive")
	INACTIVE("inactive"),

	@SerializedName("hardware")
	ACTIVE_HARDWARE("hardware"),

	@SerializedName("software")
	ACTIVE_SOFTWARE("software");

	private String name;

	KillSwitchStatus(String name) {
		this.name = name;
	}

	@Override
	public String toString() {
		return name;
	}
}
