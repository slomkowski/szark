package eu.slomkowski.szark.client.camera;

public enum CameraMode {
	RAW("RAW"),
	HUD("RAW");

	private String mnemonic;

	private CameraMode(String mnemonic) {
		this.mnemonic = mnemonic;
	}

	public String getMnemonic() {
		return mnemonic;
	}
}
