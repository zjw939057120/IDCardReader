#pragma once
#include "string"

static const std::string base64_chars =
"ABCDEFGHIJKLMNOPQRSTUVWXYZ"
"abcdefghijklmnopqrstuvwxyz"
"0123456789+/";

//多线程处理
void MultithreadHandle(void);

//Web服务
void HttpServer(void);

//读取身份证信息
std::string ReadIDCard(int iPort = 1001);

//wchar_t转char
std::string WcharToChar(const wchar_t* wp);

//根据性别代码获得性别字符串
std::string GetSexByCodeA(const char* pi_pszSexCode);

//根据民族代码获得民族字符串
std::string GetNationByCodeA(const char* pi_pszNationCode);

//移除字符串空格
char * trim(char * ptr);

std::string trim(std::string &s);

//检查端口占用情况
bool check_port(const char *host, int ports);
//base64编码
std::string base64_encode(const char * bytes_to_encode, unsigned int in_len);

// 普通sting类型 转UTF-8编码格式字符串
std::string string_To_UTF8(const std::string & str);

// UTF-8编码格式字符串  转普通sting类型
std::string UTF8_To_string(const std::string & str);