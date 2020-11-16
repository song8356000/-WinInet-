#pragma once

#include <Windows.h>
#include <string>  
#include <vector>
#include <ShlObj.h>
#include <assert.h>  
#include <io.h>
#include <direct.h>

class CStringHelper
{
public:
	CStringHelper(void);
	~CStringHelper(void);

public:

	// 多字符转宽字符
	static std::wstring s2ws(const char* pStr,int len);  

	static std::wstring s2ws(const std::string& strA); 

	// 宽字符转多字符
	static std::string ws2s(const wchar_t* pStr,int len);

	static std::string ws2s(const std::wstring& wstr);  

	// UTF8转Unicode
	static std::wstring UTF8ToUnicode(const std::string& str);

	// UTF8转Ansi
	static std::string UTF8ToAnsi(const std::string& str);

	// Unicode转UTF8
	static std::string UnicodeToUTF8(const std::wstring& wstr);

	// 字符串分割函数
	static std::vector<std::string> Split(std::string str, std::string strDelimiter);

	static std::vector<std::wstring> Split(std::wstring wstr, std::wstring wstrDelimiter);

	// 获取当前执行程序的路径
	static std::string	GetCurrentPathA();

	static std::wstring GetCurrentPathW();

	// 获取当前执行程序名称
	static std::string	GetCurrentProcessNameA();

	static std::wstring GetCurrentProcessNameW();

	// 获取桌面路径
	static std::string	GetDeskPathA();

	static std::wstring GetDeskPathW();

	// 获取保存文件目录
	static std::string GetSaveDirPathA(HWND hWnd = NULL);

	// 获取保存文件目录
	static std::wstring GetSaveDirPathW(HWND hWnd = NULL);
	
	// 格式化路径 : 把'/'改为'\';
	static bool FormatPathA(std::string & strPath);

	static bool FormatPathW(std::wstring & wstrPath);

	// 替换字符串里面的的某些字符
	static std::string StrReplace(std::string strRes, std::string pattern, std::string dstPattern,int count=-1);

	std::string& Trim(std::string& str, bool bHead = true, bool bTail = true);

	std::wstring& Trim(std::wstring& str, bool bHead = true, bool bTail = true);

	// 获取文件名
	/*
	bExt
		[in]是否需要扩展名：true:带扩展名，false:不带扩展名
	*/
	static std::string	GetFileName(const char * filepath, bool bExt = true);

	// 获取文件名扩展名
	static std::string	GetExtName(const char * filepath);

	 /* 图片转Base64 
    DataByte 
        [in]输入的数据长度,以字节为单位 
    */  
    static std::string ImageToBase64(const unsigned char* Data,int DataByte);  
   
	/* Base64转图片 
    DataByte 
        [in]输入的数据长度,以字节为单位 
    OutByte 
        [out]输出的数据长度,以字节为单位,请不要通过返回值计算 
        输出数据的长度 
    */  
    static std::string Base64ToImage(const char* Data,int DataByte,int& OutByte);

	// 求积  用完后记得用 delete[] 释放返回的char *,否则会造成内存泄漏
	static char * Quadrature(const char *data_a, const char *data_b);

	// 将字符串中的字母转为小写
	static std::string tolower(const std::string & src);

	// 将字符串中的字母转为大写
	static std::string toupper(const std::string & src);

	// 判断文件夹是否存在
	static bool dirExists(const std::string& strDir);

	// 判断文件是否存在
	static bool isFileExists(const std::string& strPath);

	// 创建文件夹
	static bool CreateDirByPath(const std::string& strPath, bool bHide = false);

	// 时间戳转换成标准时间
	static std::string StampTime2StandardString(INT64 stampTime, bool bDate = true);

	// 删除该目录下的所有文件
	static bool ClearDir(const std::string & strDirPath);
};

