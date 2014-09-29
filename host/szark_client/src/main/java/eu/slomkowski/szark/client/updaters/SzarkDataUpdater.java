package eu.slomkowski.szark.client.updaters;

import com.google.gson.Gson;
import com.google.gson.GsonBuilder;
import eu.slomkowski.szark.client.status.KillSwitchStatus;
import eu.slomkowski.szark.client.status.Status;

import java.io.IOException;
import java.net.InetSocketAddress;
import java.nio.ByteBuffer;
import java.nio.channels.DatagramChannel;

/**
 * This is the TimerTask, which refreshes the image in the camera view panel and
 * all the indicators.
 *
 * @author Michał Słomkowski
 */
public class SzarkDataUpdater {

	private final Gson gson;
	private final Status status;
	private final ByteBuffer buff = ByteBuffer.allocate(1024);
	private DatagramChannel channel;

	/**
	 * Constructor takes the reference
	 *
	 * @param hostname or the IP address of the SZARK server
	 * @param port     port used by the port
	 */
	public SzarkDataUpdater(String hostname, int port, Status status) {

		gson = new GsonBuilder().excludeFieldsWithoutExposeAnnotation().create();

		this.status = status;

		try {
			channel = DatagramChannel.open();
			channel.connect(new InetSocketAddress(hostname, port));
		} catch (IOException e) {
			e.printStackTrace();
			System.exit(2);
		}

	}

	public synchronized Status update()
			throws HardwareStoppedException, ConnectionErrorException {

		try {
			String output = gson.toJson(status);

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
		} catch (final IOException e) {
			throw new ConnectionErrorException(e);
		}
	}

	public class HardwareStoppedException extends Exception {
		public HardwareStoppedException(String msg) {
			super(msg);
		}
	}

	public class ConnectionErrorException extends IOException {
		public ConnectionErrorException(Throwable e) {
			super(e);
		}
	}
}
