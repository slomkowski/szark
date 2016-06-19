package eu.slomkowski.szark.client.status;

import com.google.gson.annotations.SerializedName;

public enum CalibrationStatus {
    @SerializedName("none")
    NONE,

    @SerializedName("prog")
    IN_PROGRESS,

    @SerializedName("done")
    DONE
}
