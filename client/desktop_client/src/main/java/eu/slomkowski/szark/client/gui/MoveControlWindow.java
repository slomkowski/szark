package eu.slomkowski.szark.client.gui;

import eu.slomkowski.szark.client.status.Direction;
import eu.slomkowski.szark.client.status.Status;

import javax.swing.*;
import java.awt.*;
import java.awt.event.*;

/**
 * This class creates a window with buttons to control the direction of the
 * robot as well as the slider to control speed. It may be helpful when joystick
 * is not connected. The window also grab the keys: arrows and shift - used as
 * stop button.
 *
 * @author Michał Słomkowski
 */
public class MoveControlWindow extends JDialog implements ActionListener, KeyEventDispatcher, AdjustmentListener {

    // the reference to the SzarkStatus class - it will be modified by button
    // presses etc.
    private final Status status;

    private final JPanel directionP = new JPanel(new GridLayout(3, 3));

    private final JButton directionForwardB = new JButton(new ImageIcon(getClass().getResource("/img/forward.png")));
    private final JButton directionForwardLeftB = new JButton(new ImageIcon(getClass().getResource(
            "/img/left-forward.png")));
    private final JButton directionForwardRightB = new JButton(new ImageIcon(getClass().getResource(
            "/img/right-forward.png")));

    private final JButton directionBackwardB = new JButton(new ImageIcon(getClass().getResource("/img/backward.png")));
    private final JButton directionBackwardLeftB = new JButton(new ImageIcon(getClass().getResource(
            "/img/left-backward.png")));
    private final JButton directionBackwardRightB = new JButton(new ImageIcon(getClass().getResource(
            "/img/right-backward.png")));

    private final JButton directionRotateLeftB = new JButton(new ImageIcon(getClass().getResource(
            "/img/rotate-left.png")));
    private final JButton directionRotateRightB = new JButton(new ImageIcon(getClass().getResource(
            "/img/rotate-right.png")));

    private final JButton directionStopB = new JButton(new ImageIcon(getClass().getResource("/img/stop-control.png")));

    private final JPanel speedP = new JPanel(new FlowLayout());
    private final JScrollBar speedControlBar = new JScrollBar(JScrollBar.HORIZONTAL, 0, 0, 0, 15); // start
    // value,
    // extent,
    // min,
    // max
    private final JTextField speedValueField = new JTextField(Integer.toString(speedControlBar.getValue()), 2);

    private static final long serialVersionUID = 1L;

    /**
     * Default constructor.
     *
     * @param status - reference to the SzarkStatus object, which will be
     *               modified by the class.
     */
    public MoveControlWindow(final Status status) {
        this.status = status;

        // grid of buttons
        directionP.add(directionForwardLeftB);
        directionP.add(directionForwardB);
        directionP.add(directionForwardRightB);

        directionP.add(directionRotateLeftB);
        directionP.add(directionStopB);
        directionP.add(directionRotateRightB);

        directionP.add(directionBackwardLeftB);
        directionP.add(directionBackwardB);
        directionP.add(directionBackwardRightB);

        // speed slider and labels
        speedP.add(new JLabel("Speed:"), BorderLayout.WEST);
        speedP.add(speedControlBar, BorderLayout.CENTER);
        speedP.add(speedValueField, BorderLayout.EAST);

        speedControlBar.setPreferredSize(new Dimension(150, 20));

        // window settings
        setLayout(new BorderLayout());
        add(directionP, BorderLayout.NORTH);
        add(speedP, BorderLayout.SOUTH);

        setTitle("SZARK - Movement Control");
        setDefaultCloseOperation(HIDE_ON_CLOSE);
        pack();
        setResizable(false);
        setVisible(false);
        setEnabled(false);

        // listeners
        KeyboardFocusManager.getCurrentKeyboardFocusManager().addKeyEventDispatcher(this);

        speedControlBar.addAdjustmentListener(this);

        directionForwardB.addActionListener(this);
        directionForwardLeftB.addActionListener(this);
        directionForwardRightB.addActionListener(this);

        directionBackwardB.addActionListener(this);
        directionBackwardLeftB.addActionListener(this);
        directionBackwardRightB.addActionListener(this);

        directionRotateLeftB.addActionListener(this);
        directionRotateRightB.addActionListener(this);

        directionStopB.addActionListener(this);

    }

	/*
     * directionForwardB directionForwardLeftB directionForwardRightB
	 * 
	 * directionBackwardB directionBackwardLeftB directionBackwardRightB
	 * 
	 * directionRotateLeftB directionRotateRightB
	 * 
	 * directionStopB
	 */

    @Override
    public void setEnabled(boolean enabled) {
        directionForwardB.setEnabled(enabled);
        directionForwardLeftB.setEnabled(enabled);
        directionForwardRightB.setEnabled(enabled);

        directionBackwardB.setEnabled(enabled);
        directionBackwardLeftB.setEnabled(enabled);
        directionBackwardRightB.setEnabled(enabled);

        directionRotateLeftB.setEnabled(enabled);
        directionRotateRightB.setEnabled(enabled);

        directionStopB.setEnabled(enabled);
        speedControlBar.setEnabled(enabled);
        speedValueField.setEnabled(enabled);
    }

    @Override
    public void adjustmentValueChanged(AdjustmentEvent e) {
        speedValueField.setText(Integer.toString(e.getValue()));

        status.motors.left.setSpeed((byte) e.getValue());
        status.motors.right.setSpeed((byte) e.getValue());
    }

    @Override
    public boolean dispatchKeyEvent(KeyEvent e) {
        if (e.getID() == KeyEvent.KEY_PRESSED) {
            switch (e.getKeyCode()) {
                case KeyEvent.VK_UP:
                    directionForwardB.doClick();
                    break;
                case KeyEvent.VK_DOWN:
                    directionBackwardB.doClick();
                    break;
                case KeyEvent.VK_LEFT:
                    directionRotateLeftB.doClick();
                    break;
                case KeyEvent.VK_RIGHT:
                    directionRotateRightB.doClick();
                    break;
                case KeyEvent.VK_SHIFT:
                    directionStopB.doClick();
                    break;
            }
        }
        return true;
    }

    @Override
    public void actionPerformed(ActionEvent e) {
        if (e.getSource() == directionForwardB) {
            status.motors.left.setDirection(Direction.FORWARD);
            status.motors.right.setDirection(Direction.FORWARD);
        } else if (e.getSource() == directionForwardLeftB) {
            status.motors.left.setDirection(Direction.STOP);
            status.motors.right.setDirection(Direction.FORWARD);
        } else if (e.getSource() == directionForwardRightB) {
            status.motors.left.setDirection(Direction.FORWARD);
            status.motors.right.setDirection(Direction.STOP);
        } else if (e.getSource() == directionRotateLeftB) {
            status.motors.left.setDirection(Direction.BACKWARD);
            status.motors.right.setDirection(Direction.FORWARD);
        } else if (e.getSource() == directionStopB) {
            status.motors.left.setDirection(Direction.STOP);
            status.motors.right.setDirection(Direction.STOP);
        } else if (e.getSource() == directionRotateRightB) {
            status.motors.left.setDirection(Direction.FORWARD);
            status.motors.right.setDirection(Direction.BACKWARD);
        } else if (e.getSource() == directionBackwardB) {
            status.motors.left.setDirection(Direction.BACKWARD);
            status.motors.right.setDirection(Direction.BACKWARD);
        } else if (e.getSource() == directionBackwardLeftB) {
            status.motors.left.setDirection(Direction.STOP);
            status.motors.right.setDirection(Direction.BACKWARD);
        } else if (e.getSource() == directionBackwardRightB) {
            status.motors.left.setDirection(Direction.BACKWARD);
            status.motors.right.setDirection(Direction.STOP);
        }
    }

}
