#include "HttpClient.h"
#include <Shlwapi.h>
#include <algorithm>
#include <cctype>
#include <functional>
#include "ParseUrl.h"
#include "StringHelper.h"

#define  BUFFER_SIZE       1024*10  

#define  NORMAL_CONNECT             INTERNET_FLAG_KEEP_CONNECTION  
#define  SECURE_CONNECT             NORMAL_CONNECT | INTERNET_FLAG_SECURE  
#define  NORMAL_REQUEST             INTERNET_FLAG_RELOAD | INTERNET_FLAG_DONT_CACHE   
#define  SECURE_REQUEST             NORMAL_REQUEST | INTERNET_FLAG_SECURE | INTERNET_FLAG_IGNORE_CERT_CN_INVALID
#define  MY_HTTP_REQUEST			INTERNET_FLAG_RELOAD
#define  MY_HTTPS_REQUEST			INTERNET_FLAG_SECURE


HttpClient::HttpClient()
{
	m_cb = NULL;
	m_pUserData = NULL;
	m_bStopReadFile = false;
}

HttpClient::~HttpClient()
{
	m_cb = NULL;
	m_pUserData = NULL;
	m_bStopReadFile = false;
	Clear();
}

void HttpClient::Clear()
{

}

int HttpClient::ExecuteRequest(LPCTSTR strMethod, LPCTSTR strUrl, LPCTSTR strPostData, string &strResponse)
{
	return SUCCESS;
}

int HttpClient::HttpGet(LPCTSTR strUrl, LPCTSTR strPostData, string &strResponse, const std::vector<std::string> & vePostHeaders, int & nStatus, const char * pSavePath)
{
	int nRet = SUCCESS;
	m_bStopReadFile = false;
	PARSEDURL pu;
	pu.cbSize = sizeof(pu);
	HRESULT hr = ParseURL(strUrl, &pu);
	if (SUCCEEDED(hr))
	{
		if (pu.nScheme != URL_SCHEME_HTTP && pu.nScheme != URL_SCHEME_HTTPS)
		{
			return FAILURE;
		}
	}

	Uri url = Uri::Parse(strUrl);
	
	// 建立会话
	HINTERNET hInternet;
	hInternet = InternetOpen(IE_AGENT, INTERNET_OPEN_TYPE_PRECONFIG, NULL, NULL, 0);
	if (hInternet == NULL)
	{
		OutputDebugStringA("appobd  -------建立会话--------\n");
		return FAILURE;
	}
	
	DWORD dwFlags = INTERNET_FLAG_RELOAD | INTERNET_FLAG_CACHE_IF_NET_FAIL;
	INTERNET_PORT nPort = atoi((CStringHelper::ws2s(url.Port)).c_str());
	if (0 == wcscmp(_T("https"), url.Protocol.c_str()))
	{
		nPort = INTERNET_DEFAULT_HTTPS_PORT;
		dwFlags = dwFlags | INTERNET_FLAG_SECURE | INTERNET_FLAG_IGNORE_REDIRECT_TO_HTTP;
	}

	// 建立连接
	HINTERNET hConnect;
	hConnect = InternetConnect(hInternet, url.Host.c_str(), nPort,//
		L"", L"", INTERNET_SERVICE_HTTP, 0, 0);

	if (hInternet == NULL)
	{
		InternetCloseHandle(hInternet);
		OutputDebugStringA("appobd  -------建立连接--------\n");

		return FAILURE;
	}

	std::wstring wstrObject = url.Path + url.QueryString;
	HINTERNET httpFile;
	httpFile = HttpOpenRequest(hConnect, L"GET", wstrObject.c_str(), HTTP_VERSION, NULL, 0, dwFlags, 0);//
	if (httpFile == NULL)
	{
		OutputDebugStringA("appobd  -------打开请求--------\n");
		InternetCloseHandle(hConnect);
		InternetCloseHandle(hInternet);

		return FAILURE;
	}

	for (UINT i = 0; i < vePostHeaders.size(); i++)
	{
		std::string strHeader = vePostHeaders[i];
		HttpAddRequestHeadersA(httpFile, strHeader.c_str(), strHeader.length(), HTTP_ADDREQ_FLAG_ADD);
	}

	if (HttpSendRequest(httpFile, NULL, NULL, 0, 0) == FALSE)
	{
		DWORD dwError = GetLastError();
		if (dwError == ERROR_INTERNET_NAME_NOT_RESOLVED)
		{
			nStatus = HTTP_REQUEST_TIME_OUT;
		}
		else if (dwError == ERROR_INTERNET_CANNOT_CONNECT ||
			dwError == 2)
		{
			nStatus = HTTP_SERVER_CONNECT_FAIL;
		}

		InternetCloseHandle(httpFile);
		InternetCloseHandle(hConnect);
		InternetCloseHandle(hInternet);
		OutputDebugStringA("appobd  -------发送请求--------\n");

		return FAILURE;
	}


	INT64 i64TotalSize = 0;
	DWORD dwLength = sizeof(i64TotalSize);
	BOOL bRet = ::HttpQueryInfo(httpFile, HTTP_QUERY_CONTENT_LENGTH | HTTP_QUERY_FLAG_NUMBER, &i64TotalSize, &dwLength, 0);
		
	int nState = 0;
	DWORD n = sizeof(nState);
	bRet = ::HttpQueryInfo(httpFile, HTTP_QUERY_STATUS_CODE | HTTP_QUERY_FLAG_NUMBER, &nState, &n, 0);
	nStatus = nState;

	FILE *fp = NULL;
	if (pSavePath)
	{
		fopen_s(&fp, pSavePath, "wb");
	}

	char buf[BUFFER_SIZE + 1] = { 0 };
	DWORD buf_len = BUFFER_SIZE;
	DWORD buf_read = BUFFER_SIZE;
	INT64 buf_count = 0;
	std::string strRawResponse;
	while (!m_bStopReadFile)
	{
		InternetReadFile(httpFile, buf, buf_len, &buf_read);
		if (buf_read == 0) break;
		buf[buf_read] = '\0';

		if (pSavePath && fp)
		{
			fwrite(buf, 1, buf_read, fp);
		} 
		else
		{
			strRawResponse += buf;
		}

		buf_count += buf_read;
		memset(buf, 0, BUFFER_SIZE + 1);

		if (m_cb)
		{
			m_cb(buf_count, i64TotalSize, m_pUserData);
		}
	}

	if (fp)
	{
		fclose(fp);
	}
	strResponse = strRawResponse;

	if (m_bStopReadFile)
	{
		nStatus = HTTP_SERVER_DISCONNECT;
	}

	InternetCloseHandle(httpFile);
	InternetCloseHandle(hConnect);
	InternetCloseHandle(hInternet);

	return nRet;
}

int HttpClient::HttpPost(LPCTSTR strUrl, LPCTSTR strPostData, const std::vector<std::string> & vePostHeaders, string &strResponse, int & nStatus, const char * pSavePath)
{
	m_bStopReadFile = false;
	PARSEDURL pu;
	pu.cbSize = sizeof(pu);
	HRESULT hr = ParseURL(strUrl, &pu);
	if (SUCCEEDED(hr))
	{
		if (pu.nScheme != URL_SCHEME_HTTP && pu.nScheme != URL_SCHEME_HTTPS)
		{
			return FAILURE;
		}
	}

	Uri url = Uri::Parse(strUrl);

	// 建立会话
	HINTERNET hInternet;
	hInternet = InternetOpen(IE_AGENT, INTERNET_OPEN_TYPE_PRECONFIG, NULL, NULL, 0);
	if (hInternet == NULL)
		return FAILURE;

	DWORD dwFlags = INTERNET_FLAG_RELOAD | INTERNET_FLAG_CACHE_IF_NET_FAIL;
	INTERNET_PORT nPort = atoi((CStringHelper::ws2s(url.Port)).c_str());
	if (0 == wcscmp(_T("https"), url.Protocol.c_str()))
	{
		nPort = INTERNET_DEFAULT_HTTPS_PORT;
		dwFlags = dwFlags | INTERNET_FLAG_SECURE;
	}

	// 建立连接
	HINTERNET hConnect;
	hConnect = InternetConnect(hInternet, url.Host.c_str(), nPort,
		L"", L"", INTERNET_SERVICE_HTTP, 0, 0);

	if (hInternet == NULL)
	{
		InternetCloseHandle(hInternet);
		return FAILURE;
	}

	std::wstring wstrObject = url.Path + url.QueryString;
	HINTERNET httpFile;
	httpFile = HttpOpenRequest(hConnect, L"POST", wstrObject.c_str(), HTTP_VERSION, NULL, 0, dwFlags, 0);
	if (httpFile == NULL)
	{
		InternetCloseHandle(hConnect);
		InternetCloseHandle(hInternet);

		return FAILURE;
	}

	for (UINT i=0; i<vePostHeaders.size(); i++)
	{
		std::string strHeader = vePostHeaders[i];
		HttpAddRequestHeadersA(httpFile, strHeader.c_str(), strHeader.length(), HTTP_ADDREQ_FLAG_ADD);
	}

	std::wstring wstrPostData = strPostData;
	std::string strPostData2 = CStringHelper::UnicodeToUTF8(wstrPostData);
	DWORD dwSize = strPostData2.length();
	if (HttpSendRequestA(httpFile, NULL, NULL, (LPVOID)strPostData2.c_str(), dwSize) == FALSE)
	{
		DWORD dwError = GetLastError();
		if (dwError == ERROR_INTERNET_NAME_NOT_RESOLVED)
		{
			nStatus = HTTP_REQUEST_TIME_OUT;
		}
		else if (dwError == ERROR_INTERNET_CANNOT_CONNECT ||
			dwError == 2)
		{
			nStatus = HTTP_SERVER_CONNECT_FAIL;
		}

		InternetCloseHandle(httpFile);
		InternetCloseHandle(hConnect);
		InternetCloseHandle(hInternet);

		return FAILURE;
	}


	INT64 i64TotalSize = 0;
	DWORD dwLength = sizeof(i64TotalSize);
	BOOL bRet = ::HttpQueryInfo(httpFile, HTTP_QUERY_CONTENT_LENGTH | HTTP_QUERY_FLAG_NUMBER, &i64TotalSize, &dwLength, 0);

	int nState = 0;
	DWORD n = sizeof(nState);
	::HttpQueryInfo(httpFile, HTTP_QUERY_STATUS_CODE | HTTP_QUERY_FLAG_NUMBER, &nState, &n, 0);
	nStatus = nState;

	FILE *fp = NULL;
	if (pSavePath)
	{
		fopen_s(&fp, pSavePath, "wb");
	}

	char buf[BUFFER_SIZE + 1] = { 0 };
	DWORD buf_len = BUFFER_SIZE;
	DWORD buf_read = BUFFER_SIZE;
	INT64 buf_count = 0;
	std::string strRawResponse;
	while (!m_bStopReadFile)
	{
		InternetReadFile(httpFile, buf, buf_len, &buf_read);
		if (buf_read == 0) break;
		buf[buf_read] = '\0';

		if (pSavePath && fp)
		{
			fwrite(buf, 1, buf_read, fp);
		}
		else
		{
			strRawResponse += buf;
		}

		buf_count += buf_read;
		memset(buf, 0, BUFFER_SIZE + 1);

		if (m_cb)
		{
			m_cb(buf_count, i64TotalSize, m_pUserData);
		}
	}

	if (fp)
	{
		fclose(fp);
	}
	strResponse = strRawResponse;

	if (m_bStopReadFile)
	{
		nStatus = HTTP_SERVER_DISCONNECT;
	}
	
	InternetCloseHandle(httpFile);
	InternetCloseHandle(hConnect);
	InternetCloseHandle(hInternet);

	return SUCCESS;
}

BOOL UseHttpSendRequestDataToWeb(HINTERNET hRequest, std::string strAccessToken, std::multimap<std::string, std::string> mapPostData, std::multimap<std::string, std::string> mapInFileName, BOOL bUseHeader)
{
	if (!hRequest)
		return FALSE;

	char szFormat[1024] = { 0 };
	ULONGLONG   ullPostSize = 0;
	std::string strKey = "";
	std::string strValue = "";
	std::string strFilePath = "";
	std::string strFileName = "";
	std::string strFormat = "";
	std::map<std::string, std::string> mapFirstFile;
	std::map<std::string, std::string> mapPostFile;
	std::vector<std::string> vecPostData;

	std::multimap<std::string, std::string>::iterator it;
	for (it = mapInFileName.begin(); it != mapInFileName.end(); it++)
	{
		strKey = it->first;
		strValue = it->second;

		FILE *fp = NULL;
		fopen_s(&fp, strValue.c_str(), "rb");
		if (NULL == fp)
		{
			continue;
		}
	
		fseek(fp, 0L, SEEK_END);
		ullPostSize += ftell(fp);
		fclose(fp);

		strFileName = CStringHelper::GetFileName(strValue.c_str());
		std::string strExtName = CStringHelper::GetExtName(strFileName.c_str());
		if (0 == strcmp(strExtName.c_str(), "png"))
		{
			std::wstring wstr = CStringHelper::s2ws(strFileName);
			strFileName = CStringHelper::UnicodeToUTF8(wstr);
			sprintf_s(szFormat, "-----------------------------7dd16d1320516\r\nContent-Disposition: form-data; name=%s; filename=%s\r\nContent-Type: image/x-png\r\n\r\n", strKey.c_str(), strFileName.c_str());
		}
		else if (0 == strcmp(strExtName.c_str(), "jpg"))
		{
			std::wstring wstr = CStringHelper::s2ws(strFileName);
			strFileName = CStringHelper::UnicodeToUTF8(wstr);
			sprintf_s(szFormat, "-----------------------------7dd16d1320516\r\nContent-Disposition: form-data; name=%s; filename=%s\r\nContent-Type: image/pjpeg\r\n\r\n", strKey.c_str(), strFileName.c_str());
		}
		else if (0 == strcmp(strExtName.c_str(), "bmp"))
		{
			std::wstring wstr = CStringHelper::s2ws(strFileName);
			strFileName = CStringHelper::UnicodeToUTF8(wstr);
			sprintf_s(szFormat, "-----------------------------7dd16d1320516\r\nContent-Disposition: form-data; name=%s; filename=%s\r\nContent-Type: image/bmp\r\n\r\n", strKey.c_str(), strFileName.c_str());
		}
		else
		{
			std::wstring wstr = CStringHelper::s2ws(strFileName);
			strFileName = CStringHelper::UnicodeToUTF8(wstr);
			sprintf_s(szFormat, "-----------------------------7dd16d1320516\r\nContent-Disposition: form-data; name=%s; filename=%s\r\n\r\n", strKey.c_str(), strFileName.c_str());
		}

		if (it != mapInFileName.begin())
		{
			strFormat = "\r\n";
		}
		
		strFormat += szFormat;
		ullPostSize += strFormat.length();

		if (it == mapInFileName.begin())
		{
			mapFirstFile.insert(make_pair(strFormat, strValue));
		}
		else
		{
			mapPostFile.insert(make_pair(strFormat, strValue));
		}
	}

	std::multimap<std::string, std::string>::iterator it2;
	for (it2 = mapPostData.begin(); it2 != mapPostData.end(); it2++)
	{
		std::string strKey = it2->first;
		std::string strValue = it2->second;

		std::vector<std::string> vecValue = CStringHelper::Split(strValue, ",");
		if (vecValue.size() == 2)
		{
			int nValue = 0;
			char csValue[16] = { 0 };
			strcmp(vecValue[0].c_str(), "int") == 0 ? sprintf_s(csValue, "%s", vecValue[1].c_str()) : sprintf_s(csValue, "%s", vecValue[0].c_str());
			nValue = atoi(csValue);
			sprintf_s(szFormat, "-----------------------------7dd16d1320516\r\nContent-Disposition: form-data; name=%s\r\n\r\n%d\r\n", strKey.c_str(), nValue);
		}
		else
		{
			std::wstring wstr = CStringHelper::s2ws(strValue);
			strValue = CStringHelper::UnicodeToUTF8(wstr);
			sprintf_s(szFormat, "-----------------------------7dd16d1320516\r\nContent-Disposition: form-data; name=%s\r\n\r\n%s\r\n", strKey.c_str(), strValue.c_str());
		}

//		sprintf_s(szFormat, "-----------------------------7dd16d1320516\r\nContent-Disposition: form-data; name=\"%s\"\r\n\r\n\"%s\"\r\n", strKey.c_str(), strValue.c_str());
		strFormat = szFormat;
		ullPostSize += strFormat.length();

		vecPostData.push_back(strFormat);
	}


	//此处是header和body的前置数据段，一般向WEB发送数据是不需要的(即：bUseHeader=FALSE)  
	//因为模拟了通过web页面向服务器post数据，所以需要添加boundary(分隔符, 用于描述不同的数据名称、标识、data==)  
	//模拟时，\r\n是需要的，个数是通过Fiddler2软件截获正常网页发送时为防止错才固定的  
	//至于数字7dd16d1320516本来是随机值，但对于业务来说，此处仅仅是分割作用，无所谓了  
	char szHeader[2048] = { 0 };
	if (strAccessToken.length() > 0)
	{
		sprintf_s(szHeader, "%s%s", strAccessToken.c_str(), "Accept: *,*/*\r\nConnection: Keep-Alive\r\nContent-Type: application/octet-stream\r\nContent-Type: multipart/form-data; boundary=---------------------------7dd16d1320516\r\n");
	}
	else
	{
		sprintf_s(szHeader, "%s", "Accept: *,*/*\r\nConnection: Keep-Alive\r\nContent-Type: application/octet-stream\r\nContent-Type: multipart/form-data; boundary=---------------------------7dd16d1320516\r\n");
	}
	//char szHeader[] = "Accept: *,*/*\r\nConnection: Keep-Alive\r\nContent-Type: application/octet-stream\r\nContent-Type: multipart/form-data; boundary=---------------------------7dd16d1320516\r\n";
	char*   pchEnd = "\r\n-----------------------------7dd16d1320516--\r\n";
	int     nEndLen = strlen(pchEnd);

	//发送上传请求  
	INTERNET_BUFFERSA   bufferIn = { 0 };
	bufferIn.dwStructSize = sizeof(INTERNET_BUFFERSA);   //必须设置，否则出错  
	bufferIn.lpcszHeader = szHeader;
	bufferIn.dwHeadersLength = strlen(szHeader);
	bufferIn.dwBufferTotal = ullPostSize + nEndLen;
	if (!HttpSendRequestExA(hRequest, &bufferIn, NULL, 0, 0))
		return FALSE;

	const int   N_SizeEveryTime = 1024;
	BYTE        bufData[N_SizeEveryTime] = { 0 };
	DWORD       dwBytesRead = 0;
	DWORD       dwBytesOut = 0;
	DWORD       dwBytesOutAll = 0;
	BOOL        bRet = TRUE;

	for (UINT i = 0; i < vecPostData.size(); i++)
	{
		std::string strPostData = vecPostData[i];
		bRet = InternetWriteFile(hRequest, strPostData.c_str(), strPostData.length(), &dwBytesOut);
		if (!bRet)
			return FALSE;

		dwBytesOutAll += dwBytesOut;
	}

	std::map<std::string, std::string>::iterator it3;
	for (it3 = mapFirstFile.begin(); it3 != mapFirstFile.end(); it3++)
	{
		strKey = it3->first;
		strValue = it3->second;
		bRet = InternetWriteFile(hRequest, strKey.c_str(), strKey.length(), &dwBytesOut);
		if (!bRet)
			return FALSE;

		dwBytesOutAll += dwBytesOut;

		FILE *fp = NULL;
		fopen_s(&fp, strValue.c_str(), "rb");
		if (NULL == fp)
		{
			return FALSE;
		}

		//读取并发送数据  
		while (TRUE)
		{
			dwBytesRead = fread(bufData, 1, N_SizeEveryTime, fp);
			if (dwBytesRead <= 0)
				break;

			//write数据  
			bRet = InternetWriteFile(hRequest, bufData, dwBytesRead, &dwBytesOut);
			if (!bRet)
				return FALSE;

			dwBytesOutAll += dwBytesOut;
		}

		fclose(fp);
	}

	for (it3 = mapPostFile.begin(); it3 != mapPostFile.end(); it3++)
	{
		strKey = it3->first;
		strValue = it3->second;
		bRet = InternetWriteFile(hRequest, strKey.c_str(), strKey.length(), &dwBytesOut);
		if (!bRet)
			return FALSE;

		dwBytesOutAll += dwBytesOut;

		FILE *fp = NULL;
		fopen_s(&fp, strValue.c_str(), "rb");
		if (NULL == fp)
		{
			return FALSE;
		}

		//读取并发送数据  
		while (TRUE)
		{
			dwBytesRead = fread(bufData, 1, N_SizeEveryTime, fp);
			if (dwBytesRead <= 0)
				break;

			//write数据  
			bRet = InternetWriteFile(hRequest, bufData, dwBytesRead, &dwBytesOut);
			if (!bRet)
				return FALSE;

			dwBytesOutAll += dwBytesOut;
		}

		fclose(fp);
	}

	//如果filename不为空，则发送表尾结束符  
	if (bUseHeader)
	{
		bRet = InternetWriteFile(hRequest, pchEnd, nEndLen, &dwBytesOut);
		if (!bRet)
			return FALSE;

		dwBytesOutAll += dwBytesOut;
	}

	//结束上传请求  
	if (!HttpEndRequest(hRequest, NULL, 0, 0))
	{
		DWORD   dwErr = GetLastError();
		return FALSE;
	}

	return dwBytesOutAll == bufferIn.dwBufferTotal;
}

//int HttpClient::HttpPostEX(LPCTSTR strUrl, LPCTSTR strPostData, string &strResponse, const char * pInFilePath, const char * pOutFilePath)
int HttpClient::HttpPostEX(std::string strUrl, std::string strAccessToken, std::multimap<std::string, std::string> mapPostData, std::multimap<std::string, std::string> mapInFileName, string &strResponse, int & nStatus, const char * pOutFilePath)
{
	m_bStopReadFile = false;
	const int   N_BufferSizeRead_Net = 1024;
	BOOL        bRet = FALSE;
	DWORD       dwErr = 0;
	HINTERNET   hOpen = NULL;
	HINTERNET   hConnect = NULL;
	HINTERNET   hRequest = NULL;
	int nRet = FAILURE;
	do
	{
		PARSEDURLA pu;
		pu.cbSize = sizeof(pu);
		HRESULT hr = ParseURLA(strUrl.c_str(), &pu);
		if (SUCCEEDED(hr))
		{
			if (pu.nScheme != URL_SCHEME_HTTP && pu.nScheme != URL_SCHEME_HTTPS)
			{
				break;;
			}
		}

		Uri url = Uri::Parse(CStringHelper::s2ws(strUrl));
		std::wstring wstrObject = url.Path + url.QueryString;

		//Initializes an application's use of the WinINet functions.  
		hOpen = InternetOpen(IE_AGENT, INTERNET_OPEN_TYPE_PRECONFIG, NULL, NULL, 0);
		if (!hOpen)
			goto EndFlag;

		DWORD dwFlags = INTERNET_FLAG_NO_CACHE_WRITE | INTERNET_FLAG_RELOAD;
		INTERNET_PORT nPort = atoi((CStringHelper::ws2s(url.Port)).c_str());
		if (0 == wcscmp(_T("https"), url.Protocol.c_str()))
		{
			nPort = INTERNET_DEFAULT_HTTPS_PORT;
			dwFlags = INTERNET_FLAG_NO_CACHE_WRITE | INTERNET_FLAG_SECURE;
		}

		//Opens an File Transfer Protocol (FTP), Gopher, or HTTP session for a given site  
		hConnect = InternetConnect(hOpen, url.Host.c_str(), nPort,
			NULL, NULL, INTERNET_SERVICE_HTTP, NULL, NULL);
		if (!hConnect)
			goto EndFlag;

		//Creates an HTTP request handle.  
		TCHAR* szAccept[] = { _T("*/*"), NULL };
		hRequest = HttpOpenRequest(hConnect, _T("POST"), wstrObject.c_str(), HTTP_VERSION/*NULL*/, NULL, /*NULL*/(LPCTSTR*)szAccept, dwFlags/*INTERNET_FLAG_SECURE| INTERNET_FLAG_RELOAD /*| INTERNET_FLAG_KEEP_CONNECTION*/, 0);
		if (!hRequest)
			goto EndFlag;

		if (UseHttpSendRequestDataToWeb(hRequest, strAccessToken, mapPostData, mapInFileName, true))
		{
			//Retrieves header information associated with an HTTP request  
			int nState = 0;
			DWORD n = sizeof(nState);
			::HttpQueryInfo(hRequest, HTTP_QUERY_STATUS_CODE | HTTP_QUERY_FLAG_NUMBER, &nState, &n, 0);
			nStatus = nState;

			INT64 i64TotalSize = 0;
			DWORD dwLength = sizeof(i64TotalSize);
			BOOL bRet = ::HttpQueryInfo(hRequest, HTTP_QUERY_CONTENT_LENGTH | HTTP_QUERY_FLAG_NUMBER, &i64TotalSize, &dwLength, 0);

			char    chBuf[N_BufferSizeRead_Net] = { 0 };
			DWORD   dwBytesRead = 0;
			INT64	buf_count = 0;
			FILE *fp = NULL;
			if (pOutFilePath)
			{
				fopen_s(&fp, pOutFilePath, "wb");
			}

			do
			{
				//读取上传完成后的返回结果  
				memset(chBuf, 0, N_BufferSizeRead_Net);
				if (InternetReadFile(hRequest, chBuf, N_BufferSizeRead_Net - 1, &dwBytesRead) && dwBytesRead > 0)
				{
					/// <读取保存返回的数据>
					if (fp && nState == 201)
					{
						fwrite(chBuf, 1, dwBytesRead, fp);
					}
					else
					{
						strResponse += chBuf;
					}

					buf_count += dwBytesRead;
					if (m_cb)
					{
						m_cb(buf_count, i64TotalSize, m_pUserData);
					}
				}
				else
				{
					dwErr = GetLastError();
					break;
				}
			} while (/*dwContentLen > 0 &&*/ dwBytesRead > 0 && !m_bStopReadFile);

			if (fp)
			{
				fclose(fp);
			}

			nRet = SUCCESS;
		}
		else
		{
			DWORD dwError = GetLastError();
			if (dwError == ERROR_INTERNET_NAME_NOT_RESOLVED)
			{
				nStatus = HTTP_REQUEST_TIME_OUT;
			}
			else if (dwError == ERROR_INTERNET_CANNOT_CONNECT ||
				dwError == 2)
			{
				nStatus = HTTP_SERVER_CONNECT_FAIL;
			}
		}

EndFlag:
		if (hRequest)
		{
			bRet = InternetCloseHandle(hRequest);
			hRequest = NULL;
		}
		if (hConnect)
		{
			bRet = InternetCloseHandle(hConnect);
			hConnect = NULL;
		}
		if (hOpen)
		{
			bRet = InternetCloseHandle(hOpen);
			hOpen = NULL;
		}
		break;

	} while (0);

	return nRet;
}

void HttpClient::RegiterCallBack(PFDataProgressCallBack cb, void *pUserData)
{
	m_cb = cb;
	m_pUserData = pUserData;
}

void HttpClient::StopInternetReadFile()
{
	m_bStopReadFile = true;
}