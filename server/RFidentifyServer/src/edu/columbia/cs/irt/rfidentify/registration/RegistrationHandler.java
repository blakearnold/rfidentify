package edu.columbia.cs.irt.rfidentify.registration;

import java.io.IOException;
import java.io.PrintStream;
import java.sql.SQLException;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import edu.columbia.cs.irt.rfidentify.dbhandler.DBAccess;
import edu.columbia.cs.irt.rfidentify.server.HttpConstants;
import edu.columbia.cs.irt.rfidentify.server.HttpRequest;
import edu.columbia.cs.irt.rfidentify.server.Servlet;


public class RegistrationHandler implements Servlet, HttpConstants {
	private Logger logger = LoggerFactory.getLogger(RegistrationHandler.class);
	private DBAccess dbAccess;
	private String lastSwipe;
	public RegistrationHandler(DBAccess dbAccess){
		this.dbAccess = dbAccess;
	}

	@Override
	public void doBadMethod(PrintStream ps) {
		// TODO Auto-generated method stub
		
	}
	@Override
	public void doBadMethod(HttpRequest request, PrintStream ps) {
		// TODO Auto-generated method stub
		
	}
	@Override
	public boolean doPost(HttpRequest request, PrintStream response) {
		logger.info("received post");
		if (request.getType().equals("kiosk")) {
			logger.info("received post");
			String idTagNum = request.getForm().get("tag");
			String name = request.getForm().get("name");
			if(idTagNum!=null && name != null){
				logger.info("new id tag from kiosk " + idTagNum + " " + name);
				//register
				try {
					dbAccess.addName(idTagNum, name);
				} catch (SQLException e) {
					logger.error("problem registering tag" + e.getMessage(), e);
				}
			}
		}

		try {
			response.write(EOL);
			response.print("<p>Thank you for registering</p>");
			response.write(EOL);
		} catch (IOException e) {
			logger.error("Error sending thank you page", e);
		}
		return true;
		
	}
	@Override
	public void doHead(HttpRequest request, PrintStream response) {
		// TODO Auto-generated method stub
		
	}
	@Override
	public boolean doGet(HttpRequest request, PrintStream response) {
		String name = request.getName();
		if (name != null && name.equals("kiosk")) {
			String idTagNum = request.getForm().get("idTag");
			
			//Receiving new idTag
			if(idTagNum!=null){
				lastSwipe = idTagNum;
				logger.info("kiosk swiped idtag: " + idTagNum);
				return true;
			}  
			
			//Receiving AJAX request
			if (request.getForm().get("type") != null){
		        try {
					response.write(EOL);
					response.println(lastSwipe);
					response.write(EOL);
					response.close();
					return false;
				} catch (IOException e) {
					logger.error("Error sending lastswipe to kiosk", e);
				}
				
			}
		}
		return true;
		
	}
}
