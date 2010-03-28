package edu.columbia.cs.irt.rfidentify.server;
import java.io.IOException;
import java.net.*;
import java.util.Properties;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import com.apple.dnssd.*;

import edu.columbia.cs.irt.core.PropertyLoader;

public class ZeroConfigRegister implements RegisterListener{

    private Logger logger = LoggerFactory.getLogger(this.getClass());
    private String name;
    private int port;
    private DNSSDRegistration dnss;
    private String service;
    
    
    public ZeroConfigRegister(int port){
    	this.port = port;
    	loadProperties();
    }
	@Override
	public void serviceRegistered(DNSSDRegistration registration, int flags,
			String serviceName, String regType, String domain) {
	    logger.info("Registered Name  : " + serviceName);
	    logger.info("           Type  : " + regType);
	    logger.info("           Domain: " + domain);
		
	}

	@Override
	public void operationFailed(DNSSDService service, int err) {
		logger.error("ZeroConfig registration failed " + err, service);
		
	}


	public void start() {
		logger.info("Registration Starting");
		logger.info("Requested Name: " + name);
		logger.info("          Port: " + port);
		try {
			dnss = DNSSD.register(name, service, port, this);
		} catch (DNSSDException e) {
			logger.error("Could not register ZeroConfig service", e);
		}
	}
	
	public void stop(){
		logger.info("Registration Stopping");
		dnss.stop();
	}
	
	private void loadProperties(){
		try {
			Properties p = PropertyLoader.loadProps("zeroconfig.properties");
			this.name = p.getProperty("name");
			this.service = p.getProperty("service");
			if(name==null){
				name = "rfidentify";
				service = "_rfid-srv._tcp";
			}
		} catch (IOException e) {
			logger.error("Could not load properties", e);
		}
	}

}
