package eu.slomkowski.szark.client.updaters;

import com.google.gson.Gson;
import com.google.gson.GsonBuilder;
import eu.slomkowski.szark.client.HardcodedConfiguration;
import eu.slomkowski.szark.client.gui.MainWindowLogic;
import eu.slomkowski.szark.client.joystick.JoystickBackend;
import eu.slomkowski.szark.client.status.KillSwitchStatus;
import eu.slomkowski.szark.client.status.Status;

import javax.swing.*;
import java.io.IOException;
import java.net.InetSocketAddress;
import java.nio.ByteBuffer;
import java.nio.channels.DatagramChannel;
import java.util.List;
import java.util.concurrent.atomic.AtomicBoolean;

public class ControlUpdater extends SwingWorker<Void, Status> {
	private final Gson gson;
	private final Status status;
	private final AtomicBoolean needToStop = new AtomicBoolean(false);
	private final MainWindowLogic mainWindow;
	private final ByteBuffer buff = ByteBuffer.allocate(1024);
	private DatagramChannel channel;
	private JoystickDataUpdater joystickDataUpdater = null;

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
			channel = DatagramChannel.open();
			channel.connect(new InetSocketAddress(hostname, HardcodedConfiguration.CONTROL_SERVER_PORT));
			channel.socket().setSoTimeout(1000);
		} catch (IOException e) {
			e.printStackTrace();
			System.exit(2);
		}
	}

	private Status updateCycle() throws HardwareStoppedException, IOException {
		String output = gson.toJson(status);

		buff.clear();
		buff.put(output.getBytes());
		buff.flip();

		channel.write(buff);

		buff.clear();
		int length = channel.read(buff);
		String receivedJson = new String(buff.array(), 0, length);
		Status receivedStatus = gson.fromJson(receivedJson, Status.class);

		if (status.getSerial() != receivedStatus.getSerial()) {
			System.err.println(String.format("Sent(%d) and received(%d) serial numbers didn't match",
					status.getSerial(), receivedStatus.getSerial()));

			status.setSerial(Math.max(receivedStatus.getSerial(), status.getSerial()) + 2);

			return null;
		}

		status.incrementSerial();

		if (!status.isKillswitchEnable() &&
				receivedStatus.getReceivedKillSwitchStatus() != KillSwitchStatus.INACTIVE) {

			if (receivedStatus.getReceivedKillSwitchStatus() == KillSwitchStatus.ACTIVE_HARDWARE) {
				throw new HardwareStoppedException("Kill switch has been pressed!");
			} else {
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
			channel.disconnect();
			channel.close();

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
