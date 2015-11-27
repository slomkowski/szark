package eu.slomkowski.szark.client;

import eu.slomkowski.szark.client.gui.MainWindowLogic;

import javax.swing.*;

class SzarkClient implements Runnable {

    @Override
    public void run() {
        new MainWindowLogic();
    }

    public static void main(String[] args) {
        SwingUtilities.invokeLater(new SzarkClient());
    }
}
