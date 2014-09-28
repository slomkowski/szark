package eu.slomkowski.szark.client.updaters;

import eu.slomkowski.szark.client.HardcodedConfiguration;
import eu.slomkowski.szark.client.utils.ByteBufferBackedInputStream;

import java.awt.*;
import java.awt.image.BufferedImage;
import java.io.IOException;
import java.net.InetSocketAddress;
import java.nio.ByteBuffer;
import java.nio.channels.DatagramChannel;
import java.util.List;
import java.util.concurrent.atomic.AtomicBoolean;

import javax.imageio.ImageIO;
import javax.swing.*;

public class CameraImageUpdater extends JLabel {

	private UpdateTask updateTask = null;
	private Camera choosenCamera;
	private DatagramChannel channel;

	public void setChoosenCamera(Camera choosenCamera) {
		this.choosenCamera = choosenCamera;
	}

	public void enableCameraView(String hostname) {
		setIcon(null);
		try {
			channel = DatagramChannel.open();
			channel.connect(new InetSocketAddress(hostname, 10192));
		} catch (final IOException e) {
			e.printStackTrace();
		}

		updateTask = new UpdateTask();
		updateTask.execute();
	}

	public void disableCameraView() {
		if (updateTask != null) {
			updateTask.stopThread();
			updateTask = null;
		}

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

	public static enum Camera {
		HEAD, GRIPPER
	}

	private class UpdateTask extends SwingWorker<Void, BufferedImage> {
		private final ByteBuffer buff = ByteBuffer.allocate(50000);
		private AtomicBoolean needToStop = new AtomicBoolean(false);
		private BufferedImage image = null;

		@Override
		protected Void doInBackground() throws Exception {
			while (needToStop.get() == false) {
				try {
					buff.clear();
					buff.put((byte) 1);

					channel.write(buff);

					buff.clear();
					channel.read(buff);

					buff.flip();

					if (needToStop.get()) {
						break;
					}

					publish(ImageIO.read(new ByteBufferBackedInputStream(buff)));

				} catch (IOException e) {
					e.printStackTrace();
				}
			}

			return null;
		}

		@Override
		protected void process(List<BufferedImage> chunks) {
			image = chunks.get(chunks.size() - 1);
			CameraImageUpdater.this.repaint();
		}

		public void stopThread() {
			needToStop.set(true);
		}

		public BufferedImage getBufferedImage() {
			return image;
		}
	}
}