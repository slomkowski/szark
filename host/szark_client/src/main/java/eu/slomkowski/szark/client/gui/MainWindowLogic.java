package eu.slomkowski.szark.client.gui;

import eu.slomkowski.szark.client.HardcodedConfiguration;
import eu.slomkowski.szark.client.camera.CameraMode;
import eu.slomkowski.szark.client.camera.CameraType;
import eu.slomkowski.szark.client.joystick.InvalidJoystickException;
import eu.slomkowski.szark.client.joystick.JoystickBackend;
import eu.slomkowski.szark.client.status.CalibrationStatus;
import eu.slomkowski.szark.client.status.Direction;
import eu.slomkowski.szark.client.status.Status;
import eu.slomkowski.szark.client.updaters.JoystickDataUpdater;
import eu.slomkowski.szark.client.updaters.SzarkDataUpdater;

import javax.swing.*;
import javax.swing.event.ChangeEvent;
import java.awt.event.ActionEvent;
import java.util.Timer;
import java.util.TimerTask;

/**
 * This class provides the logic for the main window. The appearance is moved to
 * view class.
 *
 * @author Michał Słomkowski
 */
public class MainWindowLogic extends MainWindowView {

	private final Status status = new Status();

	// windows are initialized now, buttons only make them visible
	private final MoveControlWindow mConWin = new MoveControlWindow(status);

	private Timer szarkUpdaterTimer; // TODO zamienić na SwingWorker

	private JoystickBackend jBackend;

	private boolean enableJoystick = HardcodedConfiguration.ENABLE_JOYSTICK;

	private boolean connected = false;

	private SzarkUpdater szarkUpdater;

	public MainWindowLogic() {
		thingsWhenDisconnect(false);
		// joystick setup
		if (enableJoystick) {
			try {
				jBackend = new JoystickBackend();

				mWinMoveCtrl.setEnabled(false);
				mWinMoveCtrl.setText(mWinMoveCtrl.getText() + " (Disabled because of joystick)");
			} catch (final InvalidJoystickException e) {
				JOptionPane.showMessageDialog(this,
						"Joystick error: " + e.getMessage() + ". Joystick won't be used.",
						"Joystick error",
						JOptionPane.WARNING_MESSAGE);
				e.printStackTrace();
				enableJoystick = false;
			}
		}
	}

	public void thingsWhenEnabling() {
		thingsWhenDisabling();

		status.setKillswitchEnable(false);
		startStopButton.setIcon(iconStop);
		setControlsEnabled(true);

		if (!enableJoystick) {
			mConWin.setEnabled(true);
		}
	}

	public void thingsWhenDisabling() {
		status.clean();

		status.setKillswitchEnable(true);
		startStopButton.setIcon(iconStart);

		setControlsEnabled(false); // disabling all controls
		// enabling these, which has to be enabled
		startStopButton.setEnabled(true);

		cameraSelectGripper.setEnabled(true);
		cameraSelectHead.setEnabled(true);
		cameraDisplayHud.setEnabled(true);

		batteryCurrBar.setEnabled(true);
		batteryVoltBar.setEnabled(true);

		lightGripper.setSelected(false);
		lightHigh.setSelected(false);
		lightLow.setSelected(false);
		lightCamera.setSelected(false);

		speedLimit5.setSelected(true);
		status.motors.setSpeedLimit(5);

		// joints initial speeds
		armGripperSpeedLimiter.setValue(HardcodedConfiguration.GRIPPER_SPPED);
		armElbowSpeedLimiter.setValue(HardcodedConfiguration.ELBOW_SPEED);
		armShoulderSpeedLimiter.setValue(HardcodedConfiguration.SHOULDER_SPEED);
		stateChanged(null); // write initial values from sliders to status

		updateIndicators(status);
		mConWin.setEnabled(false);
	}

	private void thingsWhenConnect() {
		thingsWhenDisabling();

		szarkUpdaterTimer = new Timer(true);

		szarkUpdater = new SzarkUpdater(this, connectHostnameField.getSelectedItem().toString());
		szarkUpdaterTimer.schedule(szarkUpdater, 0, HardcodedConfiguration.SZARK_REFRESH_INTERVAL);

		cameraScreen.enableCameraView(connectHostnameField.getSelectedItem().toString());

		connected = true;

		connectButton.setText("Disconnect");
		mConnConnect.setText("Disconnect");

		connectHostnameField.setEnabled(false);

		cameraSelectHead.setSelected(true);
		cameraDisplayHud.setSelected(true);
		batteryCurrBar.setEnabled(true);
		batteryVoltBar.setEnabled(true);
	}

	public void thingsWhenDisconnect(boolean sendDisablingCommand) {
		if (szarkUpdaterTimer != null) {
			szarkUpdaterTimer.cancel();
		}

		// ensure sending any recent changes
		status.setKillswitchEnable(true);

		if (sendDisablingCommand && szarkUpdater != null) {
			szarkUpdater.sendChanges();
		}

		thingsWhenDisabling();

		connected = false;
		connectButton.setText("Connect");
		mConnConnect.setText("Connect");
		connectHostnameField.setEnabled(true);
		setControlsEnabled(false);
		cameraScreen.disableCameraView();
	}

	@Override
	public void actionPerformed(ActionEvent e) {
		final Object obj = e.getSource();

		if ((obj == connectButton) || (obj == mConnConnect)) {
			if (!connected) {
				thingsWhenConnect();
			} else {
				thingsWhenDisconnect(true);
			}
		} else if (obj == startStopButton) {
			if (status.isKillswitchEnable()) {
				thingsWhenEnabling();
			} else {
				thingsWhenDisabling();
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
				thingsWhenDisconnect(true);
			}
			System.exit(0);
		}
	}

	// timer task used to refresh battery, wifi status etc.
	private void updateIndicators(Status receivedStatus) {
		// battery
		batteryVoltBar.setValue((int) (10 * receivedStatus.battery.getVoltage()));
		batteryCurrBar.setValue((int) (10 * receivedStatus.battery.getCurrent()));

		batteryVoltBar.setString("Voltage: " + receivedStatus.battery.getVoltage() + "V");
		batteryCurrBar.setString("Current: " + receivedStatus.battery.getCurrent() + "A");

		// wifi power
		wifiPowerBar.setValue(100 * receivedStatus.getWirelessPower());

		// movement indicators
		statSpeedLeft.setValue(receivedStatus.motors.left.getSpeed());
		statSpeedLeft.setString("Left speed: " + receivedStatus.motors.left.getSpeed() + "/15");

		statSpeedRight.setValue(receivedStatus.motors.right.getSpeed());
		statSpeedRight.setString("Right speed: " + receivedStatus.motors.right.getSpeed() + "/15");

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

		statArmElbowSpeed.setString("Elbow " + receivedStatus.joints.elbow + ", speed: " + receivedStatus.joints.elbow.getSpeed() + "/15");
		statArmShoulderSpeed.setString("Shoulder " + receivedStatus.joints.shoulder + ", speed: "
				+ receivedStatus.joints.shoulder.getSpeed() + "/15");
		statArmGripperSpeed.setString("Gripper " + receivedStatus.joints.gripper + ", speed: " + receivedStatus.joints.gripper.getSpeed()
				+ "/15");

		// visualizer
		armVis.setUpdateStatus(receivedStatus);
	}

	@Override
	public void stateChanged(ChangeEvent arg0) {
		status.joints.elbow.setSpeedLimit(armElbowSpeedLimiter.getValue());
		status.joints.shoulder.setSpeedLimit(armShoulderSpeedLimiter.getValue());
		status.joints.gripper.setSpeedLimit(armGripperSpeedLimiter.getValue());
	}

	private class SzarkUpdater extends TimerTask {

		private final SzarkDataUpdater szdUpdater;
		private final MainWindowLogic mainWin;
		private JoystickDataUpdater jUpdater;

		public SzarkUpdater(MainWindowLogic win, String hostname) {
			mainWin = win;
			if (enableJoystick) {
				jUpdater = new JoystickDataUpdater(status, jBackend);
			}
			szdUpdater = new SzarkDataUpdater(hostname, HardcodedConfiguration.SZARK_SERVER_PORT, status);
			armVis.setUpdateStatus(status);
		}

		@Override
		public void run() {
			sendChanges();
		}

		public void sendChanges() {
			if (enableJoystick) {
				jUpdater.update();
			}
			try {
				updateIndicators(szdUpdater.update());
			} catch (final SzarkDataUpdater.ConnectionErrorException e) {
				mainWin.thingsWhenDisconnect(false);
				JOptionPane.showMessageDialog(mainWin,
						"Network error: " + e.getMessage(),
						"Network error",
						JOptionPane.ERROR_MESSAGE);
			} catch (final SzarkDataUpdater.HardwareStoppedException e) {
				mainWin.thingsWhenDisabling();
				JOptionPane.showMessageDialog(mainWin,
						"Kill switch: " + e.getMessage(),
						"Kill switch activated",
						JOptionPane.WARNING_MESSAGE);
			}
		}
	}
}
