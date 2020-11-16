#include "StringHelper.h"
#include <time.h>

CStringHelper::CStringHelper(void)
{

}

CStringHelper::~CStringHelper(void)
{

}

std::wstring CStringHelper::s2ws(const char* pStr,int ilen)  
{  
	if (NULL == pStr || 0 == ilen)
	{
		return L"";
	}

	int iChars = MultiByteToWideChar(CP_ACP , 0 , pStr , ilen , NULL , 0);   
	if (ilen == -1)
	{
		--iChars;   
	}

	if (iChars == 0)
	{
		return L"";  
	}

	std::wstring wstrBuf;  
	wstrBuf.resize(iChars);   
	::MultiByteToWideChar(CP_ACP, 0, pStr, ilen, const_cast<wchar_t*>(wstrBuf.c_str()), iChars);   

	return wstrBuf ;  
}  

std::wstring CStringHelper::s2ws(const std::string& strA)  
{  
	const char* pStr = strA.c_str();  
	int	ilen	 = strA.length(); 
	return s2ws(pStr, ilen);  
}  

std::string CStringHelper::ws2s( const wchar_t* pStr,int ilen)  
{      
	if (NULL == pStr || 0 == ilen)
	{
		return "";
	}  

	int iChars = ::WideCharToMultiByte( CP_ACP , 0, pStr , ilen, NULL, 0, NULL, NULL);   
	if (ilen == -1)  
	{
		-- iChars;   
	}

	if (iChars == 0)  
	{
		return "";  
	}

	std::string strBuf ;  
	strBuf.resize(iChars);  
	::WideCharToMultiByte(CP_ACP, 0, pStr, ilen, const_cast<char*>(strBuf.c_str()), iChars, NULL, NULL);   

	return strBuf ;   
}  

std::string CStringHelper::ws2s(const std::wstring& wstr)  
{  
	const wchar_t* pStr = wstr.c_str();  
	int ilen = wstr.length();  
	return ws2s(pStr, ilen);  
}  

std::wstring CStringHelper::UTF8ToUnicode(const std::string& str)
{
	int  len = 0;
	len = str.length();
	int  unicodeLen = ::MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, NULL, 0);  
	
	wchar_t* pUnicode;  
	pUnicode = new  wchar_t[unicodeLen+1];  
	memset(pUnicode, 0, (unicodeLen+1)*sizeof(wchar_t));  
	::MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, (LPWSTR)pUnicode, unicodeLen);  
	
	std::wstring  wstrBuf;  
	wstrBuf = (wchar_t*)pUnicode;
	
	delete[] pUnicode; 
	pUnicode = NULL;

	return  wstrBuf;  
}

// UTF8转Ansi
std::string CStringHelper::UTF8ToAnsi(const std::string& str)
{
	std::string strAnsi;
	std::wstring wstr = UTF8ToUnicode(str);
	strAnsi = ws2s(wstr);

	return strAnsi;
}

std::string CStringHelper::UnicodeToUTF8(const std::wstring& wstr)
{
	char* pBuf;
	int iLen;
	
	iLen = WideCharToMultiByte(CP_UTF8,	0, wstr.c_str(),	-1,	NULL, 0, NULL, NULL);
	pBuf = new char[iLen + 1];
	memset(( void* )pBuf, 0, sizeof( char ) * ( iLen + 1 ));
	::WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), -1, pBuf, iLen, NULL, NULL);
	
	std::string strBuf;
	strBuf = pBuf;
	
	delete[] pBuf;
	pBuf = NULL;

	return strBuf;
}

// 字符串分割函数
std::vector<std::string> CStringHelper::Split(std::string strContext, std::string strDelimiter)
{
	std::vector<std::string> vecResult;
	
	if (strContext.empty())
	{
		return vecResult;
	}

	if (strDelimiter.empty())
	{
		vecResult.push_back(strContext);
		return vecResult;
	}
	
	strContext += strDelimiter;
	
	// 扩展字符串以方便操作
	int iSize = strContext.size();
	for (int i = 0; i < iSize; i++)
	{
		int iPos = strContext.find(strDelimiter, i);
		if(iPos < iSize)
		{
			std::string strElement = strContext.substr(i, iPos - i);
			vecResult.push_back(strElement);
			i = iPos + strDelimiter.size() - 1;
		}
	}

	return vecResult;
} 

// 字符串分割函数
std::vector<std::wstring> CStringHelper::Split(std::wstring wstrContext, std::wstring wstrDelimiter)
{
	std::vector<std::wstring> vecResult;

	if (wstrContext.empty())
	{
		return vecResult;
	}

	if (wstrDelimiter.empty())
	{
		vecResult.push_back(wstrContext);
		return vecResult;
	}

	wstrContext += wstrDelimiter;

	// 扩展字符串以方便操作
	int iSize = wstrContext.size();
	for (int i = 0; i < iSize; i++)
	{
		int iPos = wstrContext.find(wstrDelimiter, i);
		if(iPos < iSize)
		{
			std::wstring wstrElement = wstrContext.substr(i, iPos - i);
			vecResult.push_back(wstrElement);
			i = iPos + wstrDelimiter.size() - 1;
		}
	}

	return vecResult;
}

std::string CStringHelper::GetCurrentPathA()
{
	char czPath[MAX_PATH] = {0};  
	std::string strPath;
	
	::GetModuleFileNameA(NULL, (LPSTR)czPath, sizeof(czPath));
	strPath = czPath;
	
	int iPos = strPath.find_last_of("\\");
	strPath	 = strPath.substr(0, iPos + 1);

	return strPath;
}

std::wstring CStringHelper::GetCurrentPathW()
{
	wchar_t wczPath[MAX_PATH] = {0};  
	std::wstring wstrPath;

	::GetModuleFileNameW(NULL, (LPWSTR)wczPath, sizeof(wczPath));
	wstrPath = wczPath;

	int iPos = wstrPath.find_last_of(L"\\");
	wstrPath = wstrPath.substr(0, iPos + 1);

	return wstrPath;
}

std::string	CStringHelper::GetDeskPathA()
{
	char czPath[MAX_PATH] = {0};
	std::string strPath; 

	::SHGetSpecialFolderPathA(0, czPath, CSIDL_DESKTOPDIRECTORY, 0);
	strPath = czPath;
	strPath += "\\";

	return strPath;
}

// 获取当前执行程序名称
std::string	CStringHelper::GetCurrentProcessNameA()
{
	char czPath[MAX_PATH] = {0};  
	std::string strPath;

	::GetModuleFileNameA(NULL, (LPSTR)czPath, sizeof(czPath));
	strPath = czPath;

	int iPos = strPath.find_last_of("\\");
	strPath	 = strPath.substr(iPos + 1, strPath.length());

	return strPath;
}

std::wstring CStringHelper::GetCurrentProcessNameW()
{
	wchar_t wczPath[MAX_PATH] = {0};  
	std::wstring wstrPath;

	::GetModuleFileNameW(NULL, (LPWSTR)wczPath, sizeof(wczPath));
	wstrPath = wczPath;

	int iPos = wstrPath.find_last_of(L"\\");
	wstrPath = wstrPath.substr(iPos + 1, wstrPath.length());

	return wstrPath;
}

std::wstring CStringHelper::GetDeskPathW()
{
	wchar_t wczPath[MAX_PATH] = {0};
	std::wstring wstrPath; 

	::SHGetSpecialFolderPathW(0, wczPath, CSIDL_DESKTOPDIRECTORY, 0);
	wstrPath = wczPath;
	wstrPath += L"\\";

	return wstrPath;
}

// 获取保存文件目录
std::string CStringHelper::GetSaveDirPathA(HWND hWnd)
{
	char szDefaultDir[MAX_PATH] = { 0 };
	char szPath[MAX_PATH] = { 0 };
	std::wstring  strSelDir = L"";

	std::string str = CStringHelper::GetDeskPathA();
	sprintf_s(szDefaultDir, "%s", str.c_str());

	BROWSEINFOA brInfo;
	ITEMIDLIST *pItem;

	brInfo.hwndOwner = hWnd;
	brInfo.pidlRoot = 0;
	brInfo.pszDisplayName = 0;
	brInfo.lpszTitle = "选择路径";
	brInfo.ulFlags = BIF_RETURNONLYFSDIRS | BIF_USENEWUI;
	brInfo.lpfn = NULL;
	brInfo.iImage = 0;
	brInfo.lParam = long(szDefaultDir);

	pItem = SHBrowseForFolderA(&brInfo);

	if (pItem != NULL)
	{
		if (SHGetPathFromIDListA(pItem, szPath))
		{
			str = szPath;
			str += "\\";
			return str;
		}
	}

	return "";
}

// 获取保存文件目录
std::wstring CStringHelper::GetSaveDirPathW(HWND hWnd)
{
	wchar_t szDefaultDir[MAX_PATH] = { 0 };
	wchar_t szPath[MAX_PATH] = { 0 };
	std::wstring  strSelDir = L"";

	std::wstring wstr = CStringHelper::GetDeskPathW();
	swprintf_s(szDefaultDir, L"%s", wstr.c_str());

	BROWSEINFO brInfo;
	ITEMIDLIST *pItem;

	brInfo.hwndOwner = hWnd;
	brInfo.pidlRoot = 0;
	brInfo.pszDisplayName = 0;
	brInfo.lpszTitle = L"选择路径";
	brInfo.ulFlags = 0;
	brInfo.lpfn = NULL;
	brInfo.iImage = 0;
	brInfo.lParam = long(szDefaultDir);

	pItem = SHBrowseForFolder(&brInfo);

	if (pItem != NULL)
	{
		if (SHGetPathFromIDList(pItem, szPath))
		{
			wstr = szPath;
			return wstr;
		}
	}

	return L"";
}


// 格式化路径
bool CStringHelper::FormatPathA(std::string & strPath)
{
	char cz[MAX_PATH] = { 0 };
	std::string strInPath = strPath;
	strcat_s(cz, strInPath.c_str());
	
	for (size_t i = 0; i< strlen(cz); i++)
	{
		if (cz[i] == '/')
		{
			cz[i] = '\\';
		}
	}

	strPath = cz;

	return true;
}

bool CStringHelper::FormatPathW(std::wstring & wstrPath)
{
	wchar_t wcz[MAX_PATH] = { 0 };
	std::wstring wstrInPath = wstrPath;
	wcscat_s(wcz, wstrInPath.c_str());

	for (size_t i = 0; i< wcslen(wcz); i++)
	{
		if (wcz[i] == L'/')
		{
			wcz[i] = L'\\';
		}
	}

	wstrPath = wcz;

	return true;
}

std::string& CStringHelper::Trim(std::string& str, bool bHead, bool bTail)
{
	if (str.empty())
	{
		return str;
	}
	static const std::string delims = " \t\r";
	if (bTail)
		str.erase(str.find_last_not_of(delims) + 1);
	if (bHead)
		str.erase(0, str.find_first_not_of(delims));
	return str;
}

std::wstring& CStringHelper::Trim(std::wstring& str, bool bHead, bool bTail)
{
	if (str.empty())
	{
		return str;
	}
	static const std::wstring delims = L" \t\r";
	if (bTail)
		str.erase(str.find_last_not_of(delims) + 1);
	if (bHead)
		str.erase(0, str.find_first_not_of(delims));
	return str;
}

// 获取文件名
std::string	CStringHelper::GetFileName(const char * filepath, bool bExt)
{
	if (NULL != filepath)
	{
		std::string strFilePath = filepath;	
		FormatPathA(strFilePath);

		int iPos = strFilePath.find_last_of("\\");
		strFilePath = strFilePath.substr(iPos + 1, strFilePath.length());
		if (!bExt)
		{
			iPos = -1;
			iPos = strFilePath.find_last_of(".");
			strFilePath = strFilePath.substr(0, iPos);
		}

		return strFilePath;
	}

	return "";
}

// 获取文件名扩展名
std::string	CStringHelper::GetExtName(const char * filepath)
{
	if (NULL != filepath)
	{
		std::string strFilePath = filepath;	
		FormatPathA(strFilePath);

		int iPos = strFilePath.find_last_of(".");
		strFilePath = strFilePath.substr(iPos + 1, strFilePath.length()-1);

		return strFilePath;
	}

	return "";
}

// 替换字符串里面的的某些字符
std::string CStringHelper::StrReplace(std::string strRes, std::string pattern, std::string dstPattern,int count)
{
	std::string retStr="";
	std::string::size_type pos;
	int i=0,l_count=0,szStr=strRes.length();
	if(-1 == count) // replace all
		count = szStr;
	for(i=0; i<szStr; i++)
	{        
		if(std::string::npos == (pos=strRes.find(pattern,i)))  break;
		if(pos < szStr)
		{            
			retStr += strRes.substr(i,pos-i) + dstPattern;
			i=pos+pattern.length()-1;
			if(++l_count >= count)
			{
				i++;
				break;
			}
		}
	}
	
	retStr += strRes.substr(i);
	return retStr;
}

std::string CStringHelper::ImageToBase64(const unsigned char* Data,int DataByte)  
{  
	//编码表  
	const char EncodeTable[]="ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";  
	//返回值  
	std::string strEncode;  
	unsigned char Tmp[4]={0};  
	int LineLength=0;  
	for(int i=0;i<(int)(DataByte / 3);i++)  
	{  
		Tmp[1] = *Data++;  
		Tmp[2] = *Data++;  
		Tmp[3] = *Data++;  
		strEncode+= EncodeTable[Tmp[1] >> 2];  
		strEncode+= EncodeTable[((Tmp[1] << 4) | (Tmp[2] >> 4)) & 0x3F];  
		strEncode+= EncodeTable[((Tmp[2] << 2) | (Tmp[3] >> 6)) & 0x3F];  
		strEncode+= EncodeTable[Tmp[3] & 0x3F];  
		if(LineLength+=4,LineLength==76) {strEncode+="\r\n";LineLength=0;}  
	}  
	//对剩余数据进行编码  
	int Mod=DataByte % 3;  
	if(Mod==1)  
	{  
		Tmp[1] = *Data++;  
		strEncode+= EncodeTable[(Tmp[1] & 0xFC) >> 2];  
		strEncode+= EncodeTable[((Tmp[1] & 0x03) << 4)];  
		strEncode+= "==";  
	}  
	else if(Mod==2)  
	{  
		Tmp[1] = *Data++;  
		Tmp[2] = *Data++;  
		strEncode+= EncodeTable[(Tmp[1] & 0xFC) >> 2];  
		strEncode+= EncodeTable[((Tmp[1] & 0x03) << 4) | ((Tmp[2] & 0xF0) >> 4)];  
		strEncode+= EncodeTable[((Tmp[2] & 0x0F) << 2)];  
		strEncode+= "=";  
	}  

	return strEncode;  
}  

std::string CStringHelper::Base64ToImage(const char* Data,int DataByte,int& OutByte)  
{  
	//解码表  
	const char DecodeTable[] =  
	{  
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  
		62, // '+'  
		0, 0, 0,  
		63, // '/'  
		52, 53, 54, 55, 56, 57, 58, 59, 60, 61, // '0'-'9'  
		0, 0, 0, 0, 0, 0, 0,  
		0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12,  
		13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, // 'A'-'Z'  
		0, 0, 0, 0, 0, 0,  
		26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38,  
		39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, // 'a'-'z'  
	};  
	//返回值  
	std::string strDecode;  
	int nValue;  
	int i= 0;  
	while (i < DataByte)  
	{  
		if (*Data != '\r' && *Data!='\n')  
		{  
			nValue = DecodeTable[*Data++] << 18;  
			nValue += DecodeTable[*Data++] << 12;  
			strDecode+=(nValue & 0x00FF0000) >> 16;  
			OutByte++;  
			if (*Data != '=')  
			{  
				nValue += DecodeTable[*Data++] << 6;  
				strDecode+=(nValue & 0x0000FF00) >> 8;  
				OutByte++;  
				if (*Data != '=')  
				{  
					nValue += DecodeTable[*Data++];  
					strDecode+=nValue & 0x000000FF;  
					OutByte++;  
				}  
			}  
			i += 4;  
		}  
		else// 回车换行,跳过  
		{  
			Data++;  
			i++;  
		}  
	}  

	return strDecode;  
}  

struct BigDataMutliplie  
{  
private:  
	char data_a[100];  
	char data_b[100];  
	int len_a;  
	int len_b;  
	bool negative;  
	bool detect_data()  
	{  
		len_a = strlen(data_a);  
		len_b = strlen(data_b);  
		if (len_a == 0 || len_b == 0)  
		{  
			return false;  
		}  
		for (int i = 0; i < len_a; i++)  
		{  
			if (!(data_a[i] >= '0'&&data_a[i] <= '9'))  
			{  
				return false;  
			}  
		}  

		for (int i = 0; i < len_b; i++)  
		{  
			if (!(data_b[i] >= '0'&&data_b[i] <= '9'))  
			{  
				return false;  
			}  
		}  

		return true;
	}  
public:  
	BigDataMutliplie()  
	{  
		memset(data_a, 0, sizeof(data_a));  
		memset(data_b, 0, sizeof(data_b));  
		len_a = 0;  
		len_b = 0;  
	}  
	bool init_data(const char *data_a, const char *data_b)  
	{  
		this->negative = false;  
		if (!data_a || !data_b)  
		{  
			return false;  
		}  
		if (*data_a == '-' || *data_a == '+')  
		{  
			strcpy_s(this->data_a, data_a + 1);  
			if (*data_a == '-')  
			{  
				negative = !negative;  
			}  
		}  
		else  
		{  
			strcpy_s(this->data_a, data_a);  
		}  
		if (*data_b == '-' || *data_b == '+')  
		{  
			strcpy_s(this->data_b, data_b + 1);  
			if (*data_b == '-')  
			{  
				negative = !negative;  
			}  
		}  
		else  
		{  
			strcpy_s(this->data_b, data_b);  
		}  
		
		return true;
	}  
	char * multiplie_ab()  
	{  
		if (!detect_data())  
		{  
			return NULL;  
		}  
		int  * int_res = new int[(len_a + len_b)*sizeof(int)];//两个数相乘最大不会超过两个数的个数相加99*99  
		char  * str_res = new char[(len_a + len_b + 2)*sizeof(char)]; //多申请两个字符的空间 一个存符号，一个存'\0'  
		memset(str_res, 0, (len_a + len_b + 2)*sizeof(char));  
		memset(int_res, 0, (len_a + len_b)*sizeof(int));  
		//采取累乘的方式然后再统一进位  
		for (int i = 0; i < len_a; i++)  
			for (int j = 0; j < len_b; j++)  
			{  
				int_res[i + j + 1] += (data_a[i] - '0')*(data_b[j] - '0'); //第一位预留出来用于保存符进位  
			}  

			//处理进位  
			for (int index = len_a + len_b + 2 - 1; index >= 0; index--)  
			{  
				if (int_res[index] >= 10)  
				{  
					int_res[index - 1] += int_res[index] / 10;  
					int_res[index] = int_res[index] % 10;  
				}  

			}  

			int j = 0, i = 0;  

			while (int_res[j] == 0) // 找到开始不为0的位置  
			{  
				j++;  
			}  

			if (negative)  
			{  
				str_res[i++] = '-';  
			}  
			//int_res 数组是从0-len_a + len_b   
			//str_res 是从除去符号位开始到len_a + len_b + 1  
			for (; i < (len_a + len_b + 1) && j < (len_a + len_b); i++, j++)  
			{  
				str_res[i] = int_res[j] + '0';  
			}  
			str_res[len_a + len_b + 1] = '\0';  
			delete[] int_res;  
			return str_res;  
	}  
}; 

char * CStringHelper::Quadrature(const char *data_a, const char *data_b)
{
	BigDataMutliplie data;
	data.init_data(data_a, data_b);  
	return data.multiplie_ab();  
}


std::string CStringHelper::tolower(const std::string & src)
{
	char *pBuf = (char*)malloc(src.length() + 1);
	char *pTemp = pBuf;
	sprintf_s(pTemp, src.length() + 1, "%s", src.c_str());
	pTemp[src.length()] = '\0';
	std::string str;

	for (size_t i = 0; i < src.length() + 1; i++)
	{
		if ((pTemp[i] >= 'A') && (pTemp[i] <= 'Z'))
			pTemp[i] = pTemp[i] + ('a' - 'A');
		str += pTemp[i];
	}

	if (pBuf)
	{
		free(pBuf);
	}

	return str;
}

std::string CStringHelper::toupper(const std::string & src)
{
	char *pBuf = (char*)malloc(src.length() + 1);
	char *pTemp = pBuf;
	sprintf_s(pTemp, src.length() + 1, "%s", src.c_str());
	pTemp[src.length()] = '\0';
	std::string str;

	for (size_t i = 0; i < src.length() + 1; i++)
	{
		if ((pTemp[i] >= 'a') && (pTemp[i] <= 'z'))
			pTemp[i] = pTemp[i] + ('A' - 'a');
		str += pTemp[i];
	}

	if (pBuf)
	{
		free(pBuf);
	}

	return str;
}

bool CStringHelper::dirExists(const std::string& strDir)
{
	int ftyp = _access(strDir.c_str(), 0);

	if (0 == ftyp)
		return true;   // this is a directory!  
	else
		return false;    // this is not a directory!  
}

// 判断文件是否存在
bool CStringHelper::isFileExists(const std::string& strPath)
{
	int ftyp = _access(strPath.c_str(), 0);

	if (0 == ftyp)
		return true;   
	else
		return false;    
}

bool CStringHelper::CreateDirByPath(const std::string& strPath, bool bHide)
{
	char *tag;
	char *fileName = (char*)malloc(strPath.length() + 1);
	sprintf_s(fileName, strPath.length() + 1, "%s", strPath.c_str());
	fileName[strPath.length()] = '\0';

	for (tag = fileName; *tag; tag++)
	{
		if (*tag == '\\')
		{
			char buf[1000], path[1000];
			strcpy_s(buf, fileName);
			buf[strlen(fileName) - strlen(tag) + 1] = NULL;
			strcpy_s(path, buf);
			if (_access(path, 6) == -1)
			{
				_mkdir(path);
				if (bHide)
				{
					SetFileAttributesA(path, FILE_ATTRIBUTE_HIDDEN);
				}
			}
		}
	}

	return true;
}

// 时间戳转换成标准时间
std::string CStringHelper::StampTime2StandardString(INT64 stampTime, bool bDate)
{
	time_t tick = (time_t)stampTime / 1000;
	struct tm tm;
	char s[100];

	localtime_s(&tm, &tick);
	if (bDate)
	{
		strftime(s, sizeof(s), "%Y-%m-%d %H:%M:%S", &tm);
	}
	else
	{
		strftime(s, sizeof(s), "%Y-%m-%d", &tm);
	}

	std::string strTime = s;

	return strTime;
}

bool CStringHelper::ClearDir(const std::string & strDirPath)
{
	char szFind[MAX_PATH];
	WIN32_FIND_DATAA FindFileData;

	strcpy_s(szFind, strDirPath.c_str());
	if (strDirPath.length() - 1 != strDirPath.find_last_of("\\"))
	{
		strcat_s(szFind, "\\*.*");
	}
	else
	{
		strcat_s(szFind, "*.*");
	}

	HANDLE hFind = ::FindFirstFileA(szFind, &FindFileData);
	if (INVALID_HANDLE_VALUE == hFind)    return false;

	while (true)
	{
		if (FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
		{
			if (FindFileData.cFileName[0] != '.')
			{
				char szFile[MAX_PATH];
				strcpy_s(szFile, strDirPath.c_str());
				if (strDirPath.length() - 1 != strDirPath.find_last_of("\\"))
				{
					strcat_s(szFile, "\\");
				}
				strcat_s(szFile, (char*)(FindFileData.cFileName));
				ClearDir(szFile);
			}
		}
		else
		{
			std::string strFileName = strDirPath;
			if (strFileName.length() - 1 != strFileName.find_last_of("\\"))
			{
				strFileName += "\\";
			}
			strFileName += FindFileData.cFileName;
			BOOL bRet = DeleteFileA(strFileName.c_str());
			if (!bRet)
			{
				DWORD dwError = GetLastError();
			}
		}
		if (!FindNextFileA(hFind, &FindFileData))    break;
	}
	FindClose(hFind);

	return true;
}