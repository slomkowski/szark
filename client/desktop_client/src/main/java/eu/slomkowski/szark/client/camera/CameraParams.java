package eu.slomkowski.szark.client.camera;

import com.google.gson.annotations.Expose;


class CameraParams {
    @Expose
    private int serial = 0;

    @Expose
    private boolean drawHud = false;

    @Expose
    private boolean compressed = false;

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

    public boolean isCompressed() {
        return compressed;
    }

    public void setCompressed(boolean compressed) {
        this.compressed = compressed;
    }
}
