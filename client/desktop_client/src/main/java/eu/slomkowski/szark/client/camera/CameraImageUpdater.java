package eu.slomkowski.szark.client.camera;

import com.google.gson.Gson;
import eu.slomkowski.szark.client.HardcodedConfiguration;
import eu.slomkowski.szark.client.gson.GsonFactory;
import lombok.extern.slf4j.Slf4j;

import javax.imageio.ImageIO;
import javax.swing.*;
import java.awt.*;
import java.awt.image.BufferedImage;
import java.io.ByteArrayInputStream;
import java.io.IOException;
import java.net.*;
import java.time.LocalTime;
import java.time.temporal.ChronoUnit;
import java.util.List;
import java.util.concurrent.atomic.AtomicBoolean;

@Slf4j
public class CameraImageUpdater extends JLabel {

    private static final int JPEG_QUALITY = 45;

    private final Gson gson = GsonFactory.getGson();
    private UpdateTask updateTask = null;
    private CameraType chosenCameraType = CameraType.GRIPPER;
    private InetSocketAddress address;
    private DatagramSocket socket;
    private boolean enabled = false;
    private CameraMode cameraMode = CameraMode.HUD;

    public void setChosenCameraType(CameraType chosenCameraType) {
        if (chosenCameraType == this.chosenCameraType && enabled) {
            return;
        }

        if (enabled) {
            disableCameraView();
        }
        this.chosenCameraType = chosenCameraType;
        enableCameraView(address.getAddress());
    }

    public void setCameraMode(CameraMode cameraMode) {
        this.cameraMode = cameraMode;
    }

    public void enableCameraView(InetAddress host) {
        setIcon(null);

        try {
            socket = new DatagramSocket();
            socket.setSoTimeout(HardcodedConfiguration.CAMERA_TIMEOUT);
            this.address = new InetSocketAddress(host, chosenCameraType.getPort());
        } catch (final IOException e) {
            e.printStackTrace();
            System.exit(1);
        }

        enabled = true;

        updateTask = new UpdateTask();
        updateTask.execute();
    }

    public void disableCameraView() {
        if (updateTask != null) {
            updateTask.stopTask();

            updateTask = null;
        }

        enabled = false;
        setIcon(new ImageIcon(getClass().getResource(HardcodedConfiguration.DEFAULT_LOGO)));
        repaint();
    }

    @Override
    protected void paintComponent(Graphics g) {
        super.paintComponent(g);

        if (updateTask == null) {
            return;
        }

        g.drawImage(updateTask.getBufferedImage(), 0, 0, this);
    }

    private class UpdateTask extends SwingWorker<Void, BufferedImage> {
        private byte[] inputBuffer = new byte[100000];
        private AtomicBoolean needToStop = new AtomicBoolean(false);
        private BufferedImage image = null;
        private CameraParams cameraParams = new CameraParams();

        private int serial = 1;

        @Override
        protected Void doInBackground() throws Exception {
            while (!needToStop.get()) {
                try {
                    cameraParams.setQuality(JPEG_QUALITY);
                    cameraParams.setDrawHud(cameraMode == CameraMode.HUD);
                    cameraParams.setSerial(serial);
                    cameraParams.setSendTimestamp(LocalTime.now());
                    cameraParams.setType(chosenCameraType);

                    String sendPacketPayload = gson.toJson(cameraParams);
                    DatagramPacket sendPacket = new DatagramPacket(sendPacketPayload.getBytes(), sendPacketPayload.length(), address);

                    socket.send(sendPacket);

                    DatagramPacket receivedPacket = new DatagramPacket(inputBuffer, inputBuffer.length);

                    try {
                        socket.receive(receivedPacket);
                    } catch (final SocketTimeoutException e) {
                        log.warn("Camera receive timeout.");
                        continue;
                    }

                    if (needToStop.get()) {
                        break;
                    }

                    int headerEnd = 0;
                    for (int i = 0; i < inputBuffer.length; ++i) {
                        if (inputBuffer[i] == 0) {
                            headerEnd = i;
                            break;
                        }
                    }

                    String json = new String(inputBuffer, 0, headerEnd);
                    CameraParams status = gson.fromJson(json, CameraParams.class);

                    if (cameraParams.getSerial() == status.getSerial()) {

                        long to = ChronoUnit.MILLIS.between(cameraParams.getSendTimestamp(), status.getReceiveTimestamp());
                        long from = ChronoUnit.MILLIS.between(status.getSendResponseTimestamp(), LocalTime.now());
                        long processingTime = ChronoUnit.MILLIS.between(status.getReceiveTimestamp(), status.getSendResponseTimestamp());

                        log.info("Trip: to {} ms, from {} ms, process: {} ms.", to, from, processingTime);
                    }

                    ByteArrayInputStream stream = new ByteArrayInputStream(inputBuffer, headerEnd + 1, receivedPacket.getLength() - headerEnd - 1);
                    BufferedImage img = ImageIO.read(stream);
                    stream.close();

                    if (img == null) {
                        throw new Exception("could not parse image. Probably malformed response.");
                    }

                    serial++;

                    publish(img);

                } catch (final Exception e) {
                    e.printStackTrace();

                    SwingUtilities.invokeLater(() -> {
                        disableCameraView();

                        log.error("Camera communication error:", e.getMessage(), e);

                        JOptionPane.showMessageDialog(CameraImageUpdater.this,
                                String.format("Camera communication error: %s. Disabling camera.",
                                        e.getMessage() != null ? e.getMessage() : e.getClass().getName()),
                                "Network error",
                                JOptionPane.ERROR_MESSAGE);
                    });

                    return null;
                }
            }

            return null;
        }

        @Override
        protected void process(List<BufferedImage> chunks) {
            image = chunks.get(chunks.size() - 1);
            CameraImageUpdater.this.setSize(image.getWidth(), image.getHeight());
            CameraImageUpdater.this.repaint();
        }

        public void stopTask() {
            needToStop.set(true);

            try {
                this.get();
                socket.close();
            } catch (Exception e) {
                e.printStackTrace();
            }
        }

        public BufferedImage getBufferedImage() {
            return image;
        }
    }
}