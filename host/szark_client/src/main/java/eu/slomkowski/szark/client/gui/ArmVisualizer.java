package eu.slomkowski.szark.client.gui;

import eu.slomkowski.szark.client.HardcodedConfiguration;
import eu.slomkowski.szark.client.status.Status;

import javax.swing.*;
import java.awt.*;
import java.awt.geom.AffineTransform;
import java.awt.geom.Point2D;

public class ArmVisualizer extends JPanel {

	final double SHOULDER_LENGTH = 53.5;
	final double SHOULDER_MIN_ANGLE = -41;
	final double SHOULDER_ANGLE_RANGE = 101;
	final double SHOULDER_MAX_POSITION = 79;

	final double ELBOW_LENGTH = 65.5;
	final double ELBOW_MIN_ANGLE = -20;
	final double ELBOW_ANGLE_RANGE = 105;
	final double ELBOW_MAX_POSITION = 105;

	final double MACHINE_WIDTH = 51;
	final double MACHINE_HEIGHT = 17.5;
	final double MACHINE_SHOULDER_BASE_OFFSET = 14.5;

	private Status status;

	public ArmVisualizer(Status status) {
		this();

		this.status = status;
	}

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

		if (!isEnabled() || (status == null)) {
			return;
		}

		final Graphics2D g = (Graphics2D) graphics;
		g.setRenderingHint(RenderingHints.KEY_ANTIALIASING, RenderingHints.VALUE_ANTIALIAS_ON);
		g.setRenderingHint(RenderingHints.KEY_TEXT_ANTIALIASING, RenderingHints.VALUE_TEXT_ANTIALIAS_ON);

		g.setStroke(new BasicStroke(5));
		final double SCALE = 1.3;
		//final AffineTransform afSave = g.getTransform();

		final Dimension lo = new Dimension(getWidth() - 10, getHeight() - 23);

		g.setColor(Color.black);

		Point shoulderBase = new Point(lo.width / 2, lo.height);

		// g.fillRect(34, 34, 55, 33);
		// g.fillRect(up.width, up.height, lo.width, lo.height);
		g.fillArc(shoulderBase.x - 25, shoulderBase.y - 5, 50, 50, 0, 180);

		double minShoulderAngle = Math.toRadians(SHOULDER_MIN_ANGLE);
		double shoulderAngleRange = Math.toRadians(SHOULDER_ANGLE_RANGE);
		double shoulderPart = (double) status.joints.shoulder.getPosition() / SHOULDER_MAX_POSITION;
		double shoulderAngle = shoulderPart * shoulderAngleRange + minShoulderAngle;

		double minElbowAngle = Math.toRadians(ELBOW_MIN_ANGLE) + shoulderAngle - minShoulderAngle;
		double elbowAngleRange = Math.toRadians(ELBOW_ANGLE_RANGE);
		double elbowPart = (double) status.joints.elbow.getPosition() / ELBOW_MAX_POSITION;
		double elbowAngle = elbowPart * elbowAngleRange + minElbowAngle;

		Point elbowBase = new Point();
		elbowBase.setLocation(shoulderBase.x - SCALE * SHOULDER_LENGTH * Math.sin(shoulderAngle),
				shoulderBase.y - SCALE * SHOULDER_LENGTH * Math.cos(shoulderAngle));

		Point gripperBase = new Point();
		gripperBase.setLocation(elbowBase.x - SCALE * ELBOW_LENGTH * Math.sin(elbowAngle),
				elbowBase.y - SCALE * ELBOW_LENGTH * Math.cos(elbowAngle));
		
		g.drawLine(shoulderBase.x, shoulderBase.y, elbowBase.x, elbowBase.y);

		g.drawLine(elbowBase.x, elbowBase.y, gripperBase.x, gripperBase.y);

		//g.drawLine(basePoint.x, b

		//final Rectangle p = new Rectangle(lo.width / 2, lo.height, 5, 80);

		//g.draw(p);

		//g.setTransform(afSave);
	}
}
