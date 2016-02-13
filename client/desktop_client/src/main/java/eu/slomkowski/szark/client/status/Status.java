package eu.slomkowski.szark.client.status;

import com.google.gson.annotations.Expose;
import com.google.gson.annotations.SerializedName;

import java.time.LocalTime;

public class Status {

    @Expose
    @SerializedName("arm")
    public JointSet joints;

    @Expose
    @SerializedName("motor")
    public MotorSet motors;

    @Expose(deserialize = false, serialize = true)
    @SerializedName("light")
    public LightSet lights;

    @Expose(deserialize = true, serialize = false)
    @SerializedName("batt")
    public Battery battery;

    @Expose(deserialize = true, serialize = false)
    @SerializedName("wifi")
    public WifiInfo wifi;

    @Expose(deserialize = false, serialize = true)
    @SerializedName("ks_en")
    private boolean killSwitchEnable;

    @Expose(deserialize = true, serialize = false)
    @SerializedName("ks_stat")
    private KillSwitchStatus receivedKillSwitchStatus;

    @Expose(serialize = true, deserialize = true)
    private int serial = 0;

    @Expose(serialize = true, deserialize = false)
    @SerializedName("tss")
    private LocalTime sendTimestamp = LocalTime.now();

    @Expose(serialize = false, deserialize = true)
    @SerializedName("tsr")
    private LocalTime receiveTimestamp;

    @Expose(serialize = false, deserialize = true)
    @SerializedName("tspb")
    private LocalTime beginProcessingTimestamp;

    @Expose(serialize = false, deserialize = true)
    @SerializedName("tspe")
    private LocalTime endProcessingTimestamp;

    public Status() {
        clean();
    }

    public synchronized void clean() {
        battery = new Battery();
        motors = new MotorSet();
        lights = new LightSet();
        joints = new JointSet();

        wifi = new WifiInfo();
        killSwitchEnable = true;
        receivedKillSwitchStatus = KillSwitchStatus.ACTIVE_SOFTWARE;
        serial = 0;
    }

    public void incrementSerial() {
        serial++;
    }

    public int getSerial() {
        return serial;
    }

    public void setSerial(int serial) {
        this.serial = serial;
    }

    public synchronized boolean isKillSwitchEnable() {
        return killSwitchEnable;
    }

    public synchronized void setKillSwitchEnable(boolean killSwitchEnable) {
        this.killSwitchEnable = killSwitchEnable;
    }

    public KillSwitchStatus getReceivedKillSwitchStatus() {
        return receivedKillSwitchStatus;
    }

    public LocalTime getSendTimestamp() {
        return sendTimestamp;
    }

    public void setSendTimestamp(LocalTime sendTimestamp) {
        this.sendTimestamp = sendTimestamp;
    }

    public LocalTime getEndProcessingTimestamp() {
        return endProcessingTimestamp;
    }

    public void setEndProcessingTimestamp(LocalTime endProcessingTimestamp) {
        this.endProcessingTimestamp = endProcessingTimestamp;
    }

    public LocalTime getBeginProcessingTimestamp() {
        return beginProcessingTimestamp;
    }

    public void setBeginProcessingTimestamp(LocalTime beginProcessingTimestamp) {
        this.beginProcessingTimestamp = beginProcessingTimestamp;
    }

    public LocalTime getReceiveTimestamp() {
        return receiveTimestamp;
    }

    public void setReceiveTimestamp(LocalTime receiveTimestamp) {
        this.receiveTimestamp = receiveTimestamp;
    }
}
