package eu.slomkowski.szark.client;
import javax.swing.*;


public class SzarkClient implements Runnable
{
	public void run()
	{
		new MainWindow();
	}

	public static void main(String[] args)
	{		
		try
		{
			UIManager.setLookAndFeel("com.sun.java.swing.plaf.gtk.GTKLookAndFeel");
		}
		catch (Exception e)
		{
			//e.printStackTrace();
		}
		
		SwingUtilities.invokeLater(new SzarkClient());
	}
}


