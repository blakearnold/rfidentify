package edu.columbia.cs.irt.rfidentify.server;

import java.io.PrintStream;

public interface Servlet {

	public boolean doGet(HttpRequest request, PrintStream response);
	
	public boolean doPost(HttpRequest request, PrintStream response);
	
	public void doHead(HttpRequest request, PrintStream response);

	public void doBadMethod(PrintStream ps);

	void doBadMethod(HttpRequest request, PrintStream ps);
	

}
