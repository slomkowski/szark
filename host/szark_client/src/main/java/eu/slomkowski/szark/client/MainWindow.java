package eu.slomkowski.szark.client;

import java.awt.event.ActionEvent;
import java.util.Timer;
import java.util.TimerTask;

import javax.swing.ImageIcon;
import javax.swing.JOptionPane;
import javax.swing.event.ChangeEvent;

/**
 * This class provides the logic for the main window. The appearance is moved to
 * MainWindowDummy class.
 * 
 * @author Michał Słomkowski
 * 
 */
public class MainWindow extends MainWindowDummy {

	private static final long serialVersionUID = -4612463062350720332L;

	private Timer szarkUpdaterTimer;
	private Timer cameraImageUpdaterTimer;

	private JoypadBackend jBackend;
	private boolean enableJoystick = Hardcoded.ENABLE_JOYSTICK;
	private boolean connected = false;

	private final SzarkStatus status = new SzarkStatus();// SzarkClient.status;

	// windows are initialized now, buttons only make them visible
	private final MoveControlWindow mConWin = new MoveControlWindow(status);
	// TODO armControlWindow można by zrobić - czyli kontrola ramieniem, gdy
	// joysticka nie ma

	private SzarkUpdater szarkUpdater;

	public MainWindow() {
		thingsWhenDisconnect();
		// joystick setup
		if (enableJoystick) {
			try {
				jBackend = new JoypadBackend();

				mWinMoveCtrl.setEnabled(false);
				mWinMoveCtrl.setText(mWinMoveCtrl.getText() + " (Disabled because of joystick)");
			} catch (final JoypadBackend.InvalidJoypadException e) {
				JOptionPane.showMessageDialog(this, "Joystick error: " + e.getMessage());
				e.printStackTrace();
				enableJoystick = false;
			}
		}
	}

	private class SzarkUpdater extends TimerTask {

		private JoypadDataUpdater jUpdater;
		private final SzarkDataUpdater szdUpdater;
		private final MainWindow mainWin;

		public SzarkUpdater(MainWindow win, String hostname) {
			mainWin = win;
			if (enableJoystick) {
				jUpdater = new JoypadDataUpdater(status, jBackend);
			}
			szdUpdater = new SzarkDataUpdater(hostname, Hardcoded.SZARK_SERVER_PORT, status);
			armVis.setUpdateStatus(status);
		}

		@Override
		public void run() {
			if (enableJoystick) {
				jUpdater.update();
			}
			try {
				szdUpdater.update();
			} catch (final SzarkDataUpdater.ConnectionErrorException e) {
				mainWin.thingsWhenDisconnect();
				JOptionPane.showMessageDialog(mainWin, "Network error: " + e.getMessage());
			} catch (final SzarkDataUpdater.HardwareStoppedException e) {
				mainWin.thingsWhenDisabling();
				JOptionPane.showMessageDialog(mainWin, "Hardware emergency stop occured!");
			}
			updateIndicators();
		}
	}

	public void thingsWhenEnabling() {
		thingsWhenDisabling();

		status.setEmergencyStopped(false);
		startStopButton.setIcon(iconStop);
		setControlsEnabled(true);

		if (!enableJoystick) {
			mConWin.setEnabled(true);
		}
	}

	public void thingsWhenDisabling() {
		status.clean();

		status.setEmergencyStopped(true);
		startStopButton.setIcon(iconStart);

		setControlsEnabled(false); // disabling all controls
		// enabling these, which has to be enabled
		startStopButton.setEnabled(true);
		mServ.setEnabled(true);

		cameraSelectGripper.setEnabled(true);
		cameraSelectHead.setEnabled(true);

		batteryCurrBar.setEnabled(true);
		batteryVoltBar.setEnabled(true);

		lightGripper.setSelected(false);
		lightHigh.setSelected(false);
		lightLow.setSelected(false);
		lightCamera.setSelected(false);

		speedLimit5.setSelected(true);
		status.motors.setSpeedLimit(5);

		// arm initial speeds
		armGripperSpeedLimiter.setValue(Hardcoded.GRIPPER_SPPED);
		armElbowSpeedLimiter.setValue(Hardcoded.ELBOW_SPEED);
		armShoulderSpeedLimiter.setValue(Hardcoded.SHOULDER_SPEED);
		armWristSpeedLimiter.setValue(Hardcoded.WRIST_SPEED);
		stateChanged(null); // write initial values from sliders to status

		updateIndicators();
		mConWin.setEnabled(false);
	}

	private CameraImageUpdater cameraUpdater;

	private void thingsWhenConnect() {
		thingsWhenDisabling();

		szarkUpdaterTimer = new Timer(true);
		cameraImageUpdaterTimer = new Timer(true);

		szarkUpdater = new SzarkUpdater(this, connectHostnameField.getSelectedItem().toString());
		szarkUpdaterTimer.schedule(szarkUpdater, 0, Hardcoded.SZARK_REFRESH_INTERVAL);

		cameraUpdater = new CameraImageUpdater(cameraScreenshot, connectHostnameField.getSelectedItem().toString());
		cameraImageUpdaterTimer.schedule(cameraUpdater, 0, Hardcoded.CAMERA_REFRESH_INTERVAL);

		connected = true;

		connectButton.setText("Disconnect");
		mConnConnect.setText("Disconnect");

		connectHostnameField.setEnabled(false);

		cameraSelectHead.setSelected(true);
		batteryCurrBar.setEnabled(true);
		batteryVoltBar.setEnabled(true);
	}

	public void thingsWhenDisconnect() {
		// ensure sending any recent changes
		status.setEmergencyStopped(true);
		if (szarkUpdater != null) {
			szarkUpdater.run();
		}

		thingsWhenDisabling();

		if (szarkUpdaterTimer != null) {
			szarkUpdaterTimer.cancel();
		}
		if (cameraImageUpdaterTimer != null) {
			cameraImageUpdaterTimer.cancel();
		}

		connected = false;
		connectButton.setText("Connect");
		mConnConnect.setText("Connect");
		connectHostnameField.setEnabled(true);
		setControlsEnabled(false);
		cameraScreenshot.setIcon(new ImageIcon(getClass().getResource(Hardcoded.DEFAULT_LOGO)));
	}

	@Override
	public void actionPerformed(ActionEvent e) {
		final Object obj = e.getSource();

		if ((obj == connectButton) || (obj == mConnConnect)) {
			if (connected == false) {
				thingsWhenConnect();
			} else {
				thingsWhenDisconnect();
			}
		} else if (obj == startStopButton) {
			if (status.isEmergencyStopped()) {
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
		} else if (obj == lightCamera) {
			if (lightCamera.isSelected()) {
				status.lights.setCamera(true);
			} else {
				status.lights.setCamera(false);
			}
		} else if (obj == armCalibrateButton) {
			status.arm.setCalStatus(SzarkStatus.CalStatus.REQUESTED);
		} else if (obj == cameraSelectHead) {
			cameraUpdater.setChoosenCamera(CameraImageUpdater.Camera.HEAD);
		} else if (obj == cameraSelectGripper) {
			cameraUpdater.setChoosenCamera(CameraImageUpdater.Camera.GRIPPER);
		} else if (obj == speedLimit5) {
			status.motors.setSpeedLimit(5);
		} else if (obj == speedLimit8) {
			status.motors.setSpeedLimit(8);
		} else if (obj == speedLimit12) {
			status.motors.setSpeedLimit(12);
		} else if (obj == mServShutdown) {
			status.server.shutdown();
			thingsWhenDisconnect();
		} else if (obj == mServReboot) {
			status.server.reboot();
			thingsWhenDisconnect();
		} else if (obj == mServExit) {
			status.server.exit();
			thingsWhenDisconnect();
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
			thingsWhenDisconnect();
			System.exit(0);
		}
	}

	// timer task used to refresh battery, wifi status etc.
	private void updateIndicators() {
		// battery
		batteryVoltBar.setValue((int) (10 * status.battery.getVoltage()));
		batteryCurrBar.setValue((int) (10 * status.battery.getCurrent()));

		batteryVoltBar.setString("Voltage: " + status.battery.getVoltage() + "V");
		batteryCurrBar.setString("Current: " + status.battery.getCurrent() + "A");

		// wifi power
		wifiPowerBar.setValue(100 * status.getWirelessPower());

		// movement indicators
		statSpeedLeft.setValue(status.motors.left.getSpeed());
		statSpeedLeft.setString("Left speed: " + status.motors.left.getSpeed() + "/15");

		statSpeedRight.setValue(status.motors.right.getSpeed());
		statSpeedRight.setString("Right speed: " + status.motors.right.getSpeed() + "/15");

		if (status.motors.left.getDirection() == SzarkStatus.Direction.FORWARD) {
			statDirectionLeft.setText("<html>Left motor: <b>FORWARD");
		} else if (status.motors.left.getDirection() == SzarkStatus.Direction.BACKWARD) {
			statDirectionLeft.setText("<html>Left motor: <b>BACKWARD");
		} else {
			statDirectionLeft.setText("<html>Left motor: <b>STOPPED");
		}

		if (status.motors.right.getDirection() == SzarkStatus.Direction.FORWARD) {
			statDirectionRight.setText("<html>Right motor: <b>FORWARD");
		} else if (status.motors.right.getDirection() == SzarkStatus.Direction.BACKWARD) {
			statDirectionRight.setText("<html>Right motor: <b>BACKWARD");
		} else {
			statDirectionRight.setText("<html>Right motor: <b>STOPPED");
		}

		// arm indicators

		statArmElbowSpeed.setValue(status.arm.elbow.getSpeed());
		statArmWristSpeed.setValue(status.arm.wrist.getSpeed());
		statArmShoulderSpeed.setValue(status.arm.shoulder.getSpeed());
		statArmGripperSpeed.setValue(status.arm.gripper.getSpeed());

		statArmElbowSpeed.setString("Elbow " + status.arm.elbow + ", speed: " + status.arm.elbow.getSpeed() + "/15");
		statArmShoulderSpeed.setString("Shoulder " + status.arm.shoulder + ", speed: "
				+ status.arm.shoulder.getSpeed() + "/15");
		statArmWristSpeed.setString("Wrist " + status.arm.wrist + ", speed: " + status.arm.wrist.getSpeed() + "/15");
		statArmGripperSpeed.setString("Gripper " + status.arm.wrist + ", speed: " + status.arm.gripper.getSpeed()
				+ "/15");

		// visualizer
		armVis.setUpdateStatus(status);
	}

	@Override
	public void stateChanged(ChangeEvent arg0) {
		status.arm.elbow.setSpeedLimit(armElbowSpeedLimiter.getValue());
		status.arm.shoulder.setSpeedLimit(armShoulderSpeedLimiter.getValue());
		status.arm.wrist.setSpeedLimit(armWristSpeedLimiter.getValue());
		status.arm.gripper.setSpeedLimit(armGripperSpeedLimiter.getValue());
	}
}
