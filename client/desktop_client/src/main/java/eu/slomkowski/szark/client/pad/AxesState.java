package eu.slomkowski.szark.client.pad;

/**
 * Contains states of each axis used by the robot. Values are from -1.0 to 1.0.
 */
public class AxesState {
    private double leftCaterpillar;
    private double rightCaterpillar;

    private double shoulderJoint;
    private double elbowJoint;
    private double gripperJoint;

    public AxesState(double leftCaterpillar, double rightCaterpillar, double shoulderJoint, double elbowJoint, double gripperJoint) {
        this.leftCaterpillar = leftCaterpillar;
        this.rightCaterpillar = rightCaterpillar;
        this.shoulderJoint = shoulderJoint;
        this.elbowJoint = elbowJoint;
        this.gripperJoint = gripperJoint;
    }

    public double getLeftCaterpillar() {
        return leftCaterpillar;
    }

    public double getRightCaterpillar() {
        return rightCaterpillar;
    }

    public double getShoulderJoint() {
        return shoulderJoint;
    }

    public double getElbowJoint() {
        return elbowJoint;
    }

    public double getGripperJoint() {
        return gripperJoint;
    }


    @Override
    public String toString() {
        return String.format("%2.2f %2.2f %2.2f %2.2f %2.2f",
                leftCaterpillar,
                rightCaterpillar,
                shoulderJoint,
                elbowJoint,
                gripperJoint);
    }
}
