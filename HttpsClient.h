#pragma once

#include <map>
#include <string>
#include "../Include/curl/curl.h"

// Http get，post数据成功
#define HTTP_GET_OR_POST_SUCCESS_NOR				200

// Http get获取post数据文件
#define HTTP_GET_OR_POST_SUCCESS_DOWN_FILE			201

// VIN码拼写不符合编码要求
#define HTTP_VIN_INVALID					        400

// Access token失效
#define HTTP_ACCESSTOKEN_INVALID					401

// 需要升级到高级版才能用
#define HTTP_NEED_ADVANCED_VERSION					403

// 地址无效
#define HTTP_REQUEST_ADDR_NOT_FIND					404

// 服务器错误
#define HTTP_SERVER_INTETFACE_ERROR					500

// 请求超时
#define HTTP_REQUEST_TIME_OUT						0

// 服务器连接失败
#define HTTP_SERVER_CONNECT_FAIL					2   

// 停止获取数据（断开连接）
#define HTTP_SERVER_DISCONNECT						3


typedef enum _HttpsClentErrorCode
{
	ERR_SUCCESS = 0,
	ERR_SOCKET_ERROR,
	ERR_PARM_ERROR,
	ERR_CURLRECV_ERROR,
	ERR_CURLSEND_ERROR,
	ERR_AGAGIN,
	ERR_CURLINIT_ERROR
}HttpsErrorCode;


// 数据下载进度
typedef void(*PFDataProgressCallBack)(INT64 & i64Downloaded, INT64 & i64CountSize, void * pUserData);


class CHttpsClient
{
public:
	CHttpsClient();
	~CHttpsClient();

	int HttpsGet(std::string strUrl, std::string &strResponse, std::string & strHeader, std::vector<std::string> & vecHeaders, int & nStatus, const char * pSavePath = NULL);

	int HttpsPost(std::string strUrl, std::string strPostData, std::vector<std::string> & vecHeaders, string &strResponse, int & nStatus, const char * pSavePath = NULL);

	int HttpsPostEX(std::string strUrl, std::vector<std::string> & vecHeaders, std::multimap<std::string, std::string> mapPostData, std::multimap<std::string, std::string> mapPostFile, string &strResponse, int & nStatus, const char * pSavePath = NULL);

	void RegiterCallBack(PFDataProgressCallBack cb, void *pUserData);

	void StopRecvData();

	std::string GetError();

	bool IsStopRecvData();

	size_t WirteData(void *ptr, size_t size, size_t nmemb);


private:

	void InitParam(const char * pSavePath);

	bool PostDataAndFile();

private:

	PFDataProgressCallBack m_cb;

	void *m_pUserData;

	bool m_bStopRecvData;

	INT64 m_i64DataSize;
	
	INT64 m_i64CountSize;

	std::string m_strError;

	std::string m_strSavePath;

	std::string m_strResponse;

	FILE*		m_pFile;
};

