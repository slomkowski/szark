package eu.slomkowski.szark.client.status;

import com.google.gson.annotations.SerializedName;

public enum Direction {
    @SerializedName("backward")
    BACKWARD("backward"),

    @SerializedName("forward")
    FORWARD("forward"),

    @SerializedName("stop")
    STOP("stop");

    private String name;

    Direction(String name) {
        this.name = name;
    }

    @Override
    public String toString() {
        return name;
    }
}
