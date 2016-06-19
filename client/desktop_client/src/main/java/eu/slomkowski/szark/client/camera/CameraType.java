package eu.slomkowski.szark.client.camera;

import com.google.gson.annotations.SerializedName;
import lombok.Getter;

@Getter
public enum CameraType {

    @SerializedName("gripper")
    GRIPPER("gripper", 10192),

    @SerializedName("back")
    BACK("back", 10192);

    private final String input;

    private final int port;

    CameraType(String input, int port) {
        this.input = input;
        this.port = port;
    }
}