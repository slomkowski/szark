package eu.slomkowski.szark.client.gui;

import eu.slomkowski.szark.client.status.Status;

import javax.swing.*;
import java.awt.*;
import java.awt.geom.AffineTransform;

public class ArmVisualizer extends JPanel {

	private static final long serialVersionUID = 1L;

	private Status status;

	public ArmVisualizer(Status status) {
		this();

		this.status = status;
	}

	public void setUpdateStatus(Status status) {
		this.status = status;
		repaint();
	}

	public ArmVisualizer() {
		setPreferredSize(new Dimension(250, 250));
		setEnabled(false);
	}

	@Override
	public void setEnabled(boolean enable) {
		if (status == null) {
			return;
		}

		super.setEnabled(enable);
		repaint();
	}

	@Override
	public void paintComponent(Graphics graphics) {
		super.paintComponent(graphics);
		// if(!isEnabled() || (status == null)) return;

		final Graphics2D g = (Graphics2D) graphics;
		g.setRenderingHint(RenderingHints.KEY_ANTIALIASING, RenderingHints.VALUE_ANTIALIAS_ON);
		g.setRenderingHint(RenderingHints.KEY_TEXT_ANTIALIASING, RenderingHints.VALUE_TEXT_ANTIALIAS_ON);

		final AffineTransform afSave = g.getTransform();

		final Dimension lo = new Dimension(getWidth() - 10, getHeight() - 23);

		g.setColor(Color.black);

		// g.fillRect(34, 34, 55, 33);
		// g.fillRect(up.width, up.height, lo.width, lo.height);
		g.fillArc(lo.width / 2 - 25, lo.height - 5, 50, 50, 0, 180);

		final Rectangle p = new Rectangle(lo.width / 2, lo.height, 5, 80);

		g.draw(p);

		g.setTransform(afSave);
	}
}
