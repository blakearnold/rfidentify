package edu.columbia.cs.irt.rfidentify.display;

import java.awt.BorderLayout;
import java.awt.Button;
import java.awt.Color;
import java.awt.Font;
import java.awt.GraphicsDevice;
import java.awt.GraphicsEnvironment;
import java.awt.TextArea;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.awt.event.WindowEvent;
import java.awt.event.WindowListener;
import java.awt.event.WindowStateListener;
import java.util.Observable;
import java.util.Observer;

import javax.swing.JButton;
import javax.swing.JFrame;
import javax.swing.JLabel;
import javax.swing.JPanel;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import edu.columbia.cs.irt.rfidentify.dbhandler.DBAccess;
import edu.columbia.cs.irt.rfidentify.server.HttpRequest;

public class DisplayName extends JFrame implements Observer {


	private Logger logger = LoggerFactory.getLogger(DisplayName.class);

	private JLabel name;
	
	public DisplayName(String displayName){
		super(displayName);
		this.name = new JLabel("Waiting for request", JLabel.CENTER);
	}

	public void setupDisplay() {
		
		this.setLayout(new BorderLayout());

		// Determine if full-screen mode is supported directly
		GraphicsEnvironment ge = GraphicsEnvironment.getLocalGraphicsEnvironment();
		final GraphicsDevice gs = ge.getDefaultScreenDevice();
		if (gs.isFullScreenSupported()) {
		    // Full-screen mode is supported
		} else {
		    // Full-screen mode will be simulated
		}

		final JFrame self = this;
		// Create a button that leaves full-screen mode
		JButton btn = new JButton();
		btn.setSize(100, 50);
		btn.setBorderPainted(false);
		btn.addActionListener(new ActionListener() {
			@Override
		    public void actionPerformed(ActionEvent evt) {
		        // Return to normal windowed mode
		        GraphicsEnvironment ge = GraphicsEnvironment.getLocalGraphicsEnvironment();
		        GraphicsDevice gs = ge.getDefaultScreenDevice();
		        if(gs.getFullScreenWindow() == self)
		        	gs.setFullScreenWindow(null);
		        else
		        	gs.setFullScreenWindow(self);

		    }

		});

		this.setExtendedState(MAXIMIZED_BOTH);

		getContentPane().add(btn, BorderLayout.CENTER);


		this.setDefaultCloseOperation(JFrame.EXIT_ON_CLOSE);

		Font font = new Font("Arial Bold", Font.ITALIC, 64);
		name.setFont(font);
		name.setForeground(Color.white);
		this.setBackground(Color.black);
		getContentPane().add(name, BorderLayout.CENTER);
		// Display the window.
		this.setUndecorated(true);
		this.pack();
		this.setVisible(true);

		
	    gs.setFullScreenWindow(this);




	}
	
	public void maximize(){
		this.setResizable(false);
		this.setSize(java.awt.Toolkit.getDefaultToolkit().getScreenSize());
	}

	@Override
	public void update(Observable o, Object req) {

		name.setText((String)req);
		repaint();

	}
}
