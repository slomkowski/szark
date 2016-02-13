package eu.slomkowski.szark.client.camera;

import com.google.gson.Gson;
import eu.slomkowski.szark.client.HardcodedConfiguration;
import eu.slomkowski.szark.client.gson.GsonFactory;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import javax.imageio.ImageIO;
import javax.swing.*;
import java.awt.*;
import java.awt.image.BufferedImage;
import java.io.ByteArrayInputStream;
import java.io.IOException;
import java.net.*;
import java.util.List;
import java.util.concurrent.atomic.AtomicBoolean;

public class CameraImageUpdater extends JLabel {

    private static final int JPEG_QUALITY = 45;

    private Logger logger = LoggerFactory.getLogger(CameraImageUpdater.class);

    private final Gson gson = GsonFactory.getGson();
    private UpdateTask updateTask = null;
    private CameraType chosenCameraType = CameraType.HEAD;
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

            int port = chosenCameraType == CameraType.GRIPPER
                    ? HardcodedConfiguration.CAMERA_PORT_GRIPPER
                    : HardcodedConfiguration.CAMERA_PORT_HEAD;

            this.address = new InetSocketAddress(host, port);
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

        @Override
        protected Void doInBackground() throws Exception {
            while (!needToStop.get()) {
                try {
                    cameraParams.setQuality(JPEG_QUALITY);
                    cameraParams.setDrawHud(cameraMode == CameraMode.HUD);
                    cameraParams.setSerial(333);//todo

                    String sendPacketPayload = gson.toJson(cameraParams);
                    DatagramPacket sendPacket = new DatagramPacket(sendPacketPayload.getBytes(), sendPacketPayload.length(), address);

                    socket.send(sendPacket);

                    DatagramPacket receivedPacket = new DatagramPacket(inputBuffer, inputBuffer.length);

                    try {
                        socket.receive(receivedPacket);
                    } catch (final SocketTimeoutException e) {
                        logger.warn("Camera receive timeout.");
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

                    ByteArrayInputStream stream = new ByteArrayInputStream(inputBuffer, headerEnd + 1, receivedPacket.getLength() - headerEnd - 1);
                    BufferedImage img = ImageIO.read(stream);
                    stream.close();

                    if (img == null) {
                        throw new Exception("could not parse image. Probably malformed response.");
                    }

                    publish(img);

                } catch (final Exception e) {
                    e.printStackTrace();

                    SwingUtilities.invokeLater(() -> {
                        disableCameraView();

                        logger.error("Camera communication error:", e.getMessage(), e);

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
        protected void done() {
            try {
                socket.close();
            } catch (Exception e) {
                e.printStackTrace();
            }
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
            } catch (Exception e) {
                e.printStackTrace();
            }
        }

        public BufferedImage getBufferedImage() {
            return image;
        }
    }
}