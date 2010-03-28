package edu.columbia.cs.irt.rfidentify.server;

/*
 * Created by http://java.sun.com/developer/technicalArticles/Networking/Webserver/WebServer.java
 * Edited by blake arnold
 */
/* An example of a very simple, multi-threaded HTTP server.
 * Implementation notes are in WebServer.html, and also
 * as comments in the source code.
 */
import java.io.*;
import java.net.*;
import java.util.*;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import edu.columbia.cs.irt.core.PropertyLoader;

public class WebServer implements Runnable {

	/* static class data/methods */

	/*
	 * our server's configuration information is stored in these properties
	 */
	protected Properties props = new Properties();

	/* Where worker threads stand idle */
	private Vector<Worker> threads = new Vector<Worker>();

	/* the web server's virtual root */
	private File root;

	/* timeout on client connections */
	private int timeout = 0;

	/* max # worker threads */
	private int workers = 5;

	private int port;
	private ServerSocket ss;
	private Servlet servlet;

	Logger logger = LoggerFactory.getLogger(this.getClass());

	public WebServer(Servlet servlet) throws IOException {

		loadProps();
		printProps();

		this.servlet = servlet;
		ss = new ServerSocket(port);

	}

	@Override
	public void run() {
		/* start worker threads */
		for (int i = 0; i < workers; ++i) {
			Worker w = new Worker(threads, workers, timeout, root, servlet);
			(new Thread(w, "worker #" + i)).start();
			threads.addElement(w);
		}

		Socket s;

		while (true) {
			try {
				s = ss.accept();
				Worker w = null;
				synchronized (threads) {
					if (threads.isEmpty()) {
						Worker ws = new Worker(threads, workers, timeout, root,
								servlet);
						ws.setSocket(s);
						(new Thread(ws, "rfidserverWorker")).start();
					} else {
						w = threads.elementAt(0);
						threads.removeElementAt(0);
						w.setSocket(s);
					}
				}

			} catch (IOException e) {
				logger
						.error("Exception thrown while waiting for connection",
								e);
			}
		}

	}

	/* load www-server.properties from java.home */
	private void loadProps() {
		String pName = "www-server.properties";
		try {
			Properties props = PropertyLoader.loadProps(pName);
			String p = props.getProperty("port");

			if (p != null) {
				port = new Integer(p);
			} else {
				logger.error("no property \"port\"");
			}

			String r = props.getProperty("root");
			if (r != null) {
				root = new File(r);
				if (!root.exists()) {
					logger.error(root + " doesn't exist as server root");
				}
			} else {
				logger.error("no property \"root\"");
				root = new File(System.getProperty("user.dir"));
			}

			r = props.getProperty("timeout");
			if (r != null) {
				try {
					timeout = Integer.parseInt(r);
				} catch (NumberFormatException e) {
					logger.error("Could not convert " + timeout
							+ "to an number", e);
					timeout = 5000;
				}
			} else {
				logger.error("no property \"timeout\"");
			}

			r = props.getProperty("workers");
			if (r != null) {
				try {
					workers = Integer.parseInt(r);
				} catch (NumberFormatException e) {
					logger.error("Could not convert " + timeout
							+ "to an number", e);
					workers = 5;
				}

			} else {
				logger.error("no property \"workers\"");
			}

			/* sanity check info */

			if (port < 1024 || port > 60000)
				port = 8080;

			if (timeout <= 1000) {
				timeout = 5000;
			}
			if (workers > 25 || workers < 1) {
				workers = 5;
			}

		} catch (FileNotFoundException e) {
			logger.error("File " + pName + " does not exist " + e.getMessage(),
					e);
		} catch (IOException e) {
			logger.error("Error while loading property file " + pName
					+ " Loading defaults" + e.getMessage(), e);
		}

	}

	void printProps() {

		logger.info("port=" + port);
		logger.info("root=" + root);
		logger.info("timeout=" + timeout);
		logger.info("workers=" + workers);
	}

	public int getPort() {
		return port;
	}

}
