package edu.columbia.cs.irt.rfidentify.display;

import java.io.FileNotFoundException;
import java.io.IOException;
import java.util.HashMap;
import java.util.Map;
import java.util.Observable;
import java.util.Observer;
import java.util.Properties;

import org.json.JSONObject;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import com.textmarks.api2client.TextMarksV2APIClient;
import com.textmarks.api2client.TextMarksV2APIClientException;

import edu.columbia.cs.irt.core.PropertyLoader;
import edu.columbia.cs.irt.rfidentify.server.HttpRequest;



public class TextMarksHandler implements Observer{
	private String apiKey;
	private String user;
	private String pass;
	private String keyWord;
	Logger logger = LoggerFactory.getLogger(TextMarksHandler.class);
	
	public TextMarksHandler(){
		this.loadProperties();
	}

	public void sendText(String text)	{
		try
		{
			TextMarksV2APIClient tmapi;
			Map<String,Object> msoParams;
			JSONObject joResp; 

			// Broadcast a message to a TextMark group:
			logger.info("Broadcasting a message to a TextMark group..." + text);
			tmapi = new TextMarksV2APIClient();
			msoParams = new HashMap<String, Object>(); 
			tmapi.setApiKey(apiKey);
			tmapi.setAuthUser(user); // (or my TextMarks phone#)
			tmapi.setAuthPass(pass);
			msoParams.put("tm", keyWord);
			msoParams.put("msg", text);
			joResp = tmapi.call("GroupLeader", "broadcast_message", msoParams);
		}
		catch (TextMarksV2APIClientException e) {
			logger.error("Error code: " + e.getCode());
			logger.error("Exception: " + e.toString());
		}
		catch (Exception ex) {
			logger.error("Exception cought:\n"+ ex.toString());
		}
	}
	@Override
	public void update(Observable o, Object req) {
		String request = (String) req;
		
		this.sendText("New Speaker Name: " + request);
		
	}
	
	private void loadProperties(){
		String pName = "textmarks-api.properties";
		Properties props;
		try{
			props = PropertyLoader.loadProps(pName);
			apiKey = props.getProperty("ApiKey");
			user = props.getProperty("AuthUser");
			pass = props.getProperty("AuthPass");
			keyWord = props.getProperty("keyword");
			} catch (FileNotFoundException e) {
				logger.error("File " + pName + " does not exist" + e.getMessage(), e);
			} catch (IOException e) {
				logger.error("Error while loading property file " + pName
						+ " Loading defaults" + e.getMessage(), e);
			}

	}
}
