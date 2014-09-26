package eu.slomkowski.szark.client.gui;

import eu.slomkowski.szark.client.HardcodedConfiguration;
import eu.slomkowski.szark.client.gui.ArmVisualizer;

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

	protected JMenuItem mConnConnect = new JMenuItem("CONNECT BUTTON");

	protected JMenuItem mWinMoveCtrl = new JMenuItem("Show move control window");
	protected JMenuItem mWinArmCtrl = new JMenuItem("Show joints control window");

	protected JLabel cameraScreenshot = new JLabel(new ImageIcon(getClass().getResource(HardcodedConfiguration.DEFAULT_LOGO)));

	protected JComboBox<String> connectHostnameField = new JComboBox<String>(HardcodedConfiguration.DEFAULT_HOSTNAMES);
	protected JButton connectButton = new JButton("CONNECT BUTTON");
	protected JButton exitButton = new JButton("Exit");

	protected JButton startStopButton = new JButton();
	protected ImageIcon iconStop = new ImageIcon(getClass().getResource("/img/stop.png"));
	protected ImageIcon iconStart = new ImageIcon(getClass().getResource("/img/start.png"));

	protected JProgressBar batteryVoltBar = new JProgressBar(JProgressBar.HORIZONTAL);
	protected JProgressBar batteryCurrBar = new JProgressBar(JProgressBar.HORIZONTAL);

	protected JProgressBar wifiPowerBar = new JProgressBar(JProgressBar.HORIZONTAL);

	protected JCheckBox lightHigh = new JCheckBox("Head");
	protected JCheckBox lightGripper = new JCheckBox("Gripper");
	protected JCheckBox lightLow = new JCheckBox("Low");
	protected JCheckBox lightCamera = new JCheckBox("Camera");

	protected JRadioButton speedLimit5 = new JRadioButton("5     ");
	protected JRadioButton speedLimit8 = new JRadioButton("8      ");
	protected JRadioButton speedLimit12 = new JRadioButton("12");

	protected ArmVisualizer armVis = new ArmVisualizer();

	protected JProgressBar statSpeedLeft = new JProgressBar(JProgressBar.HORIZONTAL);
	protected JProgressBar statSpeedRight = new JProgressBar(JProgressBar.HORIZONTAL);
	protected JLabel statDirectionLeft = new JLabel();
	protected JLabel statDirectionRight = new JLabel();

	protected JProgressBar statArmGripperSpeed = new JProgressBar(JProgressBar.HORIZONTAL);
	protected JProgressBar statArmShoulderSpeed = new JProgressBar(JProgressBar.HORIZONTAL);
	protected JProgressBar statArmElbowSpeed = new JProgressBar(JProgressBar.HORIZONTAL);

	protected JRadioButton cameraSelectHead = new JRadioButton("Head");
	protected JRadioButton cameraSelectGripper = new JRadioButton("Gripper");

	protected JSlider armShoulderSpeedLimiter = new JSlider(JSlider.HORIZONTAL, 0, 15, 0);
	protected JSlider armElbowSpeedLimiter = new JSlider(JSlider.HORIZONTAL, 0, 15, 0);
	protected JSlider armGripperSpeedLimiter = new JSlider(JSlider.HORIZONTAL, 0, 15, 0);

	protected JButton armCalibrateButton = new JButton("Calibrate joints");

	public MainWindowView() {
		JPanel p; // temporary reference

		setTitle("SZARK - client v. " + HardcodedConfiguration.VERSION);

		setDefaultCloseOperation(DO_NOTHING_ON_CLOSE);
		addWindowListener(new WindowAdapter() {

			@Override
			public void windowClosing(WindowEvent e) {
				exitButton.doClick();
			}
		});

		setLayout(new BorderLayout());

		// MENU
		final JMenuBar menubar = new JMenuBar();
		setJMenuBar(menubar);

		// connection
		final JMenu mConn = new JMenu("Connection");
		mConn.add(mConnConnect);
		menubar.add(mConn);

		mConnConnect.addActionListener(this);

		// windows
		final JMenu mWin = new JMenu("Windows");
		mWin.add(mWinArmCtrl);
		mWin.add(mWinMoveCtrl);
		menubar.add(mWin);

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
		p.setBorder(new TitledBorder("Emergency stop & start button:"));
		p.add(startStopButton);// , BorderLayout.WEST);
		sidePanel.add(p);

		// CAMERA SELECTOR
		p = new JPanel();
		p.setLayout(new FlowLayout());
		p.setBorder(new TitledBorder("Select camera:"));
		p.add(cameraSelectHead);
		p.add(cameraSelectGripper);
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
		armVis.setBorder(new TitledBorder("Arm visualizer:"));
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
		statSpeedLeft.setMaximum(15);
		statSpeedRight.setMaximum(15);
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
		p.setBorder(new TitledBorder("Arm parameters & limiters:"));

		final JPanel agauge = new JPanel(new GridLayout(3, 1, 1, 2));
		agauge.add(statArmShoulderSpeed);
		agauge.add(statArmElbowSpeed);
		agauge.add(statArmGripperSpeed);

		final JPanel aslid = new JPanel(new GridLayout(3, 1, 1, 2));
		aslid.add(armShoulderSpeedLimiter);
		aslid.add(armElbowSpeedLimiter);
		aslid.add(armGripperSpeedLimiter);

		statArmElbowSpeed.setStringPainted(true);
		statArmGripperSpeed.setStringPainted(true);
		statArmShoulderSpeed.setStringPainted(true);

		statArmElbowSpeed.setMaximum(15);
		statArmGripperSpeed.setMaximum(15);
		statArmShoulderSpeed.setMaximum(15);

		p.add(agauge);
		p.add(aslid, BorderLayout.EAST);

		lowerPanel.add(p);

		// CAMERA VIEW
		p = new JPanel();
		p.setBorder(new TitledBorder("Camera view:"));
		p.add(cameraScreenshot);// , BorderLayout.CENTER);
		cameraScreenshot.setPreferredSize(new Dimension(640, 480));

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

	public JLabel getCameraScreenshotJLabel() {
		return cameraScreenshot;
	}

	@Override
	public abstract void stateChanged(ChangeEvent arg0);
}
