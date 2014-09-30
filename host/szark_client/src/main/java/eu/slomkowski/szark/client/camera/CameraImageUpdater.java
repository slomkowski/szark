package eu.slomkowski.szark.client.camera;

import eu.slomkowski.szark.client.HardcodedConfiguration;
import eu.slomkowski.szark.client.utils.ByteBufferBackedInputStream;

import java.awt.*;
import java.awt.image.BufferedImage;
import java.io.IOException;
import java.net.InetSocketAddress;
import java.nio.ByteBuffer;
import java.nio.channels.DatagramChannel;
import java.util.List;
import java.util.concurrent.ExecutionException;
import java.util.concurrent.atomic.AtomicBoolean;

import javax.imageio.ImageIO;
import javax.swing.*;

public class CameraImageUpdater extends JLabel {

	private UpdateTask updateTask = null;
	private CameraType chosenCameraType = CameraType.HEAD;
	private DatagramChannel channel;
	private String hostname;
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
		enableCameraView(hostname);
	}

	public void setCameraMode(CameraMode cameraMode) {
		this.cameraMode = cameraMode;
	}

	public void enableCameraView(String hostname) {
		this.hostname = hostname;
		setIcon(null);

		try {
			channel = DatagramChannel.open();

			int port = chosenCameraType == CameraType.GRIPPER
					? HardcodedConfiguration.CAMERA_PORT_GRIPPER
					: HardcodedConfiguration.CAMERA_PORT_HEAD;

			channel.connect(new InetSocketAddress(hostname, port));
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
		private final ByteBuffer buff = ByteBuffer.allocate(50000);
		private AtomicBoolean needToStop = new AtomicBoolean(false);
		private BufferedImage image = null;

		@Override
		protected Void doInBackground() throws Exception {
			while (!needToStop.get()) {
				try {
					buff.clear();
					buff.put(cameraMode.getMnemonic().getBytes());

					channel.write(buff);

					buff.clear();
					channel.read(buff);

					buff.flip();

					if (needToStop.get()) {
						break;
					}

					BufferedImage img = ImageIO.read(new ByteBufferBackedInputStream(buff));

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
			try {
				channel.disconnect();
				channel.close();
			} catch (IOException e) {
				e.printStackTrace();
			}
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