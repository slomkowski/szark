package eu.slomkowski.szark.client.camera;

import com.google.gson.annotations.Expose;
import com.google.gson.annotations.SerializedName;
import lombok.Data;

import java.time.LocalTime;

@Data
class CameraParams {
    @Expose
    private int serial = 0;

    @Expose
    private boolean drawHud = false;

    @Expose
    private int quality;

    @SerializedName("input")
    @Expose(serialize = true, deserialize = false)
    private CameraType type;

    @Expose(serialize = true, deserialize = false)
    @SerializedName("tss")
    private LocalTime sendTimestamp = LocalTime.now();

    @Expose(serialize = false, deserialize = true)
    @SerializedName("tsr")
    private LocalTime receiveTimestamp = LocalTime.now();

    @Expose(serialize = false, deserialize = true)
    @SerializedName("tssr")
    private LocalTime sendResponseTimestamp = LocalTime.now();

}
