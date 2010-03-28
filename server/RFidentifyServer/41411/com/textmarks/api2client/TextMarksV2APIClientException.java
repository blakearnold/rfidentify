package com.textmarks.api2client;

/**
 * Exception superclass used by TextMarksAPIClient.
 */
public class TextMarksV2APIClientException extends Exception
{
	static public int ERROR_UNKNOWN		= -1;
	
	// -----------------------------------------------------------------------
	
	public TextMarksV2APIClientException(String sMsg) {
		super(sMsg);
	}
	
	public TextMarksV2APIClientException(String sMsg, Throwable cause) {
		super(sMsg, cause);
	}
	
	public TextMarksV2APIClientException(String sMsg, int iCode) {
		super(sMsg);
		this.m_iCode = iCode;
	}
	
	public TextMarksV2APIClientException(String sMsg, int iCode, Throwable cause) {
		super(sMsg, cause);
		this.m_iCode = iCode;
	}
	
	// -----------------------------------------------------------------------

	/** Get numeric error code. */
	public int getCode() { return this.m_iCode; }

	// -----------------------------------------------------------------------

	/** Numeric error code. */
	protected int m_iCode		= ERROR_UNKNOWN;
}