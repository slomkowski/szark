package eu.slomkowski.szark.client;

import net.java.games.input.ControllerEnvironment;
import org.junit.Test;

import static org.hamcrest.MatcherAssert.assertThat;
import static org.hamcrest.Matchers.greaterThan;
import static org.junit.Assert.assertTrue;

public class JinputTest {
    @Test
    public void testLoadNativeLibraries() throws Exception {
        final ControllerEnvironment ce = ControllerEnvironment.getDefaultEnvironment();
        assertTrue(ce.isSupported());
        assertThat(ce.getControllers().length, greaterThan(0));
    }
}
