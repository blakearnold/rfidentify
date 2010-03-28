package edu.columbia.cs.irt.rfidentify.server;

import java.io.PrintStream;
import java.util.LinkedList;
import java.util.Observable;

public class RFidentify extends Observable implements Servlet, HttpConstants{

	private LinkedList<Servlet> servlets = new LinkedList<Servlet>();
	public RFidentify(){
		
	}
	
	public void registerServlet(Servlet r){
		servlets.add(r);
	}
	
	@Override
	public void doBadMethod(HttpRequest request, PrintStream ps) {

		
	}

	@Override
	public boolean doGet(HttpRequest request, PrintStream response) {
		this.setChanged();
		this.notifyObservers(request);
		for(Servlet r:servlets){
			if(!r.doGet(request, response))
				return false;
		}
		return true;
		
	}

	@Override
	public void doHead(HttpRequest request, PrintStream response) {
		// TODO Auto-generated method stub
		
	}

	@Override
	public boolean doPost(HttpRequest request, PrintStream response) {
		this.setChanged();
		this.notifyObservers(request);
		for(Servlet r:servlets){
			if(!r.doPost(request, response))
				return false;
		}
		return true;
	}

	@Override
	public void doBadMethod(PrintStream ps) {
		// TODO Auto-generated method stub
		
	}

}
