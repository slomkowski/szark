package eu.slomkowski.szark.client.gui;

import eu.slomkowski.szark.client.HardcodedConfiguration;
import eu.slomkowski.szark.client.camera.CameraMode;
import eu.slomkowski.szark.client.camera.CameraType;
import eu.slomkowski.szark.client.joystick.InvalidJoystickException;
import eu.slomkowski.szark.client.joystick.JoystickBackend;
import eu.slomkowski.szark.client.status.CalibrationStatus;
import eu.slomkowski.szark.client.status.Direction;
import eu.slomkowski.szark.client.status.Status;
import eu.slomkowski.szark.client.updaters.ControlUpdater;

import javax.swing.*;
import javax.swing.event.ChangeEvent;
import java.awt.event.ActionEvent;

/**
 * This class provides the logic for the main window. The appearance is moved to
 * view class.
 *
 * @author Michał Słomkowski
 */
public class MainWindowLogic extends MainWindowView {

	private final Status status = new Status();

	private final MoveControlWindow mConWin = new MoveControlWindow(status);

	private JoystickBackend joystickBackend = null;

	private boolean connected = false;

	private ControlUpdater controlUpdater;

	public MainWindowLogic() {
		performControlServerDisconnection(false);

		if (HardcodedConfiguration.JOYSTICK_ENABLE) {
			try {
				joystickBackend = new JoystickBackend();

				mWinMoveCtrl.setEnabled(false);
				mWinMoveCtrl.setText(mWinMoveCtrl.getText() + " (Disabled because of joystick)");
			} catch (final InvalidJoystickException e) {
				e.printStackTrace();

				JOptionPane.showMessageDialog(this,
						"Joystick error: " + e.getMessage() + ". Joystick won't be used.",
						"Joystick error",
						JOptionPane.WARNING_MESSAGE);
			}
		}
	}

	public void performCameraServerConnection() {
		cameraSelectHead.setSelected(true);
		cameraDisplayHud.setSelected(true);
		setCameraControlsEnabled(true);

		cameraScreen.enableCameraView(connectHostnameField.getSelectedItem().toString());
	}

	public void performCameraServerDisconnection() {
		setCameraControlsEnabled(false);
		cameraScreen.disableCameraView();
	}

	public void performKillSwitchDisable() {
		performKillSwitchEnable();

		status.setKillswitchEnable(false);
		startStopButton.setIcon(iconStop);
		setControlsEnabled(true);

		if (joystickBackend == null) {
			mConWin.setEnabled(true);
		}
	}

	public void performKillSwitchEnable() {
		status.clean();

		status.setKillswitchEnable(true);
		startStopButton.setIcon(iconStart);

		setControlsEnabled(false); // disabling all controls
		// enabling these, which has to be enabled
		startStopButton.setEnabled(true);

		batteryCurrBar.setEnabled(true);
		batteryVoltBar.setEnabled(true);

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

		updateIndicators(status);
		mConWin.setEnabled(false);
	}

	private void performControlServerConnection() {
		performKillSwitchEnable();

		controlUpdater = new ControlUpdater(this,
				connectHostnameField.getSelectedItem().toString(),
				status,
				joystickBackend);
		controlUpdater.execute();

		batteryCurrBar.setEnabled(true);
		batteryVoltBar.setEnabled(true);
	}

	public void connectToAll() {
		performControlServerConnection();
		performCameraServerConnection();

		connected = true;

		connectButton.setText("Disconnect");
		mConnConnect.setText(connectButton.getText());
		connectHostnameField.setEnabled(false);
	}

	public void disconnectFromAll() {
		performCameraServerDisconnection();
		performControlServerDisconnection(true);

		connected = false;

		connectButton.setText("Connect");
		mConnConnect.setText(connectButton.getText());
		connectHostnameField.setEnabled(true);
	}

	public void performControlServerDisconnection(boolean sendDisablingCommand) {
		performKillSwitchEnable();
		setControlsEnabled(false);

		status.setKillswitchEnable(true);
		if (sendDisablingCommand && controlUpdater != null) {
			controlUpdater.stopTask();
		}
		controlUpdater = null;
	}

	@Override
	public void actionPerformed(ActionEvent e) {
		final Object obj = e.getSource();

		if ((obj == connectButton) || (obj == mConnConnect)) {
			if (!connected) {
				connectToAll();
			} else {
				disconnectFromAll();
			}
		} else if (obj == startStopButton) {
			if (status.isKillswitchEnable()) {
				performKillSwitchDisable();
			} else {
				performKillSwitchEnable();
			}
		} else if (obj == lightLow) {
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
			status.joints.setCalStatus(CalibrationStatus.REQUESTED);
		} else if (obj == cameraSelectHead) {
			cameraScreen.setChosenCameraType(CameraType.HEAD);
		} else if (obj == cameraSelectGripper) {
			cameraScreen.setChosenCameraType(CameraType.GRIPPER);
		} else if (obj == cameraDisplayHud) {
			if (cameraDisplayHud.isSelected()) {
				cameraScreen.setCameraMode(CameraMode.HUD);
			} else {
				cameraScreen.setCameraMode(CameraMode.RAW);
			}
		} else if (obj == speedLimit5) {
			status.motors.setSpeedLimit(5);
		} else if (obj == speedLimit8) {
			status.motors.setSpeedLimit(8);
		} else if (obj == speedLimit12) {
			status.motors.setSpeedLimit(12);
		} else if (obj == mWinMoveCtrl) {
			if (mConWin.isVisible()) {
				// hide
				mConWin.setVisible(false);
				mWinMoveCtrl.setText("Show move control window");
			} else {
				// show
				mConWin.setVisible(true);
				mWinMoveCtrl.setText("Hide move control window");
			}
		} else if (obj == exitButton) {
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
		wifiPowerBar.setValue(100 * receivedStatus.getWirelessPower());

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
