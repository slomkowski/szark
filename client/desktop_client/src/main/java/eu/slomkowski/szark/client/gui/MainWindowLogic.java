package eu.slomkowski.szark.client.gui;

import eu.slomkowski.szark.client.HardcodedConfiguration;
import eu.slomkowski.szark.client.camera.CameraMode;
import eu.slomkowski.szark.client.camera.CameraType;
import eu.slomkowski.szark.client.pad.PadException;
import eu.slomkowski.szark.client.pad.PadStatusUpdater;
import eu.slomkowski.szark.client.status.Direction;
import eu.slomkowski.szark.client.status.Status;
import eu.slomkowski.szark.client.updaters.ControlUpdater;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import javax.swing.*;
import javax.swing.event.ChangeEvent;
import java.awt.event.ActionEvent;
import java.net.Inet4Address;
import java.net.Inet6Address;
import java.net.InetAddress;
import java.net.UnknownHostException;
import java.util.Arrays;
import java.util.Optional;

/**
 * This class provides the logic for the main window. The appearance is moved to
 * view class.
 *
 * @author Michał Słomkowski
 */
public class MainWindowLogic extends MainWindowView {

    private static Logger logger = LoggerFactory.getLogger(MainWindowLogic.class);

    private final Status status = new Status();

    private final MoveControlWindow mConWin = new MoveControlWindow(status);

    private PadStatusUpdater padStatusUpdater = null;

    private boolean connected = false;

    private ControlUpdater controlUpdater;

    public MainWindowLogic() {
        performControlServerDisconnection(false);
        performCameraServerDisconnection();

        if (HardcodedConfiguration.JOYSTICK_ENABLE) {
            try {
                padStatusUpdater = new PadStatusUpdater();

                mWinMoveCtrl.setEnabled(false);
                mWinMoveCtrl.setText(mWinMoveCtrl.getText() + " (Disabled because of joystick)");
            } catch (final PadException e) {
                e.printStackTrace();

                JOptionPane.showMessageDialog(this,
                        "Joystick error: " + e.getMessage() + ". Joystick won't be used.",
                        "Joystick error",
                        JOptionPane.WARNING_MESSAGE);
            }
        }
    }

    public void performCameraServerConnection(InetAddress address) {
        cameraSelectGripper.setSelected(true);
        cameraDisplayHud.setSelected(true);
        setCameraControlsEnabled(true);

        cameraScreen.enableCameraView(address);
    }

    public void performCameraServerDisconnection() {
        setCameraControlsEnabled(false);
        cameraScreen.disableCameraView();
    }

    public synchronized void performKillSwitchDisable() {
        int currentSerial = status.getSerial();
        status.clean();
        status.setSerial(currentSerial);
        status.setKillSwitchEnable(false);

        startStopButton.setIcon(iconStop);
        setDeviceControlsEnabled(true);

        if (padStatusUpdater == null) {
            mConWin.setEnabled(true);
        }

        initializeControls();
    }

    public synchronized void performKillSwitchEnable() {
        status.setKillSwitchEnable(true);

        startStopButton.setIcon(iconStart);

        setDeviceControlsEnabled(false); // disabling all controls
        mConWin.setEnabled(false);

        initializeControls();
    }


    private void performControlServerConnection(InetAddress address) {
        performKillSwitchEnable();

        controlUpdater = new ControlUpdater(this, address, status, padStatusUpdater);
        controlUpdater.execute();

        setOverallControlsEnabled(true);
    }

    public void performControlServerDisconnection(boolean sendDisablingCommand) {
        performKillSwitchEnable();
        setOverallControlsEnabled(false);
        setDeviceControlsEnabled(false);

        if (sendDisablingCommand && controlUpdater != null) {
            status.clean();
            status.setKillSwitchEnable(true);

            controlUpdater.stopTask();
        }
        controlUpdater = null;
    }

    public void connectToAll() {
        final String hostName = connectHostnameField.getSelectedItem().toString();

        setConnectButtonsEnabled(false);
        setConnectButtonsText("Connecting...");

        connectHostnameField.setEnabled(false);

        new SwingWorker<Void, Void>() {

            private InetAddress address = null;
            private Exception exception = null;

            @Override
            protected Void doInBackground() throws Exception {
                try {
                    InetAddress[] addresses = InetAddress.getAllByName(hostName);

                    Optional<InetAddress> ipv6addr = Arrays.stream(addresses).filter(a -> a instanceof Inet6Address).findFirst();
                    Optional<InetAddress> ipv4addr = Arrays.stream(addresses).filter(a -> a instanceof Inet4Address).findFirst();

                    if (ipv6addr.isPresent() && ipv6addr.get().isReachable(1500)) {
                        logger.info("Using IPv6 address: {}.", ipv6addr.get().toString());
                        address = ipv6addr.get();
                    } else if (ipv4addr.isPresent() && ipv4addr.get().isReachable(1500)) {
                        logger.info("Using IPv4 address: {}.", ipv4addr.get().toString());
                        address = ipv4addr.get();
                    } else {
                        throw new UnknownHostException(String.format("host %s not reachable", hostName));
                    }
                } catch (final UnknownHostException e) {
                    exception = e;
                }
                return null;
            }

            @Override
            protected void done() {
                if (exception != null) {
                    JOptionPane.showMessageDialog(MainWindowLogic.this,
                            String.format("Unknown host: %s\n%s", hostName, exception.getMessage()),
                            "Address resolve error",
                            JOptionPane.ERROR_MESSAGE);
                    setConnectButtonsText("Connect");
                    setConnectButtonsEnabled(true);
                    connectHostnameField.setEnabled(true);
                } else {
                    performControlServerConnection(address);
                    performCameraServerConnection(address);

                    connected = true;

                    setConnectButtonsText("Disconnect");
                    setConnectButtonsEnabled(true);
                    connectHostnameField.setEnabled(false);
                }
            }
        }.execute();
    }

    public void disconnectFromAll() {
        setConnectButtonsEnabled(false);
        setConnectButtonsText("Disconnecting...");

        new SwingWorker<Void, Void>() {

            @Override
            protected Void doInBackground() throws Exception {
                performCameraServerDisconnection();
                performControlServerDisconnection(true);

                return null;
            }

            @Override
            protected void done() {
                connected = false;

                setConnectButtonsText("Connect");
                setConnectButtonsEnabled(true);

                connectHostnameField.setEnabled(true);
            }
        }.execute();
    }

    private void initializeControls() {
        lightGripper.setSelected(false);
        lightHigh.setSelected(false);
        lightLow.setSelected(false);
        lightCamera.setSelected(false);

        speedLimit5.setSelected(true);

        status.motors.setSpeedLimit(5);

        // joints initial speeds
        armGripperSpeedLimiter.setValue(HardcodedConfiguration.JOINT_SPEED_INIT_GRIPPER);
        armElbowSpeedLimiter.setValue(HardcodedConfiguration.JOINT_SPEED_INIT_ELBOW);
        armShoulderSpeedLimiter.setValue(HardcodedConfiguration.JOINT_SPEED_INIT_SHOULDER);
        stateChanged(null); // write initial values from sliders to status
    }

    @Override
    public void actionPerformed(ActionEvent e) {
        final Object obj = e.getSource();

        if ((obj == connectButton) || (obj == mConnConnect)) {
            connectButton.setEnabled(false);
            mConnConnect.setEnabled(false);
            if (!connected) {
                connectToAll();
            } else {
                disconnectFromAll();
            }
            connectButton.setEnabled(true);
            mConnConnect.setEnabled(true);
        } else if (obj == startStopButton) {
            startStopButton.setEnabled(false);
            if (status.isKillSwitchEnable()) {
                performKillSwitchDisable();
            } else {
                performKillSwitchEnable();
            }
            startStopButton.setEnabled(true);
        }

        if (obj == lightLow) {
            if (lightLow.isSelected()) {
                status.lights.setLow(true);
            } else {
                status.lights.setLow(false);
            }
        } else if (obj == lightGripper) {
            if (lightGripper.isSelected()) {
                status.lights.setGripper(true);
            } else {
                status.lights.setGripper(false);
            }
        } else if (obj == lightHigh) {
            if (lightHigh.isSelected()) {
                status.lights.setHigh(true);
            } else {
                status.lights.setHigh(false);
            }
        } else if (obj == armCalibrateButton) {
            status.joints.setBeginCalibration(true);
            armCalibrateButton.setText("Sending request...");
        } else if (obj == speedLimit5) {
            status.motors.setSpeedLimit(5);
        } else if (obj == speedLimit8) {
            status.motors.setSpeedLimit(8);
        } else if (obj == speedLimit11) {
            status.motors.setSpeedLimit(11);
        }

        if (obj == mWinMoveCtrl) {
            if (mConWin.isVisible()) {
                // hide
                mConWin.setVisible(false);
                mWinMoveCtrl.setText("Show move control window");
            } else {
                // show
                mConWin.setVisible(true);
                mWinMoveCtrl.setText("Hide move control window");
            }
        } else if (obj == cameraSelectGripper) {
            cameraScreen.setChosenCameraType(CameraType.GRIPPER);
        } else if (obj == cameraSelectBack) {
            cameraScreen.setChosenCameraType(CameraType.BACK);
        } else if (obj == cameraDisplayHud) {
            if (cameraDisplayHud.isSelected()) {
                cameraScreen.setCameraMode(CameraMode.HUD);
            } else {
                cameraScreen.setCameraMode(CameraMode.RAW);
            }
        }

        if (obj == exitButton) {
            if (connected) {
                disconnectFromAll();
            }
            System.exit(0);
        }
    }

    public void updateIndicators(Status receivedStatus) {
        // battery
        batteryVoltBar.setValue((int) (10 * receivedStatus.battery.getVoltage()));
        batteryCurrBar.setValue((int) (10 * receivedStatus.battery.getCurrent()));

        batteryVoltBar.setString("Voltage: " + receivedStatus.battery.getVoltage() + "V");
        batteryCurrBar.setString("Current: " + receivedStatus.battery.getCurrent() + "A");

        // wifi power
        int wifiPower = (int) receivedStatus.wifi.getStrength();
        wifiPowerBar.setValue(150 + wifiPower);
        wifiPowerBar.setString("Power: " + wifiPower + " dBm");

        // movement indicators
        statSpeedLeft.setValue(receivedStatus.motors.left.getSpeed());
        statSpeedLeft.setString(String.format("Left: %d/%d",
                receivedStatus.motors.left.getSpeed(),
                HardcodedConfiguration.MOTOR_SPEED_MAX));

        statSpeedRight.setValue(receivedStatus.motors.right.getSpeed());
        statSpeedRight.setString(String.format("Right: %d/%d",
                receivedStatus.motors.left.getSpeed(),
                HardcodedConfiguration.MOTOR_SPEED_MAX));

        if (receivedStatus.motors.left.getDirection() == Direction.FORWARD) {
            statDirectionLeft.setText("<html>Left motor: <b>FORWARD");
        } else if (receivedStatus.motors.left.getDirection() == Direction.BACKWARD) {
            statDirectionLeft.setText("<html>Left motor: <b>BACKWARD");
        } else {
            statDirectionLeft.setText("<html>Left motor: <b>STOPPED");
        }

        if (receivedStatus.motors.right.getDirection() == Direction.FORWARD) {
            statDirectionRight.setText("<html>Right motor: <b>FORWARD");
        } else if (receivedStatus.motors.right.getDirection() == Direction.BACKWARD) {
            statDirectionRight.setText("<html>Right motor: <b>BACKWARD");
        } else {
            statDirectionRight.setText("<html>Right motor: <b>STOPPED");
        }

        statArmElbowSpeed.setValue(receivedStatus.joints.elbow.getSpeed());
        statArmShoulderSpeed.setValue(receivedStatus.joints.shoulder.getSpeed());
        statArmGripperSpeed.setValue(receivedStatus.joints.gripper.getSpeed());

        statArmElbowSpeed.setString(String.format("Elbow %s, speed: %d/%d",
                receivedStatus.joints.elbow,
                receivedStatus.joints.elbow.getSpeed(),
                HardcodedConfiguration.JOINT_SPEED_MAX));

        statArmShoulderSpeed.setString(String.format("Shoulder %s, speed: %d/%d",
                receivedStatus.joints.shoulder,
                receivedStatus.joints.shoulder.getSpeed(),
                HardcodedConfiguration.JOINT_SPEED_MAX));

        statArmGripperSpeed.setString(String.format("Gripper %s, speed: %d/%d",
                receivedStatus.joints.gripper,
                receivedStatus.joints.gripper.getSpeed(),
                HardcodedConfiguration.JOINT_SPEED_MAX));

        switch (receivedStatus.joints.getCalStatus()) {
            case NONE:
                armCalibrateButton.setText("Calibrate joints");
                break;
            case IN_PROGRESS:
                armCalibrateButton.setText("Calibrating...");
                break;
            case DONE:
                armCalibrateButton.setText("Calibrate joints [DONE]");
                break;
        }

        // visualizer
        armVis.setUpdateStatus(receivedStatus);
    }

    @Override
    public void stateChanged(ChangeEvent arg0) {
        status.joints.elbow.setSpeedLimit(armElbowSpeedLimiter.getValue());
        status.joints.shoulder.setSpeedLimit(armShoulderSpeedLimiter.getValue());
        status.joints.gripper.setSpeedLimit(armGripperSpeedLimiter.getValue());
    }
}
