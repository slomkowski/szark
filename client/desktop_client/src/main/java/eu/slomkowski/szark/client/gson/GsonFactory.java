package eu.slomkowski.szark.client.gson;

import com.google.gson.Gson;
import com.google.gson.GsonBuilder;

import java.time.LocalTime;

public class GsonFactory {
    private static Gson gson = null;

    public static Gson getGson() {
        if (gson == null) {
            gson = new GsonBuilder()
                           .excludeFieldsWithoutExposeAnnotation()
                           .registerTypeAdapter(LocalTime.class, new LocalTimeTypeAdapter())
                           .create();
        }
        return gson;
    }
}
