package eu.slomkowski.szark.client;

import com.google.gson.Gson;
import com.google.gson.GsonBuilder;

import java.io.IOException;
import java.net.InetSocketAddress;
import java.nio.ByteBuffer;
import java.nio.channels.DatagramChannel;
import java.util.concurrent.TimeoutException;

/**
 * This is the TimerTask, which refreshes the image in the camera view panel and
 * all the indicators.
 *
 * @author Michał Słomkowski
 */
public class SzarkDataUpdater {

	private final Gson gson;
	private final SzarkStatus status;
	private final ByteBuffer buff = ByteBuffer.allocate(1024);
	private DatagramChannel channel;

	/**
	 * Constructor takes the reference
	 *
	 * @param hostname or the IP address of the SZARK server
	 * @param port     port used by the port
	 */
	public SzarkDataUpdater(String hostname, int port, SzarkStatus status) {

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
	public synchronized void update() throws HardwareStoppedException, ConnectionErrorException {

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
			SzarkStatus recvStatus = gson.fromJson(receivedJson, SzarkStatus.class);

			fillStatus(recvStatus);
		} catch (final IOException e) {
			e.printStackTrace();
			throw new ConnectionErrorException(e);
		}
	}

	private void fillStatus(SzarkStatus recv) throws HardwareStoppedException {
		status.battery = recv.battery;

		//status.arm.shoulder = recv.arm.shoulder;
		//status.arm.elbow = recv.arm.elbow;
		//status.arm.gripper = recv.arm.gripper;

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
