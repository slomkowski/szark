package eu.slomkowski.szark.client.camera;

import eu.slomkowski.szark.client.HardcodedConfiguration;

import javax.imageio.ImageIO;
import javax.swing.*;
import java.awt.*;
import java.awt.image.BufferedImage;
import java.io.ByteArrayInputStream;
import java.io.IOException;
import java.net.DatagramPacket;
import java.net.DatagramSocket;
import java.net.InetSocketAddress;
import java.net.SocketTimeoutException;
import java.util.List;
import java.util.concurrent.atomic.AtomicBoolean;

public class CameraImageUpdater extends JLabel {

	private UpdateTask updateTask = null;
	private CameraType chosenCameraType = CameraType.HEAD;
	private InetSocketAddress address;
	private DatagramSocket socket;
	private boolean enabled = false;
	private CameraMode cameraMode = CameraMode.HUD;

	public void setChosenCameraType(CameraType chosenCameraType) {
		if (chosenCameraType == this.chosenCameraType && enabled) {
			return;
		}

		if (enabled) {
			disableCameraView();
		}
		this.chosenCameraType = chosenCameraType;
		enableCameraView(address.getHostName());
	}

	public void setCameraMode(CameraMode cameraMode) {
		this.cameraMode = cameraMode;
	}

	public void enableCameraView(String hostname) {
		setIcon(null);

		try {
			socket = new DatagramSocket();
			socket.setSoTimeout(HardcodedConfiguration.CAMERA_TIMEOUT);

			int port = chosenCameraType == CameraType.GRIPPER
					? HardcodedConfiguration.CAMERA_PORT_GRIPPER
					: HardcodedConfiguration.CAMERA_PORT_HEAD;

			this.address = new InetSocketAddress(hostname, port);
		} catch (final IOException e) {
			e.printStackTrace();
			System.exit(1);
		}

		enabled = true;

		updateTask = new UpdateTask();
		updateTask.execute();
	}

	public void disableCameraView() {
		if (updateTask != null) {
			updateTask.stopTask();

			updateTask = null;
		}

		enabled = false;
		setIcon(new ImageIcon(getClass().getResource(HardcodedConfiguration.DEFAULT_LOGO)));
		repaint();
	}

	@Override
	protected void paintComponent(Graphics g) {
		super.paintComponent(g);

		if (updateTask == null) {
			return;
		}

		g.drawImage(updateTask.getBufferedImage(), 0, 0, this);
	}

	private class UpdateTask extends SwingWorker<Void, BufferedImage> {
		private byte[] inputBuffer = new byte[100000];
		private AtomicBoolean needToStop = new AtomicBoolean(false);
		private BufferedImage image = null;

		@Override
		protected Void doInBackground() throws Exception {
			while (!needToStop.get()) {
				try {
					DatagramPacket sendPacket = new DatagramPacket(cameraMode.getMnemonic().getBytes(),
							cameraMode.getMnemonic().length(), address);

					socket.send(sendPacket);

					DatagramPacket receivedPacket = new DatagramPacket(inputBuffer, inputBuffer.length);

					try {
						socket.receive(receivedPacket);
					} catch (final SocketTimeoutException e) {
						System.err.println("Camera receive timeout");
						continue;
					}

					if (needToStop.get()) {
						break;
					}

					ByteArrayInputStream stream = new ByteArrayInputStream(inputBuffer, 0, receivedPacket.getLength());
					BufferedImage img = ImageIO.read(stream);
					stream.close();

					if (img == null) {
						throw new Exception("could not parse image. Probably malformed response.");
					}

					publish(img);

				} catch (final Exception e) {
					e.printStackTrace();

					SwingUtilities.invokeLater(new Runnable() {
						@Override
						public void run() {
							disableCameraView();

							JOptionPane.showMessageDialog(CameraImageUpdater.this,
									String.format("Camera communication error: %s. Disabling camera.",
											e.getMessage() != null ? e.getMessage() : e.getClass().getName()),
									"Network error",
									JOptionPane.ERROR_MESSAGE);
						}
					});

					return null;
				}
			}

			return null;
		}

		@Override
		protected void done() {
			socket.close();
		}

		@Override
		protected void process(List<BufferedImage> chunks) {
			image = chunks.get(chunks.size() - 1);
			CameraImageUpdater.this.repaint();
		}

		public void stopTask() {
			needToStop.set(true);
		}

		public BufferedImage getBufferedImage() {
			return image;
		}
	}
}