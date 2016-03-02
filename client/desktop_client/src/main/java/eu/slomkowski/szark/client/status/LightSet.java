package eu.slomkowski.szark.client.status;

import com.google.gson.annotations.Expose;
import com.google.gson.annotations.SerializedName;

public class LightSet {

    @Expose
    @SerializedName("cam")
    private boolean gripper;

    @Expose
    @SerializedName("l")
    private boolean high;

    @Expose
    @SerializedName("r")
    private boolean low;

    public synchronized boolean isGripper() {
        return gripper;
    }

    public synchronized void setGripper(boolean gripper) {
        this.gripper = gripper;
    }

    public synchronized boolean isHigh() {
        return high;
    }

    public synchronized void setHigh(boolean high) {
        this.high = high;
    }

    public synchronized boolean isLow() {
        return low;
    }

    public synchronized void setLow(boolean low) {
        this.low = low;
    }
}
