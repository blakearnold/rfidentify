package edu.columbia.cs.irt.rfidentify;

import java.io.IOException;
import java.sql.DriverManager;
import java.sql.SQLException;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import edu.columbia.cs.irt.rfidentify.dbhandler.DBAccess;
import edu.columbia.cs.irt.rfidentify.dbhandler.DBInitializer;
import edu.columbia.cs.irt.rfidentify.display.DisplayName;
import edu.columbia.cs.irt.rfidentify.display.IdTagHandler;
import edu.columbia.cs.irt.rfidentify.display.TextMarksHandler;
import edu.columbia.cs.irt.rfidentify.registration.RegistrationHandler;
import edu.columbia.cs.irt.rfidentify.server.*;


class Startup {
    static Logger logger = LoggerFactory.getLogger(Startup.class);

    static DBAccess dbAccess;
    static WebServer wServer;
    static RFidentify servlet;

    public static void main(String[] args){
    	initWebServer();
    	initDatabase();
    	initIdTagHandlers();
    	initKiosk();

    
    }
    

    private static void initWebServer(){
    	servlet = new RFidentify();

    	try {
			wServer = new WebServer(servlet);
			new Thread(wServer).start();
		} catch (IOException e) {
			logger.error("Could not bind port " + e.getMessage(), e);
			System.exit(1);
		} catch (NumberFormatException e){
			logger.error("Unable to convert port to int: " + e.getMessage(), e);
			System.exit(1);
		}
    	ZeroConfigRegister zconfig = new ZeroConfigRegister(wServer.getPort());
    	zconfig.start();
    	

    }
    
    private static void initDatabase(){
    	DBInitializer dbinit = new DBInitializer();
    	try {
			dbinit.load();
		} catch (ClassNotFoundException e) {
			logger.error("sqlitejdbc.jar is not in classpath", e);
			System.exit(1);
		} catch (SQLException e) {
			logger.error("Error While connecting to database", e);
			System.exit(1);
		}
		
		try {
			dbAccess = new DBAccess(DriverManager.getConnection("jdbc:sqlite:rfidentify.db"));
	    	servlet.addObserver(dbAccess);
			dbAccess.loadData();
		} catch (SQLException e) {
			logger.error("Error While connecting to database", e);
			System.exit(1);
		}
    	
    }
    
    private static void initIdTagHandlers(){
    	//Handler for idTag updates
    	IdTagHandler idTagHandler = new IdTagHandler(dbAccess);
    	servlet.addObserver(idTagHandler);
    	
    	//Display
    	DisplayName display = new DisplayName("IEEE");
    	idTagHandler.addObserver(display);
    	display.setupDisplay();
    	//Text Service 
    	TextMarksHandler txtMarks = new TextMarksHandler();
    	idTagHandler.addObserver(txtMarks);

    }
    
    private static void initKiosk(){
    	RegistrationHandler rg = new RegistrationHandler(dbAccess);
    	servlet.registerServlet(rg);
    }
}
