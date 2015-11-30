package eu.slomkowski.szark.client.pad.xbox360;

import eu.slomkowski.szark.client.pad.AxesState;
import eu.slomkowski.szark.client.pad.AxesStateProvider;
import eu.slomkowski.szark.client.pad.PadException;
import net.java.games.input.Component;
import net.java.games.input.Controller;
import net.java.games.input.ControllerEnvironment;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import java.util.Arrays;
import java.util.List;
import java.util.Optional;
import java.util.stream.Collectors;
import java.util.stream.Stream;

public class Xbox360AxesStateProvider implements AxesStateProvider {

    private Logger logger = LoggerFactory.getLogger(Xbox360AxesStateProvider.class);

    private static final String CONTROLLER_NAME = "Microsoft X-Box 360 pad";

    private Axis leftCaterpillarAccelerator = new Axis("z", 1.0);
    private Axis leftCaterpillarBackwardButton = new Axis("Left Thumb", 1.0);

    private Axis rightCaterpillarAccelerator = new Axis("rz", 1.0);
    private Axis rightCaterpillarBackwardButton = new Axis("Right Thumb", 1.0);

    private Axis shoulderJointAccelerator = new Axis("y", -1.0);
    private Axis elbowJointAccelerator = new Axis("ry", -1.0);
    private Axis gripperJointAccelerator = new Axis("rx", 1.0);

    //A, B, X, Y, Left Thumb, Right Thumb, Select, Start, Mode, Left Thumb 3, Right Thumb 3, x, y, z, rx, ry, rz, pov

    private Controller controller;

    public Xbox360AxesStateProvider() throws PadException {
        controller = getController();

        initComponentInAxis(leftCaterpillarBackwardButton);
        initComponentInAxis(leftCaterpillarAccelerator);

        initComponentInAxis(rightCaterpillarAccelerator);
        initComponentInAxis(rightCaterpillarBackwardButton);

        initComponentInAxis(shoulderJointAccelerator);
        initComponentInAxis(elbowJointAccelerator);
        initComponentInAxis(gripperJointAccelerator);
    }

    @Override
    public AxesState getAxesState() throws PadException {
        controller.poll();

        AxesState axesState = new AxesState(
                getCaterpillarAxisState(leftCaterpillarAccelerator, leftCaterpillarBackwardButton),
                getCaterpillarAxisState(rightCaterpillarAccelerator, rightCaterpillarBackwardButton),
                getJointAxisState(shoulderJointAccelerator),
                getJointAxisState(elbowJointAccelerator),
                getJointAxisState(gripperJointAccelerator));

        logger.debug("Axes state: {}.", axesState.toString());

        return axesState;
    }

    private double getCaterpillarAxisState(Axis accelerator, Axis backwardButton) {
        double val = (accelerator.getValue() + 1.0) / 2.0;
        return backwardButton.getValue() < 0.5 ? val : -val;
    }

    private double getJointAxisState(Axis axis) {
        double val = axis.getValue();
        return Math.abs(val) > 0.20 ? val : 0; // joystick doesn't centers ideally to zero
    }

    private void initComponentInAxis(Axis axis) throws PadException {
        Optional<Component> component = Arrays.stream(controller.getComponents()).filter(c -> c.getName().equals(axis.getName())).findFirst();

        if (!component.isPresent()) {
            throw new PadException("component for axis '" + axis + "' not found");
        } else {
            axis.setComponent(component.get());
        }
    }

    private Controller getController() throws PadException {
        final ControllerEnvironment ce = ControllerEnvironment.getDefaultEnvironment();

        List<Controller> controllers = Arrays.stream(ce.getControllers()).filter(
                e -> (e.getType() == Controller.Type.GAMEPAD) || (e.getType() == Controller.Type.STICK))
                .collect(Collectors.toList());

        controllers.forEach(c -> logger.info("Found game controller: '{}'.", c.getName()));

        Optional<Controller> controller = controllers.stream().filter(c -> c.getName().equals(CONTROLLER_NAME)).findFirst();

        if (!controller.isPresent()) {
            throw new PadException("controller not found");
        }

        Stream<Component> components = Arrays.stream(controller.get().getComponents());

        logger.info("Following components are available: " + components.map(Component::getName).reduce((u, t) -> u + ", " + t).get());

        return controller.get();
    }

}

/**
 * Class to store an axis description.
 */
class Axis {

    /**
     * @param name    String name of specific axis, taken from
     *                Component.getName().
     * @param scaling Scaling factor of the value read from the axis.
     *                Negative values invert the axis.
     */
    public Axis(String name, double scaling) {
        this.name = name;
        this.scaling = scaling;
    }

    private final String name;
    private double scaling;
    private Component component;

    public double getScaling() {
        return scaling;
    }

    public String getName() {
        return name;
    }

    public double getValue() {
        return component.getPollData() * scaling;
    }

    public Component getComponent() {
        return component;
    }

    public void setComponent(Component component) {
        this.component = component;
    }

    @Override
    public String toString() {
        return getName();
    }
}