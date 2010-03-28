package com.textmarks.api2client;

/**
 * Exception subclass used by TextMarksV2APIClient for non-success result codes.
 */
public class TextMarksV2APIClientResultException extends TextMarksV2APIClientException
{
	public TextMarksV2APIClientResultException(String sMsg) {
		super(sMsg);
	}
	
	public TextMarksV2APIClientResultException(String sMsg, Throwable cause) {
		super(sMsg, cause);
	}
	
	public TextMarksV2APIClientResultException(String sMsg, int iCode) {
		super(sMsg);
		this.m_iCode = iCode;
	}
	
	public TextMarksV2APIClientResultException(String sMsg, int iCode, Throwable cause) {
		super(sMsg, cause);
		this.m_iCode = iCode;
	}
}
