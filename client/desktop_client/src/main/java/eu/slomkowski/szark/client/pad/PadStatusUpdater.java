package eu.slomkowski.szark.client.pad;

import eu.slomkowski.szark.client.pad.xbox360.Xbox360AxesStateProvider;
import eu.slomkowski.szark.client.status.AbstractMotor;
import eu.slomkowski.szark.client.status.Direction;
import eu.slomkowski.szark.client.status.Status;

public class PadStatusUpdater {
    private AxesStateProvider provider;

    private static final double EPSILON = 0.001;

    public PadStatusUpdater() throws PadException {
        provider = new Xbox360AxesStateProvider();
    }

    public void fillWithCurrentData(Status status) throws PadException {
        AxesState axesState = provider.getAxesState();

        updateAxis(status.motors.left, axesState.getLeftCaterpillar());
        updateAxis(status.motors.right, axesState.getRightCaterpillar());
        updateAxis(status.joints.shoulder, axesState.getShoulderJoint());
        updateAxis(status.joints.elbow, axesState.getElbowJoint());
        updateAxis(status.joints.gripper, axesState.getGripperJoint());
    }

    private void updateAxis(AbstractMotor.AbstractParams motor, double axisValue) {
        if (Math.abs(axisValue) < EPSILON) {
            motor.setDirection(Direction.STOP);
            motor.setSpeed(0);
            return;
        } else if (axisValue > 0) {
            motor.setDirection(Direction.FORWARD);
        } else if (axisValue < 0) {
            motor.setDirection(Direction.BACKWARD);
        }
        motor.setSpeed((byte) Math.round(motor.getSpeedLimit() * Math.abs(axisValue)));
    }
}
