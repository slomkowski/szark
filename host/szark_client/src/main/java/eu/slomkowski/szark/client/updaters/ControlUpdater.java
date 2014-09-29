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
			channel.connect(new InetSocketAddress(hostname, HardcodedConfiguration.SZARK_SERVER_PORT));
		} catch (IOException e) {
			e.printStackTrace();
			System.exit(2);
		}

	}

	private synchronized Status updateCycle() throws HardwareStoppedException, IOException {
		String output = gson.toJson(status);
		ByteBuffer buff = ByteBuffer.allocate(1024);

		buff.clear();
		buff.put(output.getBytes());
		buff.flip();

		channel.write(buff);
		status.incrementSerial();

		buff.clear();
		int length = channel.read(buff);
		String receivedJson = new String(buff.array(), 0, length);
		Status receivedStatus = gson.fromJson(receivedJson, Status.class);

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
		try {
			while (!needToStop.get()) {

				if (joystickDataUpdater != null) {
					joystickDataUpdater.update();
				}

				Status receivedStatus = updateCycle();
				publish(receivedStatus);

				Thread.sleep(HardcodedConfiguration.SZARK_REFRESH_INTERVAL, 0);
			}

		} catch (final IOException e) {
			mainWindow.thingsWhenDisconnect(false);
			// TODO disable only server
			JOptionPane.showMessageDialog(mainWindow,
					String.format("Control communication error: %s. Disabling control.",
							e.getMessage() != null ? e.getMessage() : e.getClass().getName()),
					"Network error",
					JOptionPane.ERROR_MESSAGE);
		} catch (final HardwareStoppedException e) {
			mainWindow.thingsWhenDisabling();
			JOptionPane.showMessageDialog(mainWindow,
					"Kill switch: " + e.getMessage(),
					"Kill switch activated",
					JOptionPane.WARNING_MESSAGE);
		}
		return null;
	}

	@Override
	protected void process(List<Status> chunks) {
		mainWindow.updateIndicators(chunks.get(chunks.size() - 1));
	}

	public void stopTask() {
		needToStop.set(true);

		try {
			updateCycle();
		} catch (HardwareStoppedException | IOException e) {
			e.printStackTrace();
		}
	}

	public class HardwareStoppedException extends Exception {
		public HardwareStoppedException(String msg) {
			super(msg);
		}
	}
}
