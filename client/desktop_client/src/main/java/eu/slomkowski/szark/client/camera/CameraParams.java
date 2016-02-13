package eu.slomkowski.szark.client.camera;

import com.google.gson.annotations.Expose;
import com.google.gson.annotations.SerializedName;

import java.time.LocalTime;


class CameraParams {
    @Expose
    private int serial = 0;

    @Expose
    private boolean drawHud = false;

    @Expose
    private int quality;

    @Expose(serialize = true, deserialize = false)
    @SerializedName("tss")
    private LocalTime sendTimestamp = LocalTime.now();

    @Expose(serialize = false, deserialize = true)
    @SerializedName("tsr")
    private LocalTime receiveTimestamp = LocalTime.now();

    @Expose(serialize = false, deserialize = true)
    @SerializedName("tssr")
    private LocalTime sendResponseTimestamp = LocalTime.now();

    public int getSerial() {
        return serial;
    }

    public void setSerial(int serial) {
        this.serial = serial;
    }

    public boolean isDrawHud() {
        return drawHud;
    }

    public void setDrawHud(boolean drawHud) {
        this.drawHud = drawHud;
    }

    public int getQuality() {
        return quality;
    }

    public void setQuality(int quality) {
        this.quality = quality;
    }

    public LocalTime getSendTimestamp() {
        return sendTimestamp;
    }

    public void setSendTimestamp(LocalTime sendTimestamp) {
        this.sendTimestamp = sendTimestamp;
    }

    public LocalTime getReceiveTimestamp() {
        return receiveTimestamp;
    }

    public void setReceiveTimestamp(LocalTime receiveTimestamp) {
        this.receiveTimestamp = receiveTimestamp;
    }

    public LocalTime getSendResponseTimestamp() {
        return sendResponseTimestamp;
    }

    public void setSendResponseTimestamp(LocalTime sendResponseTimestamp) {
        this.sendResponseTimestamp = sendResponseTimestamp;
    }
}
