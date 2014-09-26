package eu.slomkowski.szark.client.updaters;

import com.google.gson.Gson;
import com.google.gson.GsonBuilder;
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
			// TODO Auto-generated catch block
			e.printStackTrace();
		}

	}

	/**
	 * This function is performed by the Timer call.
	 *
	 * @throws HardwareStoppedException , ConnectException,
	 */
	public synchronized Status update() throws HardwareStoppedException, ConnectionErrorException {

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
			Status recvStatus = gson.fromJson(receivedJson, Status.class);

			fillStatus(recvStatus);

			return recvStatus;
		} catch (final IOException e) {
			e.printStackTrace();
			throw new ConnectionErrorException(e);
		}
	}

	private void fillStatus(Status recv) throws HardwareStoppedException {
		status.battery = recv.battery;

		//status.joints.shoulder = recv.joints.shoulder;
		//status.joints.elbow = recv.joints.elbow;
		//status.joints.gripper = recv.joints.gripper;

		//TODO emergency stopped
	}

	public class HardwareStoppedException extends Exception {
	}

	public class ConnectionErrorException extends IOException {
		public ConnectionErrorException(Throwable e) {
			super(e);
		}
	}
}
