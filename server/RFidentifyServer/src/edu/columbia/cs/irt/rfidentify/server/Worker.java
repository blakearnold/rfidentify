package edu.columbia.cs.irt.rfidentify.server;

import java.io.BufferedInputStream;
import java.io.File;
import java.io.FileInputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.PrintStream;
import java.net.Socket;
import java.util.Date;
import java.util.Observable;
import java.util.Vector;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

public class Worker implements HttpConstants, Runnable {
	private Servlet servlet;
	final int BUF_SIZE = 2048;

	/* buffer to use for requests */
	byte[] buf;
	private HttpRequest request;
	/* Socket to client we're handling */
	private Socket s;

	private Vector<Worker> pool;
	private int workers;
	private int timeout;
	private File root;
	private Logger logger = LoggerFactory.getLogger(this.getClass());

	public Worker(Vector<Worker> pool, int workers, int timeout, File root,
			Servlet servlet) {
		this.pool = pool;
		this.workers = workers;
		this.timeout = timeout;
		this.root = root;
		this.servlet = servlet;

		buf = new byte[BUF_SIZE];
		/* zero out the buffer from last time */
		for (int i = 0; i < BUF_SIZE; i++) {
			buf[i] = 0;
		}
		s = null;
	}

	synchronized void setSocket(Socket s) {
		this.s = s;
		notify();
	}

	public synchronized void run() {
		while (true) {
			if (s == null) {
				/* nothing to do */
				try {
					wait();
				} catch (InterruptedException e) {
					/* should not happen */
					continue;
				}
			}
			try {
				handleClient();
			} catch (Exception e) {
				logger.error("Error handling request", e);
			}
			/*
			 * go back in wait queue if there's fewer than numHandler
			 * connections.
			 */
			s = null;
			synchronized (pool) {
				if (pool.size() >= workers) {
					/* too many threads, exit this one */
					return;
				}
				pool.addElement(this);
			}
		}
	}

	private void handleClient() throws IOException {
		InputStream is = new BufferedInputStream(s.getInputStream());
		PrintStream ps = new PrintStream(s.getOutputStream());
		/*
		 * we will only block in read for this many milliseconds before we fail
		 * with java.io.InterruptedIOException, at which point we will abandon
		 * the connection.
		 */
		s.setSoTimeout(timeout);
		s.setTcpNoDelay(true);

		try {
			int nread = 0, r = 0;

			outerloop: while (nread < BUF_SIZE) {
				r = is.read(buf, nread, BUF_SIZE - nread);
				if (r == -1) {
					/* EOF */
					return;
				}
				int i = nread;
				nread += r;
				// TODO:fix check too blank line
				for (; i < nread; i++) {
					if (buf[i] == (byte) '\n' || buf[i] == (byte) '\r') {
						/* read one line */
						break outerloop;
					}
				}
			}

			String requestString = new String(buf, 0, nread, "ISO-8859-1");
			logger.info("read request\n" + requestString);
			try {
				request = parseRequest(requestString);
			} catch (IllegalArgumentException e) {
				logger.error("Bad verb " + e.getMessage());
				s.close();
				return;
			}

			boolean doingGet = false;
			switch (request.getVerb()) {
			case HEAD:
				servlet.doHead(request, ps);
				doingGet = false;
				break;
			case GET:
				doingGet = true;
				break;
			case POST:
				// read body and parse

				break;
			default:
				/* we don't support this method */
				ps.print("HTTP/1.0 " + HTTP_BAD_METHOD
						+ " unsupported method type: " + request.getVerb());
				ps.write(EOL);
				ps.flush();
				s.close();
				return;

			}

			String fname = request.getDirectory().replace('/',
					File.separatorChar);
			if (fname.startsWith(File.separator)) {
				fname = fname.substring(File.separator.length());
			}

			File targ = new File(root, fname);
			if (targ.isDirectory()) {
				File ind = new File(targ, "index.html");
				if (ind.exists()) {
					targ = ind;
				}
			}
			boolean OK = printHeaders(targ, ps);
			if (doingGet) {
				if (servlet.doGet(request, ps))
					if (OK) {
						sendFile(targ, ps);
					} else {
						send404(targ, ps);
					}
			}
			if (request.getVerb().equals(VERBS.POST))
				servlet.doPost(request, ps);
		} finally {
			s.close();
		}

		/* zero out the buffer from last time */
		for (int i = 0; i < BUF_SIZE; i++) {
			buf[i] = 0;
		}
	}

	public HttpRequest parseRequest(String request) {
		HttpRequest requestStruct = new HttpRequest();
		String[] flSplit = request.split(" ");
		requestStruct.setVerb(VERBS.valueOf(flSplit[0]));

		requestStruct.setDirectory(flSplit[1]);
		String[] uriSplit = flSplit[1].split("/");
		if (uriSplit.length >= 2) {
			requestStruct.setType(uriSplit.length >= 2 ? uriSplit[1] : null);
			requestStruct.setName(uriSplit.length > 3 ? uriSplit[2] : null);
		}
		// Split up last part of URI for ?, & and =
		String[] nameSplit = uriSplit[uriSplit.length - 1].split("\\?");
		// if last part of URI is the 2nd arg then set name to the arg before
		// the '?'
		requestStruct.setName(uriSplit.length == 3 ? nameSplit[0]
				: requestStruct.getName());

		String[] formClump = nameSplit.length >= 2 ? nameSplit[1].split("&")
				: new String[] { " = " };

		if(requestStruct.getVerb().equals(VERBS.POST)){
			flSplit = request.split("\r\n\r\n");
			if(flSplit.length<2)
				flSplit = request.split("\n\n");
			formClump = flSplit.length >= 2 ? flSplit[1].split("&")
					: new String[] { " = " };
		}
		for (String keyValue : formClump) {
			String[] keyValueClump = keyValue.split("=");
			if (keyValueClump.length >= 2) {
				logger.info("Form Data: " + keyValueClump[0] + "="
						+ keyValueClump[1]);
				keyValueClump[1]=keyValueClump[1].replace('+', ' ');
				requestStruct.getForm().put(keyValueClump[0], keyValueClump[1]);
			}

		}

		return requestStruct;
	}

	boolean printHeaders(File targ, PrintStream ps) throws IOException {
		boolean ret = false;
		int rCode = 0;
		if (!targ.exists()) {
			rCode = HTTP_NOT_FOUND;
			ps.print("HTTP/1.0 " + HTTP_NOT_FOUND + " not found");
			ps.write(EOL);
			ret = false;
		} else {
			rCode = HTTP_OK;
			ps.print("HTTP/1.0 " + HTTP_OK + " OK");
			ps.write(EOL);
			ret = true;
		}
		logger.info("From " + s.getInetAddress().getHostAddress() + ": GET "
				+ targ.getAbsolutePath() + "-->" + rCode);
		ps.print("Server: RFidentify");
		ps.write(EOL);
		ps.print("Date: " + (new Date()));
		ps.write(EOL);
		if (ret) {
			if (!targ.isDirectory()) {
				ps.print("Content-length: " + targ.length());
				ps.write(EOL);
				ps.print("Last Modified: " + (new Date(targ.lastModified())));
				ps.write(EOL);
				String name = targ.getName();
				int ind = name.lastIndexOf('.');
				String ct = null;
				if (ind > 0) {
					ct = map.get(name.substring(ind));
				}
				if (ct == null) {
					ct = "unknown/unknown";
				}
				ps.print("Content-type: " + ct);
				ps.write(EOL);
			} else {
				ps.print("Content-type: text/html");
				ps.write(EOL);
			}
		}
		return ret;
	}

	void send404(File targ, PrintStream ps) throws IOException {
		ps.write(EOL);
		ps.write(EOL);
		ps.println("Not Found\n\n" + "The requested resource was not found.\n");
	}

	void sendFile(File targ, PrintStream ps) throws IOException {
		InputStream is = null;
		ps.write(EOL);
		if (targ.isDirectory()) {
			listDirectory(targ, ps);
			return;
		}
		is = new FileInputStream(targ.getAbsolutePath());

		try {
			int n;
			while ((n = is.read(buf)) > 0) {
				ps.write(buf, 0, n);
			}
		} finally {
			is.close();
		}
	}

	/* mapping of file extensions to content-types */
	static java.util.Hashtable<String, String> map = new java.util.Hashtable<String, String>();

	static {
		fillMap();
	}

	static void setSuffix(String k, String v) {
		map.put(k, v);
	}

	static void fillMap() {
		setSuffix("", "content/unknown");
		setSuffix(".uu", "application/octet-stream");
		setSuffix(".exe", "application/octet-stream");
		setSuffix(".ps", "application/postscript");
		setSuffix(".zip", "application/zip");
		setSuffix(".sh", "application/x-shar");
		setSuffix(".tar", "application/x-tar");
		setSuffix(".snd", "audio/basic");
		setSuffix(".au", "audio/basic");
		setSuffix(".wav", "audio/x-wav");
		setSuffix(".gif", "image/gif");
		setSuffix(".jpg", "image/jpeg");
		setSuffix(".jpeg", "image/jpeg");
		setSuffix(".htm", "text/html");
		setSuffix(".html", "text/html");
		setSuffix(".text", "text/plain");
		setSuffix(".c", "text/plain");
		setSuffix(".cc", "text/plain");
		setSuffix(".c++", "text/plain");
		setSuffix(".h", "text/plain");
		setSuffix(".pl", "text/plain");
		setSuffix(".txt", "text/plain");
		setSuffix(".java", "text/plain");
	}

	void listDirectory(File dir, PrintStream ps) {
		ps.println("<TITLE>Directory listing</TITLE><P>\n");
		ps.println("<A HREF=\"..\">Parent Directory</A><BR>\n");
		String[] list = dir.list();
		for (int i = 0; list != null && i < list.length; i++) {
			File f = new File(dir, list[i]);
			if (f.isDirectory()) {
				ps.println("<A HREF=\"" + list[i] + "/\">" + list[i]
						+ "/</A><BR>");
			} else {
				ps
						.println("<A HREF=\"" + list[i] + "\">" + list[i]
								+ "</A><BR");
			}
		}
		ps.println("<P><HR><BR><I>" + (new Date()) + "</I>");
	}

}