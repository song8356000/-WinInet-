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

	// ���ַ�ת���ַ�
	static std::wstring s2ws(const char* pStr,int len);  

	static std::wstring s2ws(const std::string& strA); 

	// ���ַ�ת���ַ�
	static std::string ws2s(const wchar_t* pStr,int len);

	static std::string ws2s(const std::wstring& wstr);  

	// UTF8תUnicode
	static std::wstring UTF8ToUnicode(const std::string& str);

	// UTF8תAnsi
	static std::string UTF8ToAnsi(const std::string& str);

	// UnicodeתUTF8
	static std::string UnicodeToUTF8(const std::wstring& wstr);

	// �ַ����ָ��
	static std::vector<std::string> Split(std::string str, std::string strDelimiter);

	static std::vector<std::wstring> Split(std::wstring wstr, std::wstring wstrDelimiter);

	// ��ȡ��ǰִ�г����·��
	static std::string	GetCurrentPathA();

	static std::wstring GetCurrentPathW();

	// ��ȡ��ǰִ�г�������
	static std::string	GetCurrentProcessNameA();

	static std::wstring GetCurrentProcessNameW();

	// ��ȡ����·��
	static std::string	GetDeskPathA();

	static std::wstring GetDeskPathW();

	// ��ȡ�����ļ�Ŀ¼
	static std::string GetSaveDirPathA(HWND hWnd = NULL);

	// ��ȡ�����ļ�Ŀ¼
	static std::wstring GetSaveDirPathW(HWND hWnd = NULL);
	
	// ��ʽ��·�� : ��'/'��Ϊ'\';
	static bool FormatPathA(std::string & strPath);

	static bool FormatPathW(std::wstring & wstrPath);

	// �滻�ַ�������ĵ�ĳЩ�ַ�
	static std::string StrReplace(std::string strRes, std::string pattern, std::string dstPattern,int count=-1);

	std::string& Trim(std::string& str, bool bHead = true, bool bTail = true);

	std::wstring& Trim(std::wstring& str, bool bHead = true, bool bTail = true);

	// ��ȡ�ļ���
	/*
	bExt
		[in]�Ƿ���Ҫ��չ����true:����չ����false:������չ��
	*/
	static std::string	GetFileName(const char * filepath, bool bExt = true);

	// ��ȡ�ļ�����չ��
	static std::string	GetExtName(const char * filepath);

	 /* ͼƬתBase64 
    DataByte 
        [in]��������ݳ���,���ֽ�Ϊ��λ 
    */  
    static std::string ImageToBase64(const unsigned char* Data,int DataByte);  
   
	/* Base64תͼƬ 
    DataByte 
        [in]��������ݳ���,���ֽ�Ϊ��λ 
    OutByte 
        [out]��������ݳ���,���ֽ�Ϊ��λ,�벻Ҫͨ������ֵ���� 
        ������ݵĳ��� 
    */  
    static std::string Base64ToImage(const char* Data,int DataByte,int& OutByte);

	// ���  �����ǵ��� delete[] �ͷŷ��ص�char *,���������ڴ�й©
	static char * Quadrature(const char *data_a, const char *data_b);

	// ���ַ����е���ĸתΪСд
	static std::string tolower(const std::string & src);

	// ���ַ����е���ĸתΪ��д
	static std::string toupper(const std::string & src);

	// �ж��ļ����Ƿ����
	static bool dirExists(const std::string& strDir);

	// �ж��ļ��Ƿ����
	static bool isFileExists(const std::string& strPath);

	// �����ļ���
	static bool CreateDirByPath(const std::string& strPath, bool bHide = false);

	// ʱ���ת���ɱ�׼ʱ��
	static std::string StampTime2StandardString(INT64 stampTime, bool bDate = true);

	// ɾ����Ŀ¼�µ������ļ�
	static bool ClearDir(const std::string & strDirPath);
};

