package eu.slomkowski.szark.client.gui;

import eu.slomkowski.szark.client.HardcodedConfiguration;
import eu.slomkowski.szark.client.camera.CameraImageUpdater;

import javax.swing.*;
import javax.swing.border.TitledBorder;
import javax.swing.event.ChangeEvent;
import javax.swing.event.ChangeListener;
import java.awt.*;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.awt.event.WindowAdapter;
import java.awt.event.WindowEvent;

/**
 * The main window class. This class holds only the appearance (button, fields
 * etc), all the logic is held in the MainWindow class.
 *
 * @author Michał Słomkowski
 */
public abstract class MainWindowView extends JFrame implements ActionListener, ChangeListener {

	protected final JMenuItem mConnConnect = new JMenuItem("CONNECT BUTTON");

	protected final JMenuItem mWinMoveCtrl = new JMenuItem("Show move control window");
	private final JMenuItem mWinArmCtrl = new JMenuItem("Show joints control window");

	protected final CameraImageUpdater cameraScreen = new CameraImageUpdater();

	protected final JComboBox<String> connectHostnameField = new JComboBox<>(HardcodedConfiguration.DEFAULT_HOST_NAMES);
	protected final JButton connectButton = new JButton("CONNECT BUTTON");
	protected final JButton exitButton = new JButton("Exit");

	protected final JButton startStopButton = new JButton();
	protected ImageIcon iconStop = new ImageIcon(getClass().getResource("/img/stop.png"));
	protected ImageIcon iconStart = new ImageIcon(getClass().getResource("/img/start.png"));

	protected final JProgressBar batteryVoltBar = new JProgressBar(JProgressBar.HORIZONTAL);
	protected final JProgressBar batteryCurrBar = new JProgressBar(JProgressBar.HORIZONTAL);

	protected final JProgressBar wifiPowerBar = new JProgressBar(JProgressBar.HORIZONTAL);

	protected final JCheckBox lightHigh = new JCheckBox("Head");
	protected final JCheckBox lightGripper = new JCheckBox("Gripper");
	protected final JCheckBox lightLow = new JCheckBox("Low");
	protected final JCheckBox lightCamera = new JCheckBox("Camera");

	protected final JRadioButton speedLimit5 = new JRadioButton("5     ");
	protected final JRadioButton speedLimit8 = new JRadioButton("8      ");
	protected final JRadioButton speedLimit12 = new JRadioButton("12");

	protected final ArmVisualizer armVis = new ArmVisualizer();

	protected final JProgressBar statSpeedLeft = new JProgressBar(JProgressBar.HORIZONTAL);
	protected final JProgressBar statSpeedRight = new JProgressBar(JProgressBar.HORIZONTAL);
	protected final JLabel statDirectionLeft = new JLabel();
	protected final JLabel statDirectionRight = new JLabel();

	protected final JProgressBar statArmGripperSpeed = new JProgressBar(JProgressBar.HORIZONTAL);
	protected final JProgressBar statArmShoulderSpeed = new JProgressBar(JProgressBar.HORIZONTAL);
	protected final JProgressBar statArmElbowSpeed = new JProgressBar(JProgressBar.HORIZONTAL);

	protected final JRadioButton cameraSelectHead = new JRadioButton("Head");
	protected final JRadioButton cameraSelectGripper = new JRadioButton("Gripper");

	protected final JCheckBox cameraDisplayHud = new JCheckBox("Display HUD");

	protected final JSlider armShoulderSpeedLimiter = new JSlider(JSlider.HORIZONTAL, 0, HardcodedConfiguration.JOINT_SPEED_MAX, 0);
	protected final JSlider armElbowSpeedLimiter = new JSlider(JSlider.HORIZONTAL, 0, HardcodedConfiguration.JOINT_SPEED_MAX, 0);
	protected final JSlider armGripperSpeedLimiter = new JSlider(JSlider.HORIZONTAL, 0, HardcodedConfiguration.JOINT_SPEED_MAX, 0);

	protected final JButton armCalibrateButton = new JButton("Calibrate joints");

	public MainWindowView() {
		JPanel p; // temporary reference

		setTitle(String.format("SZARK client %s - Michał Słomkowski",
				HardcodedConfiguration.PROGRAM_VERSION));

		setDefaultCloseOperation(DO_NOTHING_ON_CLOSE);
		addWindowListener(new WindowAdapter() {

			@Override
			public void windowClosing(WindowEvent e) {
				exitButton.doClick();
			}
		});

		setLayout(new BorderLayout());

		// MENU
		final JMenuBar menuBar = new JMenuBar();
		setJMenuBar(menuBar);

		// connection
		final JMenu mConn = new JMenu("Connection");
		mConn.add(mConnConnect);
		menuBar.add(mConn);

		mConnConnect.addActionListener(this);

		// windows
		final JMenu mWin = new JMenu("Windows");
		mWin.add(mWinArmCtrl);
		mWin.add(mWinMoveCtrl);
		menuBar.add(mWin);

		mWinArmCtrl.addActionListener(this);
		mWinMoveCtrl.addActionListener(this);

		final JPanel sidePanel = new JPanel();
		sidePanel.setLayout(new BoxLayout(sidePanel, BoxLayout.PAGE_AXIS));

		p = new JPanel(new FlowLayout());
		p.setBorder(new TitledBorder("Connection parameters:"));
		p.add(new JLabel("Hostname:"), FlowLayout.LEFT);
		p.add(connectHostnameField, FlowLayout.CENTER);
		p.add(exitButton, FlowLayout.RIGHT);
		p.add(connectButton, FlowLayout.RIGHT);
		sidePanel.add(p);
		connectHostnameField.setEditable(true);

		// BATTERY
		batteryVoltBar.setStringPainted(true);
		batteryCurrBar.setStringPainted(true);
		batteryVoltBar.setMaximum(150);
		batteryCurrBar.setMaximum(150);

		p = new JPanel();
		p.setLayout(new BoxLayout(p, BoxLayout.Y_AXIS));
		p.setBorder(new TitledBorder("Battery:"));
		p.add(batteryVoltBar);
		p.add(new JLabel(" "));
		p.add(batteryCurrBar);
		sidePanel.add(p);

		p = new JPanel();
		p.setLayout(new BoxLayout(p, BoxLayout.Y_AXIS));
		p.setBorder(new TitledBorder("Wi-Fi signal strength:"));
		p.add(wifiPowerBar);
		wifiPowerBar.setStringPainted(true);
		sidePanel.add(p);

		// EMERGENCY BUTTON
		p = new JPanel(new BorderLayout());
		p.setBorder(new TitledBorder("Kill switch button:"));
		p.add(startStopButton);// , BorderLayout.WEST);
		sidePanel.add(p);

		// CAMERA SELECTOR
		p = new JPanel();
		p.setLayout(new FlowLayout());
		p.setBorder(new TitledBorder("Cameras:"));
		p.add(cameraSelectHead);
		p.add(cameraSelectGripper);
		p.add(cameraDisplayHud);
		sidePanel.add(p);
		final ButtonGroup cameraBg = new ButtonGroup();
		cameraBg.add(cameraSelectGripper);
		cameraBg.add(cameraSelectHead);

		// LIGHTS
		p = new JPanel(new FlowLayout());
		p.setBorder(new TitledBorder("Lights:"));
		p.add(lightHigh);
		p.add(lightLow);
		p.add(lightGripper);
		p.add(lightCamera);
		sidePanel.add(p);

		// ARM VISUALIZER
		// p = new JPanel();
		armVis.setBorder(new TitledBorder("Joints visualizer:"));
		// p.add(armVis);
		sidePanel.add(armVis);

		final JPanel lowerPanel = new JPanel();
		lowerPanel.setLayout(new BoxLayout(lowerPanel, BoxLayout.LINE_AXIS));

		// DIAGNOSTICS MOTORS
		statSpeedLeft.setStringPainted(true);
		statSpeedRight.setStringPainted(true);

		final JPanel mot = new JPanel(new BorderLayout());
		p = new JPanel();
		p.setLayout(new BoxLayout(p, BoxLayout.Y_AXIS));
		p.setBorder(new TitledBorder("Motor driver parameters:"));
		p.add(statDirectionLeft);
		p.add(statSpeedLeft);
		p.add(statDirectionRight);
		p.add(statSpeedRight);
		statSpeedLeft.setMaximum(HardcodedConfiguration.MOTOR_SPEED_MAX);
		statSpeedRight.setMaximum(HardcodedConfiguration.MOTOR_SPEED_MAX);
		mot.add(p, BorderLayout.NORTH);

		p = new JPanel();
		p.setLayout(new BoxLayout(p, BoxLayout.Y_AXIS));
		p.setBorder(new TitledBorder("Calibrate joints:"));
		mot.add(armCalibrateButton);

		// MOTOR SPEED LIMIT RADIOS
		p = new JPanel(new FlowLayout());
		p.setBorder(new TitledBorder("Motors speed limiter:"));
		p.add(speedLimit5);
		p.add(speedLimit8);
		p.add(speedLimit12);
		mot.add(p, BorderLayout.SOUTH);
		// group radio buttons
		final ButtonGroup speedBg = new ButtonGroup();
		speedBg.add(speedLimit5);
		speedBg.add(speedLimit8);
		speedBg.add(speedLimit12);

		lowerPanel.add(mot);

		// ARM SPEED
		p = new JPanel(new BorderLayout());
		p.setBorder(new TitledBorder("Joints parameters & limiters:"));

		final JPanel jointGaugesPanel = new JPanel(new GridLayout(3, 1, 1, 2));
		jointGaugesPanel.add(statArmShoulderSpeed);
		jointGaugesPanel.add(statArmElbowSpeed);
		jointGaugesPanel.add(statArmGripperSpeed);

		final JPanel jointSlidersPanel = new JPanel(new GridLayout(3, 1, 1, 2));
		jointSlidersPanel.add(armShoulderSpeedLimiter);
		jointSlidersPanel.add(armElbowSpeedLimiter);
		jointSlidersPanel.add(armGripperSpeedLimiter);

		statArmElbowSpeed.setStringPainted(true);
		statArmGripperSpeed.setStringPainted(true);
		statArmShoulderSpeed.setStringPainted(true);

		statArmElbowSpeed.setMaximum(HardcodedConfiguration.JOINT_SPEED_MAX);
		statArmGripperSpeed.setMaximum(HardcodedConfiguration.JOINT_SPEED_MAX);
		statArmShoulderSpeed.setMaximum(HardcodedConfiguration.JOINT_SPEED_MAX);

		p.add(jointGaugesPanel);
		p.add(jointSlidersPanel, BorderLayout.EAST);

		lowerPanel.add(p);

		// CAMERA VIEW
		p = new JPanel();
		p.setBorder(new TitledBorder("Camera view:"));
		p.add(cameraScreen);// , BorderLayout.CENTER);
		cameraScreen.setPreferredSize(new Dimension(640, 480));

		final JPanel pCamAndLower = new JPanel();
		pCamAndLower.setLayout(new BoxLayout(pCamAndLower, BoxLayout.Y_AXIS));
		pCamAndLower.add(p); // camera view
		pCamAndLower.add(lowerPanel);

		add(pCamAndLower, BorderLayout.CENTER);
		add(sidePanel, BorderLayout.EAST);

		// listeners
		connectButton.addActionListener(this);
		startStopButton.addActionListener(this);
		exitButton.addActionListener(this);

		lightGripper.addActionListener(this);
		lightLow.addActionListener(this);
		lightHigh.addActionListener(this);
		lightCamera.addActionListener(this);

		speedLimit8.addActionListener(this);
		speedLimit12.addActionListener(this);
		speedLimit5.addActionListener(this);

		cameraSelectGripper.addActionListener(this);
		cameraSelectHead.addActionListener(this);
		cameraDisplayHud.addActionListener(this);

		armGripperSpeedLimiter.addChangeListener(this);
		armShoulderSpeedLimiter.addChangeListener(this);
		armElbowSpeedLimiter.addChangeListener(this);

		armCalibrateButton.addActionListener(this);

		setVisible(true);

		pack();
	}

	protected void setControlsEnabled(boolean enable) {
		// battery
		batteryVoltBar.setEnabled(enable);
		batteryCurrBar.setEnabled(enable);
		// wifi
		wifiPowerBar.setEnabled(enable);
		// software kill switch button
		startStopButton.setEnabled(enable);
		// light
		lightGripper.setEnabled(enable);
		lightHigh.setEnabled(enable);
		lightLow.setEnabled(enable);
		lightCamera.setEnabled(enable);
		// visualizer
		armVis.setEnabled(enable);
		// stat motors
		statSpeedLeft.setEnabled(enable);
		statSpeedRight.setEnabled(enable);
		statDirectionLeft.setEnabled(enable);
		statDirectionRight.setEnabled(enable);
		// speed limiter radios
		speedLimit5.setEnabled(enable);
		speedLimit8.setEnabled(enable);
		speedLimit12.setEnabled(enable);

		// camera selector
		cameraSelectGripper.setEnabled(enable);
		cameraSelectHead.setEnabled(enable);
		cameraDisplayHud.setEnabled(enable);

		// joints
		statArmElbowSpeed.setEnabled(enable);
		statArmGripperSpeed.setEnabled(enable);
		statArmShoulderSpeed.setEnabled(enable);

		armShoulderSpeedLimiter.setEnabled(enable);
		armElbowSpeedLimiter.setEnabled(enable);
		armGripperSpeedLimiter.setEnabled(enable);
	}

	@Override
	public abstract void actionPerformed(ActionEvent e);

	@Override
	public abstract void stateChanged(ChangeEvent arg0);
}
