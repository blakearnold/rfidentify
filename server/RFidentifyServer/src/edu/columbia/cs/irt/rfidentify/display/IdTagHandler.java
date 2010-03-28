package edu.columbia.cs.irt.rfidentify.display;

import java.util.Observable;
import java.util.Observer;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import edu.columbia.cs.irt.rfidentify.dbhandler.DBAccess;
import edu.columbia.cs.irt.rfidentify.server.HttpRequest;

public class IdTagHandler extends Observable implements Observer {

	private DBAccess dbAccess;
	private Logger logger = LoggerFactory.getLogger(IdTagHandler.class);

	public IdTagHandler(DBAccess dbAccess) {
		this.dbAccess = dbAccess;
	}

	@Override
	public void update(Observable o, Object req) {
		HttpRequest request = (HttpRequest) req;
		if (request.getType() != null && request.getName() != null)
			if (request.getType().equals("dev")) {
				
				//Gumstix
				if (request.getName().equals("gumstix")) {
					String idTagNum = request.getForm().get("idTag");
					String idName = null;
					if (idTagNum != null)
						idName = dbAccess.getName(idTagNum);

					if (idName != null) {
						logger.info("name found for idtag: " + idTagNum + " "
								+ idName);
						this.setChanged();
						this.notifyObservers(idName);
					} else {
						logger.info("no name found for idtag: " + idTagNum);
					}
				}
				

			}

	}

}
