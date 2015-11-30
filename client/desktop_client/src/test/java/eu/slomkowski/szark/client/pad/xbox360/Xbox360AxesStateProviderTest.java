package eu.slomkowski.szark.client.pad.xbox360;

import com.google.common.collect.ImmutableList;
import eu.slomkowski.szark.client.pad.AxesState;
import eu.slomkowski.szark.client.pad.PadException;
import org.junit.Before;
import org.junit.Test;

import java.util.List;

import static org.hamcrest.Matchers.*;
import static org.junit.Assert.assertNotNull;
import static org.junit.Assert.assertThat;

public class Xbox360AxesStateProviderTest {
    Xbox360AxesStateProvider provider;

    @Before
    public void initProvider() throws PadException {
        provider = new Xbox360AxesStateProvider();
    }

    @Test
    public void testAxesReading() throws InterruptedException, PadException {
        for (int i = 0; i < 20; ++i) {
            AxesState axesState = provider.getAxesState();

            assertNotNull(axesState);

            List<Double> l = ImmutableList.of(
                    axesState.getLeftCaterpillar(),
                    axesState.getRightCaterpillar(),
                    axesState.getShoulderJoint(),
                    axesState.getElbowJoint(),
                    axesState.getGripperJoint()
            );

            l.forEach(v -> assertThat(v, allOf(greaterThanOrEqualTo(-1.0), lessThanOrEqualTo(1.0))));

            Thread.sleep(20);
        }
    }
}