package edu.columbia.cs.irt.rfidentify.dbhandler;

import java.sql.*;
import java.util.Observable;
import java.util.Observer;

import edu.columbia.cs.irt.rfidentify.server.HttpRequest;

public class DBInitializer{
	public DBInitializer(){
		
	}
	public void load() throws ClassNotFoundException, SQLException{
		Class.forName("org.sqlite.JDBC");
	    Connection conn = DriverManager.getConnection("jdbc:sqlite:rfidentify.db");
	    Statement stat = conn.createStatement();
	    stat.executeUpdate("drop table if exists people;");
	    stat.executeUpdate("create table people (tag, name);");
	    PreparedStatement prep = conn.prepareStatement(
	      "insert into people values (?, ?);");
	
	    prep.setString(1, "9BC3ED1D000007E0");
	    prep.setString(2, "Gandhi");
	    prep.addBatch();
	    prep.setString(1, "238432");
	    prep.setString(2, "Turing");
	    prep.addBatch();
	    prep.setString(1, "23422");
	    prep.setString(2, "Wittgenstein");
	    prep.addBatch();
	
	    conn.setAutoCommit(false);
	    prep.executeBatch();
	    conn.setAutoCommit(true);
	
	    ResultSet rs = stat.executeQuery("select * from people;");
	    while (rs.next()) {
	      System.out.println("tag number = " + rs.getString("tag"));
	      System.out.println("name = " + rs.getString("name"));
	    }
	    rs.close();
	    conn.close();
	}
	


}
