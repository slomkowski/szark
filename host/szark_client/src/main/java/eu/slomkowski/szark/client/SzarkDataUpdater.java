package eu.slomkowski.szark.client;

import java.io.IOException;
import java.net.DatagramPacket;
import java.net.DatagramSocket;
import java.net.InetAddress;
import java.net.SocketException;
import java.net.UnknownHostException;

import com.google.gson.Gson;
import com.google.gson.GsonBuilder;

/**
 * This is the TimerTask, which refreshes the image in the camera view panel and
 * all the indicators.
 * 
 * @author Michał Słomkowski
 */
public class SzarkDataUpdater {

	/**
	 * Constructor takes the reference
	 * 
	 * @param hostname
	 *             or the IP address of the SZARK server
	 * @param SZARKServerPort
	 *             port used by the SZARKServerPort
	 */

	SzarkDataUpdater(String hostname, int SZARKServerPort, SzarkStatus status) {
		this.status = status;
		udpPort = SZARKServerPort;

		gsonBuilder = new GsonBuilder().excludeFieldsWithoutExposeAnnotation();

		try {
			clientSocket = new DatagramSocket();
			IPAddress = InetAddress.getByName(hostname);

		} catch (UnknownHostException | SocketException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		}

	}

	private DatagramSocket clientSocket;
	private final int udpPort;
	private InetAddress IPAddress;

	private final GsonBuilder gsonBuilder;

	private final SzarkStatus status;

	private final StringBuffer send = new StringBuffer();
	private final byte[] received = new byte[Hardcoded.MAX_DATA_LENGTH];

	private boolean emergencyStoppedOld = true;

	public class HardwareStoppedException extends Exception {

		private static final long serialVersionUID = 1L;
	}

	public class ConnectionErrorException extends IOException {

		private static final long serialVersionUID = 1L;

		public ConnectionErrorException(Throwable e) {
			super(e);
		}
	}

	/**
	 * This function is performed by the Timer call.
	 * 
	 * @throws HardwareStoppedException
	 *              , ConnectException,
	 */
	public synchronized void update() throws HardwareStoppedException, ConnectionErrorException {

		Gson gson = gsonBuilder.create();

		try {
			String output = gson.toJson(status);
			DatagramPacket sendPacket = new DatagramPacket(output.getBytes(), output.length(), IPAddress, udpPort);
			clientSocket.send(sendPacket);
			status.incrementSerial();
		} catch (final IOException e) {
			e.printStackTrace();
			throw new ConnectionErrorException(e);
		}
	}

	private void interpretReceivedData(String data) {
		// BATTERY DATA PROCESING
		for (int i = 0; i < received.length - 5; i++) {
			if ((received[i] == 'B') && (received[i + 1] == 'g')) {
				status.battery.setVoltage((received[i + 2] - 'a') + 0.1f * (received[i + 3] - 'a'));
				status.battery.setCurrent((received[i + 4] - 'a') + 0.1f * (received[i + 5] - 'a'));

				break;
			}
		}

		if (!status.isEmergencyStopped()) {
			// HARDWARE EMERGENCY STOP NOTIFICATION
			for (int i = 0; i < received.length - 1; i++) {
				if ((received[i] == 'E')) {
					if (received[i + 1] == 'h') {
						emergencyStoppedOld = true;
						// TODO throw new HardwareStoppedException();
					}
				}
			}

			// TODO dodanie pobierania kierunku
			for (int i = 0; i < received.length - 1; i++) {
				if ((received[i] == 'A') && (received[i + 1] == 'b')) // arm
															// directions
				{
					SzarkStatus.Direction dir;

					if (received[i + 3] == 'f') {
						dir = SzarkStatus.Direction.FORWARD;
					} else if (received[i + 3] == 'b') {
						dir = SzarkStatus.Direction.BACKWARD;
					} else {
						dir = SzarkStatus.Direction.STOP;
					}

					switch (received[i + 2]) {
					case 's':
						status.arm.shoulder.setExactDirection(dir);
						break;
					case 'e':
						status.arm.elbow.setExactDirection(dir);
						break;
					case 'w':
						status.arm.wrist.setExactDirection(dir);
						break;
					case 'g':
						status.arm.gripper.setExactDirection(dir);
						break;
					}
				}
			}
		}
	}
}
