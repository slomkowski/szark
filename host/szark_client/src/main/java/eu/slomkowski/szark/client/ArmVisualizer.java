package eu.slomkowski.szark.client;
import java.awt.*;
import java.awt.geom.AffineTransform;

import javax.swing.JPanel;
import javax.swing.text.Position;

public class ArmVisualizer extends JPanel
{
	private static final long serialVersionUID = 1L;
	
	private SzarkStatus status;
	
	public ArmVisualizer(SzarkStatus status)
	{
		this();
		
		this.status = status;
	}
	
	public void setUpdateStatus(SzarkStatus status)
	{
		this.status = status;
		repaint();
	}
	
	public ArmVisualizer()
	{
		setPreferredSize(new Dimension(250, 250));
		setEnabled(false);
	}
	
	public void setEnabled(boolean enable)
	{
		if(status == null) return;
		
		super.setEnabled(enable);
		repaint();
	}

	public void paintComponent(Graphics graphics)
	{
		super.paintComponent(graphics);
		//if(!isEnabled() || (status == null)) return;
		
		Graphics2D g = (Graphics2D) graphics;
		g.setRenderingHint(RenderingHints.KEY_ANTIALIASING,
				RenderingHints.VALUE_ANTIALIAS_ON);
		g.setRenderingHint(RenderingHints.KEY_TEXT_ANTIALIASING,
				RenderingHints.VALUE_TEXT_ANTIALIAS_ON);
	
		AffineTransform afSave = g.getTransform();
		
		Dimension lo = new Dimension(getWidth() - 10, getHeight() - 23);
		Dimension up = new Dimension(5, 15);
	
		g.setColor(Color.black);
		
		//g.fillRect(34, 34, 55, 33);
		//g.fillRect(up.width, up.height, lo.width, lo.height);	
		g.fillArc(lo.width / 2 - 25, lo.height - 5, 50, 50, 0, 180);
		
		AffineTransform at = new AffineTransform();
		
		Rectangle p = new Rectangle(lo.width / 2, lo.height, 5, 80);
		
		
		
		g.draw(p);
		
		g.setTransform(afSave);
	}
}
