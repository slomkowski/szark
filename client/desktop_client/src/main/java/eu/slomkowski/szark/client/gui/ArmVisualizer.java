package eu.slomkowski.szark.client.gui;

import eu.slomkowski.szark.client.status.Status;

import javax.swing.*;
import java.awt.*;

public class ArmVisualizer extends JPanel {

	private final double SHOULDER_LENGTH = 53.5;
	private final double SHOULDER_MIN_ANGLE = -41;
	private final double SHOULDER_ANGLE_RANGE = 101;
	private final double SHOULDER_MAX_POSITION = 79;

	private final double ELBOW_LENGTH = 65.5;
	private final double ELBOW_MIN_ANGLE = -20;
	private final double ELBOW_ANGLE_RANGE = 105;
	private final double ELBOW_MAX_POSITION = 105;

	private final double MACHINE_WIDTH = 51;
	private final double MACHINE_HEIGHT = 17.5;
	private final double MACHINE_SHOULDER_BASE_OFFSET = 36.5;

	private Status status;

	public ArmVisualizer() {
		setPreferredSize(new Dimension(250, 250));
		setEnabled(false);
	}

	public void setUpdateStatus(Status status) {
		this.status = status;
		repaint();
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

		if (status == null) {
			return;
		}

		final Graphics2D g = (Graphics2D) graphics;
		g.setRenderingHint(RenderingHints.KEY_ANTIALIASING, RenderingHints.VALUE_ANTIALIAS_ON);
		g.setRenderingHint(RenderingHints.KEY_TEXT_ANTIALIASING, RenderingHints.VALUE_TEXT_ANTIALIAS_ON);

		final double SCALE = 1.5;

		if (isEnabled()) {
			g.setColor(Color.black);
		} else {
			g.setColor(Color.gray);
		}

		double minShoulderAngle = Math.toRadians(SHOULDER_MIN_ANGLE);
		double shoulderAngleRange = Math.toRadians(SHOULDER_ANGLE_RANGE);
		double shoulderPart = (double) status.joints.shoulder.getPosition() / SHOULDER_MAX_POSITION;
		double shoulderAngle = shoulderPart * shoulderAngleRange + minShoulderAngle;

		double minElbowAngle = Math.toRadians(ELBOW_MIN_ANGLE) + shoulderAngle - minShoulderAngle;
		double elbowAngleRange = Math.toRadians(ELBOW_ANGLE_RANGE);
		double elbowPart = (double) status.joints.elbow.getPosition() / ELBOW_MAX_POSITION;
		double elbowAngle = elbowPart * elbowAngleRange + minElbowAngle;


		Point basePoint = new Point(
				getWidth() / 2,
				getHeight() - 70);

		Point shoulderBase = new Point(
				(int) Math.round(basePoint.x + SCALE * MACHINE_SHOULDER_BASE_OFFSET),
				basePoint.y);

		Point elbowBase = new Point(
				(int) Math.round(shoulderBase.x - SCALE * SHOULDER_LENGTH * Math.sin(shoulderAngle)),
				(int) Math.round(shoulderBase.y - SCALE * SHOULDER_LENGTH * Math.cos(shoulderAngle)));

		Point gripperBase = new Point(
				(int) Math.round(elbowBase.x - SCALE * ELBOW_LENGTH * Math.sin(elbowAngle)),
				(int) Math.round(elbowBase.y - SCALE * ELBOW_LENGTH * Math.cos(elbowAngle)));

		g.setStroke(new BasicStroke(5));

		fillOval(g, shoulderBase, 12);
		g.drawLine(shoulderBase.x, shoulderBase.y, elbowBase.x, elbowBase.y);

		fillOval(g, elbowBase, 12);
		g.drawLine(elbowBase.x, elbowBase.y, gripperBase.x, gripperBase.y);

		g.setStroke(new BasicStroke(3));

		g.drawRect(basePoint.x, basePoint.y, (int) Math.round(SCALE * MACHINE_WIDTH),
				(int) Math.round(SCALE * MACHINE_HEIGHT));
	}

	private void fillOval(Graphics g, Point center, int diameter) {
		int radius = (int) Math.round(diameter / 2.0);
		g.fillOval(center.x - radius, center.y - radius, diameter, diameter);
	}
}
