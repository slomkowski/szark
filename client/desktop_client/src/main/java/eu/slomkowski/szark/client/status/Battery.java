package eu.slomkowski.szark.client.status;

import com.google.gson.annotations.Expose;
import com.google.gson.annotations.SerializedName;

public class Battery {

    @Expose(deserialize = true, serialize = false)
    @SerializedName("i")
    private float current;

    @Expose(deserialize = true, serialize = false)
    @SerializedName("u")
    private float voltage;

    public float getCurrent() {
        return current;
    }

    public float getVoltage() {
        return voltage;
    }
}
