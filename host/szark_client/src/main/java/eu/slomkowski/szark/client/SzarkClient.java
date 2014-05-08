package eu.slomkowski.szark.client;

import javax.swing.SwingUtilities;
import javax.swing.UIManager;

public class SzarkClient implements Runnable {

	@Override
	public void run() {
		new MainWindow();
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
