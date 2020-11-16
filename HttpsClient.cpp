#include "stdafx.h"
#include "HttpsClient.h"
#include "../Include/curl/curl.h"
#include "StringHelper.h"


CHttpsClient::CHttpsClient()
{
	m_i64DataSize = 0;
	m_i64CountSize = 0;
	m_strError = "";
	m_strSavePath = "";
	m_bStopRecvData = false;
	m_strResponse = "";
	m_cb = NULL;
	m_pFile = NULL;
}


CHttpsClient::~CHttpsClient()
{
}

size_t write_header(void *ptr, size_t size, size_t nmemb, void *stream)
{
	std::string *str = (std::string*)stream;
	(*str).append((char*)ptr, size*nmemb);

	return size*nmemb;
}


size_t write_data(void *ptr, size_t size, size_t nmemb, void *stream)
{
// 	std::string *str = (std::string*)stream;
// 	(*str).append((char*)ptr, size*nmemb);
	// 	curl_easy_pause(curl, CURLPAUSE_RECV);
	// 	curl_easy_cleanup(curl);
	//return -1; // <停止接收数据并退出>

	if (stream)
	{
		CHttpsClient *pThis = (CHttpsClient *)stream;
		if (pThis->IsStopRecvData())
		{
			return -1;
		}

		return pThis->WirteData(ptr, size, nmemb);
	}


	return 0;
}

std::string CHttpsClient::GetError()
{
	return m_strError;
}

bool CHttpsClient::IsStopRecvData()
{
	return m_bStopRecvData;
}

void CHttpsClient::StopRecvData()
{
	m_bStopRecvData = true;
}

size_t CHttpsClient::WirteData(void *ptr, size_t size, size_t nmemb)
{
	size_t written = 0;
	if (m_strSavePath.length() > 0 && m_pFile)
	{
		written = fwrite(ptr, size, nmemb, (FILE *)m_pFile);
	}
	else
	{
		m_strResponse.append((char*)ptr, size * nmemb);
	}

	if (m_cb)
	{
		written = size * nmemb;
		m_i64DataSize += written;
		m_cb(m_i64DataSize, m_i64CountSize, m_pUserData);
	}

	return size * nmemb;
}

void CHttpsClient::RegiterCallBack(PFDataProgressCallBack cb, void *pUserData)
{
	if (cb)
	{
		m_cb = cb;
	}

	if (pUserData)
	{
		m_pUserData = pUserData;
	}
}

void CHttpsClient::InitParam(const char * pSavePath)
{
	m_bStopRecvData = false;
	m_i64DataSize = 0;
	m_i64CountSize = 0;
	m_strSavePath = "";
	m_strResponse = "";
	m_pFile = NULL;

	if (pSavePath)
	{
		m_strSavePath = pSavePath;
		fopen_s(&m_pFile, pSavePath, "wb");
	}
}

int CHttpsClient::HttpsGet(std::string strUrl, std::string &strResponse, std::string & strHeader, std::vector<std::string> & vecHeaders, int & nStatus, const char * pSavePath)
{
	int nRet = ERR_SUCCESS;
	double size = 0.0;
	CURLcode ret;
	CURL *curl = curl_easy_init();
	struct curl_slist *headers = NULL;
	
	InitParam(pSavePath);

	//增加HTTP header
	for (size_t i = 0; i < vecHeaders.size(); i++)
	{
		headers = curl_slist_append(headers, vecHeaders[i].c_str());
	}
	do 
	{
		if (curl)
		{
			ret = curl_easy_setopt(curl, CURLOPT_URL, strUrl.c_str());
			if (ret != CURLE_OK)
			{
				m_strError = curl_easy_strerror(ret);
				nRet = ERR_PARM_ERROR;
				break;
			}
	
			if (headers)
			{
				curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
				if (ret != CURLE_OK)
				{
					m_strError = curl_easy_strerror(ret);
					nRet = ERR_PARM_ERROR;
					break;
				}
			}
		
			ret = curl_easy_setopt(curl, CURLOPT_NOBODY, 1L);
			if (ret != CURLE_OK)
			{
				m_strError = curl_easy_strerror(ret);
				nRet = ERR_PARM_ERROR;
				break;
			}
		
			ret = curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, write_header);
			if (ret != CURLE_OK)
			{
				m_strError = curl_easy_strerror(ret);
				nRet = ERR_PARM_ERROR;
				break;
			}
		
			ret = curl_easy_setopt(curl, CURLOPT_WRITEHEADER, (void *)&strHeader);
			if (ret != CURLE_OK)
			{
				m_strError = curl_easy_strerror(ret);
				nRet = ERR_PARM_ERROR;
				break;
			}
		
			ret = curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
			if (ret != CURLE_OK)
			{
				m_strError = curl_easy_strerror(ret);
				nRet = ERR_PARM_ERROR;
				break;
			}
			
			ret = curl_easy_perform(curl);
			if (ret != CURLE_OK)
			{
				m_strError = curl_easy_strerror(ret);
				nRet = ERR_SOCKET_ERROR;
				break;
			}
		
			ret = curl_easy_getinfo(curl, CURLINFO_CONTENT_LENGTH_DOWNLOAD, &size);
			if (ret != CURLE_OK)
			{
				m_strError = curl_easy_strerror(ret);
			}
		
			m_i64CountSize = (INT64)size;
			curl_easy_cleanup(curl);
		}
	
		curl = NULL;
		curl = curl_easy_init();
		if (curl)
		{
			// set params  
			ret = curl_easy_setopt(curl, CURLOPT_URL, strUrl.c_str()); // url  
			if (ret != CURLE_OK)
			{
				m_strError = curl_easy_strerror(ret);
				nRet = ERR_PARM_ERROR;
				break;
			}

			if (headers)
			{
				curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
				if (ret != CURLE_OK)
				{
					m_strError = curl_easy_strerror(ret);
					nRet = ERR_PARM_ERROR;
					break;
				}
			}

			ret = curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, false); // if want to use https  
			if (ret != CURLE_OK)
			{
				m_strError = curl_easy_strerror(ret);
				nRet = ERR_PARM_ERROR;
				break;
			}

			ret = curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, false); // set peer and host verify false  
			if (ret != CURLE_OK)
			{
				m_strError = curl_easy_strerror(ret);
				nRet = ERR_PARM_ERROR;
				break;
			}

			ret = curl_easy_setopt(curl, CURLOPT_VERBOSE, 1);
			if (ret != CURLE_OK)
			{
				m_strError = curl_easy_strerror(ret);
				nRet = ERR_PARM_ERROR;
				break;
			}

			ret = curl_easy_setopt(curl, CURLOPT_READFUNCTION, NULL);
			if (ret != CURLE_OK)
			{
				m_strError = curl_easy_strerror(ret);
				nRet = ERR_PARM_ERROR;
				break;
			}

			ret = curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data);
			if (ret != CURLE_OK)
			{
				m_strError = curl_easy_strerror(ret);
				nRet = ERR_PARM_ERROR;
				break;
			}

			ret = curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)this);
			if (ret != CURLE_OK)
			{
				m_strError = curl_easy_strerror(ret);
				nRet = ERR_PARM_ERROR;
				break;
			}

			ret = curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1);
			if (ret != CURLE_OK)
			{
				m_strError = curl_easy_strerror(ret);
				nRet = ERR_PARM_ERROR;
				break;
			}

			ret = curl_easy_setopt(curl, CURLOPT_HEADER, 0);
			if (ret != CURLE_OK)
			{
				m_strError = curl_easy_strerror(ret);
				nRet = ERR_PARM_ERROR;
				break;
			}

			ret = curl_easy_setopt(curl, CURLOPT_NOBODY, 0);    //需求body  
			if (ret != CURLE_OK)
			{
				m_strError = curl_easy_strerror(ret);
				nRet = ERR_PARM_ERROR;
				break;
			}

			ret = curl_easy_perform(curl);
			if (ret != CURLE_OK)
			{
				m_strError = curl_easy_strerror(ret);
				nRet = ERR_PARM_ERROR;
				break;
			}

			strResponse = m_strResponse;

			long nState = 0;
			ret = curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &nState);
			if (ret == CURLE_OK)
			{
				nStatus = (int)nState;
			}

			// release curl  
			curl_easy_cleanup(curl);
			m_strError = curl_easy_strerror(ret);
		}
	} while (false);

	if (headers)
	{
		curl_slist_free_all(headers);
	}

	if (m_pFile)
	{
		fclose(m_pFile);
		m_pFile = NULL;
	}

 	return nRet;
}

int CHttpsClient::HttpsPost(std::string strUrl, std::string strPostData, std::vector<std::string> & vecHeaders, string &strResponse, int & nStatus, const char * pSavePath)
{
	int nRet = ERR_SUCCESS;
	double size = 0.0;
	CURLcode ret;
	CURL *curl = NULL;
	struct curl_slist *headers = NULL;
	struct curl_httppost *formpost = NULL;
	struct curl_httppost *lastptr = NULL;

	InitParam(pSavePath);

	//增加HTTP header
	for (size_t i = 0; i < vecHeaders.size(); i++)
	{
		headers = curl_slist_append(headers, vecHeaders[i].c_str());
	}

	do
	{
		curl = curl_easy_init();
		if (curl)
		{
			// set params  
			ret = curl_easy_setopt(curl, CURLOPT_URL, strUrl.c_str()); // url  
			if (ret != CURLE_OK)
			{
				m_strError = curl_easy_strerror(ret);
				nRet = ERR_PARM_ERROR;
				break;
			}

			ret = curl_easy_setopt(curl, CURLOPT_POST, 1L);
			if (ret != CURLE_OK)
			{
				m_strError = curl_easy_strerror(ret);
				nRet = ERR_PARM_ERROR;
				break;
			}

			if (headers)
			{
				curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
				if (ret != CURLE_OK)
				{
					m_strError = curl_easy_strerror(ret);
					nRet = ERR_PARM_ERROR;
					break;
				}
			}

			std::wstring wstrPostData = CStringHelper::s2ws(strPostData);
			std::string strPostDataTmp = CStringHelper::UnicodeToUTF8(wstrPostData);
			ret = curl_easy_setopt(curl, CURLOPT_POSTFIELDS, strPostDataTmp.c_str());
			if (ret != CURLE_OK)
			{
				m_strError = curl_easy_strerror(ret);
				nRet = ERR_PARM_ERROR;
				break;
			}

			ret = curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, strPostDataTmp.length());
			if (ret != CURLE_OK)
			{
				m_strError = curl_easy_strerror(ret);
				nRet = ERR_PARM_ERROR;
				break;
			}

			ret = curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, false); // if want to use https  
			if (ret != CURLE_OK)
			{
				m_strError = curl_easy_strerror(ret);
				nRet = ERR_PARM_ERROR;
				break;
			}

			ret = curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, false); // set peer and host verify false  
			if (ret != CURLE_OK)
			{
				m_strError = curl_easy_strerror(ret);
				nRet = ERR_PARM_ERROR;
				break;
			}

			ret = curl_easy_setopt(curl, CURLOPT_VERBOSE, 1);
			if (ret != CURLE_OK)
			{
				m_strError = curl_easy_strerror(ret);
				nRet = ERR_PARM_ERROR;
				break;
			}

			ret = curl_easy_setopt(curl, CURLOPT_READFUNCTION, NULL);
			if (ret != CURLE_OK)
			{
				m_strError = curl_easy_strerror(ret);
				nRet = ERR_PARM_ERROR;
				break;
			}

			ret = curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data);
			if (ret != CURLE_OK)
			{
				m_strError = curl_easy_strerror(ret);
				nRet = ERR_PARM_ERROR;
				break;
			}

			ret = curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)this);
			if (ret != CURLE_OK)
			{
				m_strError = curl_easy_strerror(ret);
				nRet = ERR_PARM_ERROR;
				break;
			}

			ret = curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1);
			if (ret != CURLE_OK)
			{
				m_strError = curl_easy_strerror(ret);
				nRet = ERR_PARM_ERROR;
				break;
			}

			ret = curl_easy_setopt(curl, CURLOPT_HEADER, 0);
			if (ret != CURLE_OK)
			{
				m_strError = curl_easy_strerror(ret);
				nRet = ERR_PARM_ERROR;
				break;
			}

			ret = curl_easy_perform(curl);
			if (ret != CURLE_OK)
			{
				m_strError = curl_easy_strerror(ret);
				nRet = ERR_PARM_ERROR;
				break;
			}

			strResponse = m_strResponse;

			long nState = 0;
			ret = curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &nState);
			if (ret == CURLE_OK)
			{
				nStatus = (int)nState;
			}

			// release curl  
			curl_easy_cleanup(curl);
			m_strError = curl_easy_strerror(ret);
		}
	} while (false);

	if (headers)
	{
		curl_slist_free_all(headers);
	}

	if (m_pFile)
	{
		fclose(m_pFile);
		m_pFile = NULL;
	}

	return nRet;
}

int CHttpsClient::HttpsPostEX(std::string strUrl, std::vector<std::string> & vecHeaders, std::multimap<std::string, std::string> mapPostData, std::multimap<std::string, std::string> mapPostFile, string &strResponse, int & nStatus, const char * pSavePath)
{
	int nRet = ERR_SUCCESS;
	double size = 0.0;
	CURLcode ret;
	CURLFORMcode form_ret;
	CURL *curl = NULL;
	struct curl_slist *headers = NULL;
	struct curl_httppost *formpost = NULL;
	struct curl_httppost *lastptr = NULL;

	InitParam(pSavePath);

	//增加HTTP header
	for (size_t i = 0; i < vecHeaders.size(); i++)
	{
		headers = curl_slist_append(headers, vecHeaders[i].c_str());
	}

	do
	{
		curl = curl_easy_init();
		if (curl)
		{
			// set params  
			ret = curl_easy_setopt(curl, CURLOPT_URL, strUrl.c_str()); // url  
			if (ret != CURLE_OK)
			{
				m_strError = curl_easy_strerror(ret);
				nRet = ERR_PARM_ERROR;
				break;
			}

			ret = curl_easy_setopt(curl, CURLOPT_POST, 1L);
			if (ret != CURLE_OK)
			{
				m_strError = curl_easy_strerror(ret);
				nRet = ERR_PARM_ERROR;
				break;
			}

			if (headers)
			{
				curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
				if (ret != CURLE_OK)
				{
					m_strError = curl_easy_strerror(ret);
					nRet = ERR_PARM_ERROR;
					break;
				}
			}

			std::multimap<std::string, std::string>::iterator it1;
			for (it1 = mapPostData.begin(); it1 != mapPostData.end(); it1++)
			{
				std::string strKey = it1->first;
				std::string strValue = it1->second;
				
				std::vector<std::string> vecValue = CStringHelper::Split(strValue, ",");
				if (vecValue.size() == 2)
				{
					int nValue = 0;
					char csValue[16] = { 0 };			
					strcmp(vecValue[0].c_str(), "int") == 0 ? sprintf_s(csValue, "%s", vecValue[1].c_str()) : sprintf_s(csValue, "%s", vecValue[0].c_str());
					nValue = atoi(csValue);
					form_ret = curl_formadd(&formpost, &lastptr, CURLFORM_COPYNAME, strKey.c_str(), CURLFORM_PTRCONTENTS, nValue, CURLFORM_END);
				}
				else
				{
					std::wstring wstr = CStringHelper::s2ws(strValue);
					strValue = CStringHelper::UnicodeToUTF8(wstr);
					form_ret = curl_formadd(&formpost, &lastptr, CURLFORM_COPYNAME, strKey.c_str(), CURLFORM_PTRCONTENTS, strValue.c_str(), CURLFORM_END);
				}
			}
			 
			std::multimap<std::string, std::string>::iterator it2;
			for (it2 = mapPostFile.begin(); it2 != mapPostFile.end(); it2++)
			{
				std::string strKey = it2->first;
				std::string strValue = it2->second;

				std::string strFileName = CStringHelper::GetFileName(strValue.c_str());
				std::string strExtName = CStringHelper::GetExtName(strFileName.c_str());
				if (0 == strcmp(strExtName.c_str(), "png"))
				{
					std::wstring wstr = CStringHelper::s2ws(strFileName);
					strFileName = CStringHelper::UnicodeToUTF8(wstr);
					form_ret = curl_formadd(&formpost, &lastptr, CURLFORM_COPYNAME, strKey.c_str(), CURLFORM_FILE, strValue.c_str(), CURLFORM_FILENAME, strFileName.c_str(), CURLFORM_CONTENTTYPE, "image/x-png", CURLFORM_END);
				}
				else if (0 == strcmp(strExtName.c_str(), "jpg"))
				{
					std::wstring wstr = CStringHelper::s2ws(strFileName);
					strFileName = CStringHelper::UnicodeToUTF8(wstr);
					form_ret = curl_formadd(&formpost, &lastptr, CURLFORM_COPYNAME, strKey.c_str(), CURLFORM_FILE, strValue.c_str(), CURLFORM_FILENAME, strFileName.c_str(), CURLFORM_CONTENTTYPE, "image/pjpeg", CURLFORM_END);
				} 
				else if (0 == strcmp(strExtName.c_str(), "bmp"))
				{
					std::wstring wstr = CStringHelper::s2ws(strFileName);
					strFileName = CStringHelper::UnicodeToUTF8(wstr);
					form_ret = curl_formadd(&formpost, &lastptr, CURLFORM_COPYNAME, strKey.c_str(), CURLFORM_FILE, strValue.c_str(), CURLFORM_FILENAME, strFileName.c_str(), CURLFORM_CONTENTTYPE, "image/bmp", CURLFORM_END);
				}
				else
				{
					std::wstring wstr = CStringHelper::s2ws(strFileName);
					strFileName = CStringHelper::UnicodeToUTF8(wstr);
					form_ret = curl_formadd(&formpost, &lastptr, CURLFORM_COPYNAME, strKey.c_str(), CURLFORM_FILE, strValue.c_str(), CURLFORM_FILENAME, strFileName.c_str(), CURLFORM_END);
				}
			}

			ret = curl_easy_setopt(curl, CURLOPT_HTTPPOST, formpost);
			if (ret != CURLE_OK)
			{
				m_strError = curl_easy_strerror(ret);
				nRet = ERR_PARM_ERROR;
				break;
			}
// 			curl_formadd((curl_httppost**)&m_pFormpost,
// 				(curl_httppost**)&m_pLastptr,
// 				CURLFORM_COPYNAME, Param.strkey.c_str(),
// 				CURLFORM_STREAM, Param.value,
// 				CURLFORM_CONTENTSLENGTH, valuesize,
// 				CURLFORM_FILENAME, Param.fileinfo.szfilename,
// 				CURLFORM_CONTENTTYPE, "application/octet-stream",
// 				CURLFORM_END);
// 			curl_formadd(&formpost, &lastptr, CURLFORM_PTRNAME, "reqformat", CURLFORM_PTRCONTENTS, "plain", CURLFORM_END);
// 			curl_formadd(&formpost, &lastptr, CURLFORM_PTRNAME, "file", CURLFORM_PTRCONTENTS, fc, CURLFORM_CONTENTSLENGTH, fclen, CURLFORM_END);
// 
// 			curl_easy_setopt(curl, CURLOPT_URL, URL);
// 			curl_easy_setopt(curl, CURLOPT_HTTPPOST, formpost);
// 			curl_easy_perform(curl);
// 			curl_formfree(formpost);

			ret = curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, false); // if want to use https  
			if (ret != CURLE_OK)
			{
				m_strError = curl_easy_strerror(ret);
				nRet = ERR_PARM_ERROR;
				break;
			}

			ret = curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, false); // set peer and host verify false  
			if (ret != CURLE_OK)
			{
				m_strError = curl_easy_strerror(ret);
				nRet = ERR_PARM_ERROR;
				break;
			}

			ret = curl_easy_setopt(curl, CURLOPT_VERBOSE, 1);
			if (ret != CURLE_OK)
			{
				m_strError = curl_easy_strerror(ret);
				nRet = ERR_PARM_ERROR;
				break;
			}

			ret = curl_easy_setopt(curl, CURLOPT_READFUNCTION, NULL);
			if (ret != CURLE_OK)
			{
				m_strError = curl_easy_strerror(ret);
				nRet = ERR_PARM_ERROR;
				break;
			}

			ret = curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data);
			if (ret != CURLE_OK)
			{
				m_strError = curl_easy_strerror(ret);
				nRet = ERR_PARM_ERROR;
				break;
			}

			ret = curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)this);
			if (ret != CURLE_OK)
			{
				m_strError = curl_easy_strerror(ret);
				nRet = ERR_PARM_ERROR;
				break;
			}

			ret = curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1);
			if (ret != CURLE_OK)
			{
				m_strError = curl_easy_strerror(ret);
				nRet = ERR_PARM_ERROR;
				break;
			}

			ret = curl_easy_setopt(curl, CURLOPT_HEADER, 0);
			if (ret != CURLE_OK)
			{
				m_strError = curl_easy_strerror(ret);
				nRet = ERR_PARM_ERROR;
				break;
			}

			ret = curl_easy_perform(curl);
			if (ret != CURLE_OK)
			{
				m_strError = curl_easy_strerror(ret);
				nRet = ERR_PARM_ERROR;
				break;
			}

			strResponse = m_strResponse;

			long nState = 0;
			ret = curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &nState);
			if (ret == CURLE_OK)
			{
				nStatus = (int)nState;
			}

			// release curl  
			curl_easy_cleanup(curl);
			m_strError = curl_easy_strerror(ret);
		}
	} while (false);

	if (headers)
	{
		curl_slist_free_all(headers);
	}

	if (m_pFile)
	{
		fclose(m_pFile);
		m_pFile = NULL;
	}

	return nRet;
}
