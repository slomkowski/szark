package eu.slomkowski.szark.client.updaters;

import com.google.gson.Gson;
import com.google.gson.GsonBuilder;
import eu.slomkowski.szark.client.HardcodedConfiguration;
import eu.slomkowski.szark.client.gui.MainWindowLogic;
import eu.slomkowski.szark.client.joystick.JoystickBackend;
import eu.slomkowski.szark.client.joystick.JoystickDataUpdater;
import eu.slomkowski.szark.client.status.CalibrationStatus;
import eu.slomkowski.szark.client.status.KillSwitchStatus;
import eu.slomkowski.szark.client.status.Status;

import javax.swing.*;
import java.io.IOException;
import java.net.DatagramPacket;
import java.net.DatagramSocket;
import java.net.InetSocketAddress;
import java.net.SocketTimeoutException;
import java.util.List;
import java.util.concurrent.atomic.AtomicBoolean;

public class ControlUpdater extends SwingWorker<Void, Status> {
	private final Gson gson;
	private final Status status;
	private final AtomicBoolean needToStop = new AtomicBoolean(false);
	private final MainWindowLogic mainWindow;
	private JoystickDataUpdater joystickDataUpdater = null;
	private InetSocketAddress address;
	private DatagramSocket socket;
	private byte[] inputBuffer = new byte[1024];

	public ControlUpdater(MainWindowLogic mainWindow,
						  String hostname,
						  Status status,
						  JoystickBackend joystickBackend) {
		this.mainWindow = mainWindow;
		this.status = status;

		this.gson = new GsonBuilder().excludeFieldsWithoutExposeAnnotation().create();

		if (joystickBackend != null) {
			this.joystickDataUpdater = new JoystickDataUpdater(status, joystickBackend);
		}

		try {
			address = new InetSocketAddress(hostname, HardcodedConfiguration.CONTROL_SERVER_PORT);
			//TODO sprawdzanie, czy resolved
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
			status.incrementSerial();
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

					if (joystickDataUpdater != null) {
						joystickDataUpdater.update();
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
				SwingUtilities.invokeAndWait(new Runnable() {
					@Override
					public void run() {
						mainWindow.performKillSwitchEnable();

						JOptionPane.showMessageDialog(mainWindow,
								"Kill switch: " + e.getMessage(),
								"Kill switch activated",
								JOptionPane.WARNING_MESSAGE);
					}
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
	}

	public class HardwareStoppedException extends Exception {
		public HardwareStoppedException(String msg) {
			super(msg);
		}
	}
}
