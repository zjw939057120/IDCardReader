#pragma once
#include "string"

static const std::string base64_chars =
"ABCDEFGHIJKLMNOPQRSTUVWXYZ"
"abcdefghijklmnopqrstuvwxyz"
"0123456789+/";

//���̴߳���
void MultithreadHandle(void);

//Web����
void HttpServer(void);

//��ȡ���֤��Ϣ
std::string ReadIDCard(int iPort = 1001);

//wchar_tתchar
std::string WcharToChar(const wchar_t* wp);

//�����Ա�������Ա��ַ���
std::string GetSexByCodeA(const char* pi_pszSexCode);

//������������������ַ���
std::string GetNationByCodeA(const char* pi_pszNationCode);

//�Ƴ��ַ����ո�
char * trim(char * ptr);

std::string trim(std::string &s);

//���˿�ռ�����
bool check_port(const char *host, int ports);
//base64����
std::string base64_encode(const char * bytes_to_encode, unsigned int in_len);

// ��ͨsting���� תUTF-8�����ʽ�ַ���
std::string string_To_UTF8(const std::string & str);

// UTF-8�����ʽ�ַ���  ת��ͨsting����
std::string UTF8_To_string(const std::string & str);