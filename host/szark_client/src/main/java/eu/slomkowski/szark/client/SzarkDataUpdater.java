package eu.slomkowski.szark.client;

import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.net.InetSocketAddress;
import java.net.Socket;
import java.util.Arrays;

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
		server_addr = new InetSocketAddress(hostname, SZARKServerPort);
		this.status = status;
	}

	private final InetSocketAddress server_addr;
	private final SzarkStatus status;

	private final StringBuffer send = new StringBuffer();
	private final byte[] received = new byte[Hardcoded.MAX_DATA_LENGTH];

	private boolean emergencyStoppedOld = true;
	private boolean connectionError = false;

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
		// cleaning
		send.delete(0, send.length());
		Arrays.fill(received, (byte) 0);

		if (connectionError) {
			return;
		}

		try {
			final Socket socket = new Socket();
			socket.connect(server_addr);// , Hardcoded.SOCKET_TIMEOUT);

			final InputStream in = socket.getInputStream();
			final OutputStream out = socket.getOutputStream();

			// SERVER COMMANDS
			if (status.server.mustShutdown()) {
				send.append("Ss");
			}
			if (status.server.mustExit()) {
				send.append("Sx");
			}
			if (status.server.mustReboot()) {
				send.append("Sr");
			}

			// ENABLING OR DISABLING
			if (status.isEmergencyStopped()) {
				send.append("Ed");
				emergencyStoppedOld = true;
			} else {
				if (emergencyStoppedOld) {
					send.append("Ee");
					emergencyStoppedOld = false;
				}

				// MOTORS SETTINGS

				// left direction
				if (status.motors.left.getDirection() == SzarkStatus.Direction.FORWARD) {
					send.append("Mdlf");
				} else if (status.motors.left.getDirection() == SzarkStatus.Direction.BACKWARD) {
					send.append("Mdlb");
				} else {
					send.append("Mdl0");
				}
				// left speed
				send.append("Msl");
				send.append((char) ('a' + status.motors.left.getSpeed()));
				// right direction
				if (status.motors.right.getDirection() == SzarkStatus.Direction.FORWARD) {
					send.append("Mdrf");
				} else if (status.motors.right.getDirection() == SzarkStatus.Direction.BACKWARD) {
					send.append("Mdrb");
				} else {
					send.append("Mdr0");
				}
				// right speed
				send.append("Msr");
				send.append((char) ('a' + status.motors.right.getSpeed()));

				// LIGHTS
				if (status.lights.isHigh()) {
					send.append("Lhe");
				} else {
					send.append("Lhd");
				}
				if (status.lights.isLow()) {
					send.append("Lle");
				} else {
					send.append("Lld");
				}
				if (status.lights.isGripper()) {
					send.append("Lge");
				} else {
					send.append("Lgd");
				}
				if (status.lights.isCamera()) {
					send.append("Lce");
				} else {
					send.append("Lcd");
				}

				// ARM
				if (status.arm.getCalStatus() == SzarkStatus.CalStatus.REQUESTED) {
					send.append("AC");
					status.arm.setCalStatus(SzarkStatus.CalStatus.REQ_SENT);
				} else {
					if (status.arm.shoulder.getDirection() == SzarkStatus.Direction.FORWARD) {
						send.append("Adsf");
					} else if (status.arm.shoulder.getDirection() == SzarkStatus.Direction.BACKWARD) {
						send.append("MAdsb");
					} else {
						send.append("Ads0");
					}
					send.append("Ass");
					send.append((char) ('a' + status.arm.shoulder.getSpeed()));

					if (status.arm.elbow.getDirection() == SzarkStatus.Direction.FORWARD) {
						send.append("Adef");
					} else if (status.arm.elbow.getDirection() == SzarkStatus.Direction.BACKWARD) {
						send.append("MAdeb");
					} else {
						send.append("Ade0");
					}
					send.append("Ase");
					send.append((char) ('a' + status.arm.elbow.getSpeed()));

					if (status.arm.wrist.getDirection() == SzarkStatus.Direction.FORWARD) {
						send.append("Adwf");
					} else if (status.arm.wrist.getDirection() == SzarkStatus.Direction.BACKWARD) {
						send.append("MAdwb");
					} else {
						send.append("Adw0");
					}
					send.append("Asw");
					send.append((char) ('a' + status.arm.wrist.getSpeed()));

					if (status.arm.gripper.getDirection() == SzarkStatus.Direction.FORWARD) {
						send.append("Adgf");
					} else if (status.arm.gripper.getDirection() == SzarkStatus.Direction.BACKWARD) {
						send.append("MAdgb");
					} else {
						send.append("Adg0");
					}
					send.append("Asg");
					send.append((char) ('a' + status.arm.gripper.getSpeed()));
				}

			}
			// BATTERY DATA REQUEST
			send.append("Bg");

			// REQUEST FOR EXACT ARM DIRECTIONS
			send.append("AbsAbeAbwAbg");

			// sending the command string
			out.write(send.toString().getBytes());
			// receiving data from the server
			in.read(received, 0, Hardcoded.MAX_DATA_LENGTH);

			out.close();
			in.close();
			socket.close();

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
							throw new HardwareStoppedException();
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
		} catch (final IOException e) {
			connectionError = true;
			e.printStackTrace();
			throw new ConnectionErrorException(e);
		}
	}
}
