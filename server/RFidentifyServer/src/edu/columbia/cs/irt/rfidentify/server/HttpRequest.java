package edu.columbia.cs.irt.rfidentify.server;

import java.util.HashMap;


public class HttpRequest implements HttpConstants {

	private VERBS verb;
	private String data;
	private String directory;
	private String type;
	public HashMap<String, String> getForm() {
		return form;
	}

	public void setForm(HashMap<String, String> form) {
		this.form = form;
	}

	private String name;
	private HashMap<String, String> form;
	
	public HttpRequest(){
		form = new HashMap<String, String>();
	}

	public String getData() {
		return data;
	}

	public void setData(String data) {
		this.data = data;
	}

	public String getDirectory() {
		return directory;
	}

	public void setDirectory(String directory) {
		this.directory = directory;
	}

	public String getType() {
		return type;
	}

	public void setType(String type) {
		this.type = type;
	}

	public String getName() {
		return name;
	}

	public void setName(String name) {
		this.name = name;
	}

	public void setVerb(VERBS verb) {
		this.verb = verb;
	}

	public VERBS getVerb() {
		return verb;
	}
}
