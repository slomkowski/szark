package eu.slomkowski.szark.client;

import eu.slomkowski.szark.client.gui.MainWindowLogic;
import org.pushingpixels.substance.api.skin.SubstanceTwilightLookAndFeel;

import javax.swing.*;

class SzarkClient implements Runnable {

    @Override
    public void run() {
        try {
            UIManager.setLookAndFeel(new SubstanceTwilightLookAndFeel());
        } catch (UnsupportedLookAndFeelException e) {
            e.printStackTrace();
        }

        new MainWindowLogic();
    }

    public static void main(String[] args) {
        SwingUtilities.invokeLater(new SzarkClient());
    }
}
