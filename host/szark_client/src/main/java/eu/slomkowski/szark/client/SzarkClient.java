package eu.slomkowski.szark.client;

import eu.slomkowski.szark.client.gui.MainWindowLogic;

import javax.swing.*;

class SzarkClient implements Runnable {

	@Override
	public void run() {
		new MainWindowLogic();
	}

	public static void main(String[] args) {
		try {
			UIManager.setLookAndFeel("com.sun.java.swing.plaf.gtk.GTKLookAndFeel");
		} catch (final Exception e) {
			// e.printStackTrace();
		}

		SwingUtilities.invokeLater(new SzarkClient());
	}
}
