package edu.columbia.cs.irt.core;

import java.io.BufferedInputStream;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.IOException;
import java.io.InputStream;
import java.util.Properties;

public class PropertyLoader {
	public static Properties loadProps(String propFileName) throws IOException {
		Properties props = new Properties();
		File f = new File(System.getProperty("user.dir") + File.separator + "properties"
				+ File.separator + propFileName);
		InputStream is = new BufferedInputStream(new FileInputStream(f));
		props.load(is);
		is.close();
		return props;

	}
}
