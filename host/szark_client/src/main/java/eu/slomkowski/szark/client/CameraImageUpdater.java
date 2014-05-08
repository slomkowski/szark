package eu.slomkowski.szark.client;
import java.net.*;
import java.util.TimerTask;

import javax.swing.ImageIcon;
import javax.swing.JLabel;

/**
 * This is the TimerTask, which refreshes the image in the camera view panel.
 * @author Michał Słomkowski
 */
public class CameraImageUpdater extends TimerTask
{
	public static enum Camera { HEAD, GRIPPER };

	public synchronized Camera getChoosenCamera()
	{
		return choosenCamera;
	}
	
	Object synchronizator = new Object();

	public void setChoosenCamera(Camera choosenCamera)
	{
		synchronized(synchronizator)
		{
			this.choosenCamera = choosenCamera;
		}
	}

	/**
	 * Constructor takes the reference
	 * @param displayJLabel the JLabel which has to display the image taken from the webcam.
	 * @param hostname the host name or the IP address of the SZARK server
	 */
	CameraImageUpdater(JLabel displayJLabel, String hostname)
	{
		this.displayLabel = displayJLabel;
		this.hostname = hostname;
		
		setChoosenCamera(Camera.HEAD);
	}
	
	/**
	 * Constructor takes the reference
	 * @param displayJLabel the JLabel which has to display the image taken from the webcam.
	 * @param hostname the host name or the IP address of the SZARK server
	 * @param choosenCamera default camera to use
	 */
	CameraImageUpdater(JLabel displayJLabel, String hostname, Camera choosenCamera)
	{
		this(displayJLabel, hostname);
		setChoosenCamera(choosenCamera);
	}
	
	private JLabel displayLabel;
	private ImageIcon cameraImageIcon;
	
	private String hostname;
	private Camera choosenCamera;
	private int frameCounter = 0;
	
	/**
	 * this function is performed by the Timer
	 */
	public void run()
	{
		try
		{
			int camStreamPort;
			
			synchronized(synchronizator)
			{
				if(choosenCamera == Camera.HEAD) camStreamPort = Hardcoded.HEAD_CAMERA_PORT;
				else camStreamPort = Hardcoded.GRIPPER_CAMERA_PORT;
			}
			
			// getting an image from the CamStream server and loading it to JPanel
			cameraImageIcon = new ImageIcon(new URL("http://" + hostname + ":" +
				Integer.toString(camStreamPort) + "/?action=snapshot&n=" + Long.toString(frameCounter)));
				
			displayLabel.setIcon(cameraImageIcon);
			frameCounter++;
		}
		catch(Exception e)
		{
			// it doesn't show anything
			e.printStackTrace();
		}
	}
}