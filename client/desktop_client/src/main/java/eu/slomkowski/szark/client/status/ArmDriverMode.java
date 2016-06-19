package eu.slomkowski.szark.client.status;

import com.google.gson.annotations.SerializedName;

public enum ArmDriverMode {
    @SerializedName("directional")
    DIRECTIONAL,

    @SerializedName("positional")
    POSITIONAL,

    @SerializedName("calibrating")
    CALIBRATING
}
