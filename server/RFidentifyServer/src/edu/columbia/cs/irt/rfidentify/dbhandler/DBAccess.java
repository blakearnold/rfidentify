package edu.columbia.cs.irt.rfidentify.dbhandler;

import java.sql.Connection;
import java.sql.PreparedStatement;
import java.sql.ResultSet;
import java.sql.SQLException;
import java.sql.Statement;
import java.util.HashMap;
import java.util.Observable;
import java.util.Observer;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;
import edu.columbia.cs.irt.rfidentify.server.HttpRequest;

public class DBAccess implements Observer{

	private Logger logger = LoggerFactory.getLogger(DBAccess.class);

	private HashMap<String, String> tagName;
	private Connection conn;
	public DBAccess(Connection conn){
		tagName = new HashMap<String, String>();
		this.conn = conn;
	}
	
	public String getName(String idTag){
		return tagName.get(idTag);
	}
	
	public void addName(String tag, String name) throws SQLException{
		tagName.put(tag, name);
		PreparedStatement prep = conn.prepareStatement(
	      "insert into people values (?, ?);");
	
	    prep.setString(1, "tag");
	    prep.setString(2, "name");
	    prep.execute();
	}
	
	public void loadData() throws SQLException{
		Statement stat = conn.createStatement();
	    ResultSet rs = stat.executeQuery("select * from people;");
	    while (rs.next()) {
	      tagName.put(rs.getString("tag"), rs.getString("name"));
	    }
	    rs.close();
	}

	@Override
	public void update(Observable o, Object req) {
		HttpRequest request = (HttpRequest) req;
		if(request.getType() != null && request.getName() != null)
			if(request.getType().equals("register") && request.getName().equals("kiosk")){
				//TODO: put name in database
				//String content[] = request.getDirectory().split("?");
			}
	}
}
