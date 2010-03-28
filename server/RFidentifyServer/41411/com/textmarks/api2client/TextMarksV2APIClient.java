package com.textmarks.api2client;

import java.util.*;
import java.io.*;
import java.net.*;
import org.json.*;


/**
 * TextMarks V2 API Client Library (Java). v2.60d.
 * <hr />
 *
 * <p>TextMarks provides a text-messaging platform you can integrate into
 * your own applications to send and receive text messages to individual
 * users or groups of users.
 * </p>
 * <p>For full online documentation, visit:<ul>
 *   <li>http://www.textmarks.com/dev/docs/api2/</li>
 *   <li>http://www.textmarks.com/dev/</li>
 *   <li>http://www.textmarks.com/</li>
 * </ul>
 * </p>
 * <p>The HTTP API that this library integrates with is NOT REQUIRED.
 * You can do all kinds of wonderful things without this API and without
 * writing any code at all.  However if you wish to automate and integrate
 * TextMarks more deeply into your applications, this API may be useful.
 * </p>
 * <p>This optional Java client library provides one way to integrate with
 * the platform's HTTP API from your PHP applications.
 * </p>
 * <p><b>This library requires</b>:<ul>
 *  <li>Java 1.4 or higher.</li>
 *  <li>org.json package from http://www.json.org/java/json.zip (see http://www.json.org/java/)</li>
 * </ul>
 * </p>
 *
 * <hr />
 * Author: Dan Kamins [d k a m i n s A.T t e x t m a r k s D.O.T c o m]
 * <hr />
 * Copyright (c) 2009, TextMarks Inc. All rights reserved.
 * <hr />
 *
 * <p>THIS PROGRAM IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OR
 * CONDITIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED INCLUDING, WITHOUT
 * LIMITATION, ANY WARRANTIES OR CONDITIONS OF TITLE, NON-INFRINGEMENT,
 * MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE.
 * </p>
 * <p>RECIPIENT IS SOLELY RESPONSIBLE FOR DETERMINING THE APPROPRIATENESS
 * OF USING AND DISTRIBUTING THE PROGRAM AND ASSUMES ALL RISKS ASSOCIATED
 * WITH ITS EXERCISE OF RIGHTS UNDER THIS AGREEMENT, INCLUDING BUT NOT
 * LIMITED TO THE RISKS AND COSTS OF PROGRAM ERRORS, COMPLIANCE WITH
 * APPLICABLE LAWS, DAMAGE TO OR LOSS OF DATA, PROGRAMS OR EQUIPMENT,
 * AND UNAVAILABILITY OR INTERRUPTION OF OPERATIONS.
 * </p>
 * <p>NEITHER RECIPIENT NOR ANY CONTRIBUTORS SHALL HAVE ANY LIABILITY FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING WITHOUT LIMITATION LOST PROFITS), HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OR
 * DISTRIBUTION OF THE PROGRAM OR THE EXERCISE OF ANY RIGHTS GRANTED
 * HEREUNDER, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGES.
 * </p>
 */
public class TextMarksV2APIClient {
	
	// -----------------------------------------------------------------------
	
	// Configuration:
	static public String API_URL_BASE      = "http://java1.api2.textmarks.com";

	// -----------------------------------------------------------------------

	/**
	 * Create TextMarksV2APIClient around indicated authentication info (optional).
	 *
	 * @param sApiKey   API Key ( register at http://www.textmarks.com/dev/api/reg/ ). (null for none).
	 * @param sAuthUser Phone# or TextMarks username to authenticate to API with. (null for none).
	 * @param sAuthPass TextMarks Password associated with sAuthUser. (null for none).
	 */
	public TextMarksV2APIClient( String sApiKey, String sAuthUser, String sAuthPass ) 
	{
		this.m_sApiKey    = sApiKey;
		this.m_sAuthUser  = sAuthUser;
		this.m_sAuthPass  = sAuthPass;
	}
	public TextMarksV2APIClient( ) {
	}

	// -----------------------------------------------------------------------

	public void setApiKey(String sApiKey) { this.m_sApiKey = sApiKey; }
	public void setAuthUser(String sAuthUser) { this.m_sAuthUser = sAuthUser; }
	public void setAuthPass(String sAuthPass) { this.m_sAuthPass = sAuthPass; }
	
	public String getApiKey() { return this.m_sApiKey; }
	public String getAuthUser() { return this.m_sAuthUser; }
	public String getAuthPass() { return this.m_sAuthPass; }
	
	// -----------------------------------------------------------------------

	/**
	 * Public method to call API.
	 *
	 * The API Key and auth params are automatically added if present.
	 *
	 * @param sPackageName    Package name.
	 * @param sMethodName     Method name.
	 * @param msoParams       Params for method.
	 * @return Decoded (from JSON) response.
	 * @throws Exception on error.
	 */
	public JSONObject call( String sPackageName, String sMethodName, Map<String,Object> msoParams ) 
		throws	TextMarksV2APIClientTransportException,
				TextMarksV2APIClientResultException
	{
		return this._callJsonApiMethod(sPackageName, sMethodName, msoParams);
	}

	// -----------------------------------------------------------------------

	/**
	 * Execute HTTP request (post params to API endpoint) and return string response.
	 *
	 * @param sUrl           URL to request (method endpoint).
	 * @param msoParams      Request params.
	 * @return Response (usually JSON string).
	 * @throws TextMarksV2APIClientTransportException on error.
	 */
	protected String _rawHttpCall( String sUrl, Map<String,Object> msoParams )
		throws	TextMarksV2APIClientTransportException
	{
		// Convert param map to encoded form (to post):
		String sPostData = "";
		try {
			StringBuffer sbPostData = new StringBuffer();
			for (Map.Entry<String,Object> entry : msoParams.entrySet()) {
				sbPostData.append("&").append(URLEncoder.encode(entry.getKey(), "UTF-8"));
				sbPostData.append("=").append(URLEncoder.encode(String.valueOf(entry.getValue()), "UTF-8"));
			}
			sPostData = sbPostData.toString();
		}
		catch (UnsupportedEncodingException e) {
		}

		// Create HTTP URL connection:
		HttpURLConnection urlConnection = null;
		try {
			URL url = new URL(sUrl);
			urlConnection = (HttpURLConnection) url.openConnection();
			(urlConnection).setRequestMethod("POST");
			urlConnection.setDoInput(true);
			urlConnection.setDoOutput(true);
			urlConnection.setUseCaches(false);
			urlConnection.setRequestProperty("Content-Type", "application/x-www-form-urlencoded");
			urlConnection.setRequestProperty("Content-Length", ""+ sPostData.length());
		}
		catch (Exception e) {
			throw new TextMarksV2APIClientTransportException("TextMarksV2APIClient ("+sUrl+") saw HTTP connection error: " + e.toString(), e);
		}

		// Send request:
		try {
			DataOutputStream out = new DataOutputStream(urlConnection.getOutputStream());
			out.writeBytes(sPostData);
			out.close();
		}
		catch (Exception e) {
			throw new TextMarksV2APIClientTransportException("TextMarksV2APIClient ("+sUrl+") saw HTTP request error: " + e.toString(), e);
		}

		// Get response:
		try {
			BufferedReader in = new BufferedReader(new InputStreamReader(urlConnection.getInputStream()));
			StringBuffer sbResponse = new StringBuffer();
			String sBuf;
			while((sBuf = in.readLine()) != null) {
				sbResponse.append(sBuf);
			}
			in.close();
			return sbResponse.toString();
		}
		catch (Exception e) {
			throw new TextMarksV2APIClientTransportException("TextMarksV2APIClient ("+sUrl+") saw HTTP response error: " + e.toString(), e);
		}
	}

	/**
	 * Execute API call and return decoded JSON response.
	 *
	 * The API Key and auth params are automatically added.
	 *
	 * @param sPackageName    Package name.
	 * @param sMethodName     Method name.
	 * @param msoParams       Params for method.
	 * @return Decoded (from JSON) response.
	 * @throws Exception on error.
	 */
	protected JSONObject _callJsonApiMethod( String sPackageName, String sMethodName, Map<String,Object> msoParams )
		throws	TextMarksV2APIClientTransportException,
				TextMarksV2APIClientResultException
	{
		// Prep:
		Map<String,Object> msoParamsFull = new HashMap<String,Object>(msoParams); // (copy to keep original clean)
		if (this.m_sApiKey != null)   { msoParamsFull.put("api_key", this.m_sApiKey); }
		if (this.m_sAuthUser != null) { msoParamsFull.put("auth_user", this.m_sAuthUser); }
		if (this.m_sAuthPass != null) { msoParamsFull.put("auth_pass", this.m_sAuthPass); }
		String sUrl = API_URL_BASE + "/" + sPackageName + "/" + sMethodName + "/";

		// Make actual HTTP call:
		String sResp = this._rawHttpCall(sUrl, msoParamsFull);

		// Parse JSON response:
		JSONObject joResp = null;
		try {
			joResp = new JSONObject(sResp);
		}
		catch (JSONException e) {
			throw new TextMarksV2APIClientTransportException("TextMarksV2APIClient ("+sUrl+") got invalid JSON response: " + e.toString(), e);
		}

		// Check API response code:
		try {
			int iResCode = joResp.getJSONObject("head").getInt("rescode");
			String sResMsg = joResp.getJSONObject("head").getString("resmsg");
			if (iResCode != 0) {
				throw new TextMarksV2APIClientResultException("TextMarksV2APIClient.call("+sPackageName+"."+sMethodName+") got API error #"+iResCode+": "+sResMsg, iResCode);
			}
		}
		catch (JSONException e) {
			throw new TextMarksV2APIClientTransportException("TextMarksV2APIClient ("+sUrl+") got bad malformed JSON response: " + e.toString(), e);
		}
		
		return joResp;
	}

	// -----------------------------------------------------------------------

	protected String m_sApiKey		= null;
	protected String m_sAuthUser	= null;
	protected String m_sAuthPass	= null;
}

