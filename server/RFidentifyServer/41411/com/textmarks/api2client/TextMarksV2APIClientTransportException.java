package com.textmarks.api2client;

/**
 * Exception subclass used by TextMarksV2APIClient for transport-level errors.
 */
public class TextMarksV2APIClientTransportException extends TextMarksV2APIClientException
{
	public TextMarksV2APIClientTransportException(String sMsg) {
		super(sMsg);
	}
	
	public TextMarksV2APIClientTransportException(String sMsg, Throwable cause) {
		super(sMsg, cause);
	}
	
	public TextMarksV2APIClientTransportException(String sMsg, int iCode) {
		super(sMsg);
		this.m_iCode = iCode;
	}
	
	public TextMarksV2APIClientTransportException(String sMsg, int iCode, Throwable cause) {
		super(sMsg, cause);
		this.m_iCode = iCode;
	}
}
