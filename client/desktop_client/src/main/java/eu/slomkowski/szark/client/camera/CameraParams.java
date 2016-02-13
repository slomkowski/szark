package eu.slomkowski.szark.client.camera;

import com.google.gson.annotations.Expose;


class CameraParams {
    @Expose
    private int serial = 0;

    @Expose
    private boolean drawHud = false;

    @Expose
    private int quality;

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
}
