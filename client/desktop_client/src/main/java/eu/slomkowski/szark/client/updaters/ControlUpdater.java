package eu.slomkowski.szark.client.updaters;

import com.google.gson.Gson;
import com.google.gson.GsonBuilder;
import eu.slomkowski.szark.client.HardcodedConfiguration;
import eu.slomkowski.szark.client.gui.MainWindowLogic;
import eu.slomkowski.szark.client.pad.PadStatusUpdater;
import eu.slomkowski.szark.client.status.CalibrationStatus;
import eu.slomkowski.szark.client.status.KillSwitchStatus;
import eu.slomkowski.szark.client.status.Status;

import javax.swing.*;
import java.io.IOException;
import java.net.*;
import java.util.List;
import java.util.concurrent.atomic.AtomicBoolean;

public class ControlUpdater extends SwingWorker<Void, Status> {
    private final Gson gson;
    private final Status status;
    private final AtomicBoolean needToStop = new AtomicBoolean(false);
    private final MainWindowLogic mainWindow;
    private PadStatusUpdater padStatusUpdater;
    private InetSocketAddress address;
    private DatagramSocket socket;
    private byte[] inputBuffer = new byte[1024];

    public ControlUpdater(MainWindowLogic mainWindow,
                          InetAddress host,
                          Status status,
                          PadStatusUpdater padStatusUpdater) {
        this.mainWindow = mainWindow;
        this.status = status;
        this.padStatusUpdater = padStatusUpdater;

        this.gson = new GsonBuilder().excludeFieldsWithoutExposeAnnotation().create();

        try {
            address = new InetSocketAddress(host, HardcodedConfiguration.CONTROL_SERVER_PORT);
            socket = new DatagramSocket();
            socket.setSoTimeout(HardcodedConfiguration.CONTROL_SERVER_TIMEOUT);
        } catch (IOException e) {
            e.printStackTrace();
            System.exit(2);
        }
    }

    private synchronized Status updateCycle() throws HardwareStoppedException, IOException {
        String output = gson.toJson(status);

        DatagramPacket packet = new DatagramPacket(output.getBytes(), output.length(), address);
        socket.send(packet);

        DatagramPacket receivedPacket = new DatagramPacket(inputBuffer, inputBuffer.length);

        try {
            socket.receive(receivedPacket);
        } catch (final SocketTimeoutException e) {
            System.err.println("Control receive timeout");
            status.setSerial(0);
            return null;
        }

        String receivedJson = new String(receivedPacket.getData(), 0, receivedPacket.getLength());
        Status receivedStatus = gson.fromJson(receivedJson, Status.class);

        boolean responseMatchesRequest = false;

        if (status.getSerial() == receivedStatus.getSerial()) {
            responseMatchesRequest = true;
        }

        if (receivedStatus.joints.getCalStatus() == CalibrationStatus.IN_PROGRESS) {
            status.joints.setBeginCalibration(false);
        }

        status.incrementSerial();

        if (!status.isKillSwitchEnable() &&
                receivedStatus.getReceivedKillSwitchStatus() != KillSwitchStatus.INACTIVE) {

            if (receivedStatus.getReceivedKillSwitchStatus() == KillSwitchStatus.ACTIVE_HARDWARE) {
                throw new HardwareStoppedException("Kill switch has been pressed!");
            } else if (responseMatchesRequest) {
                throw new HardwareStoppedException("Other client has disabled the device");
            }
        }

        return receivedStatus;
    }

    @Override
    protected Void doInBackground() throws Exception {
        while (!needToStop.get()) {
            try {
                while (!needToStop.get()) {

                    if (padStatusUpdater != null) {
                        padStatusUpdater.fillWithCurrentData(status);
                    }

                    Status receivedStatus = updateCycle();

                    if (receivedStatus != null) {
                        publish(receivedStatus);
                    }

                    Thread.sleep(HardcodedConfiguration.CONTROL_SERVER_REFRESH_INTERVAL, 0);
                }

            } catch (final IOException e) {
                SwingUtilities.invokeAndWait(new Runnable() {
                    @Override
                    public void run() {
                        mainWindow.performControlServerDisconnection(false);

                        JOptionPane.showMessageDialog(mainWindow,
                                String.format("Control communication error: %s. Disabling control.",
                                        e.getMessage() != null ? e.getMessage() : e.getClass().getName()),
                                "Network error",
                                JOptionPane.ERROR_MESSAGE);
                    }
                });

                return null;
            } catch (final HardwareStoppedException e) {
                SwingUtilities.invokeAndWait(() -> {
                    mainWindow.performKillSwitchEnable();

                    JOptionPane.showMessageDialog(mainWindow,
                            "Kill switch: " + e.getMessage(),
                            "Kill switch activated",
                            JOptionPane.WARNING_MESSAGE);
                });
            } catch (final Exception e) {
                e.printStackTrace();
                throw e;
            }
        }
        return null;
    }

    @Override
    protected void process(List<Status> chunks) {
        mainWindow.updateIndicators(chunks.get(chunks.size() - 1));
    }

    @Override
    protected void done() {
        try {
            socket.close();
        } catch (Exception e) {
            e.printStackTrace();
        }
    }

    public void stopTask() {
        needToStop.set(true);

        try {
            this.get();
        } catch (Exception e) {
            e.printStackTrace();
        }
    }

    public class HardwareStoppedException extends Exception {
        public HardwareStoppedException(String msg) {
            super(msg);
        }
    }
}
