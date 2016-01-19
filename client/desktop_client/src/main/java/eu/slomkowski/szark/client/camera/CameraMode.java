package eu.slomkowski.szark.client.camera;

import com.google.gson.annotations.SerializedName;

public enum CameraMode {
    @SerializedName("raw")
    RAW("RAW"),

    @SerializedName("hud")
    HUD("HUD");

    private String mnemonic;

    CameraMode(String mnemonic) {
        this.mnemonic = mnemonic;
    }

    public String getMnemonic() {
        return mnemonic;
    }
}
