#include "stdafx.h"
#include "util.h"
#include <process.h>
#include "httplib.h"
#include <sstream>
#include <fstream>

void MultithreadHandle(void)
{
	HANDLE   hThread;
	unsigned   threadID;
	hThread = (HANDLE)_beginthreadex(NULL, 0, (unsigned(__stdcall *)(void *))HttpServer, NULL, 0, &threadID);
}

void HttpServer(void)
{
	using namespace httplib;

	Server svr;

	svr.set_error_handler([](const auto& req, auto& res) {
		auto fmt = "{\"code\":0,\"msg\":\"Error Status: %d\"}";
		char buf[BUFSIZ];
		snprintf(buf, sizeof(buf), fmt, res.status);
		res.set_header("Access-Control-Allow-Origin", "*");
		res.set_content(buf, "application/json;charset=utf-8");
	});

	svr.Get("/", [](const Request& req, Response& res) {
		res.set_header("Access-Control-Allow-Origin", "*");
		res.set_content(string_To_UTF8("{\"code\":1,\"msg\":\"欢迎使用二代身份证阅读器服务，请访问<a href=\"help\">帮助文档</a>查看帮助文档\"}"), "text/html;charset=utf-8");
	});

	svr.Get("/read", [](const Request& req, Response& res) {
		res.set_header("Access-Control-Allow-Origin", "*");
		res.set_content(string_To_UTF8(ReadIDCard()), "application/json;charset=utf-8");
	});

	svr.Get(R"(/read/(\d+))", [&](const Request& req, Response& res) {
		auto iPort = req.matches[1];
		res.set_header("Access-Control-Allow-Origin", "*");
		res.set_content(string_To_UTF8(ReadIDCard(std::stoi(iPort))), "application/json;charset=utf-8");
	});
	svr.Get("/help", [](const Request& req, Response& res) {
		res.set_header("Access-Control-Allow-Origin", "*");
		res.set_content(string_To_UTF8("<b>帮助文档</b>：<br/><br/><b>默认读卡：</b><br/>请求地址：/read；<br/>请求方式： GET；<br/>响应类型： application/json;charset=utf-8；<br/><br/><b>指定端口读卡：</b><br/>请求地址：/read/1001；<br/>请求方式： GET；<br/>响应类型： application/json;charset=utf-8；<br/><br/><b>状态码说明：</b><br/>1，成功；<br/>0：错误；<br/>-1：无效端口；<br/>-2：程序不完整；<br/>-3：寻卡失败；<br/>-4：选卡失败；<br/>-6：读取身份信息失败；<br/>-7：照片读取失败；<br/><br/><b>错误响应示例：</b><br/>{\"code\":-3,\"msg\":\"寻卡失败.ErrCode = 128\"}；<br/><br/><b>成功响应示例：</b><br/>{\"code\":1,\"msg\":\"读卡成功\",\"data\":{\"name\":\"XXX\",\"sex\":\"XXX\",\"nation\":\"XXX\",\"birth\":\"XXX\",\"addr\":\"XXX\",\"id\":\"XXX\",\"dept\":\"XXX\",\"start\":\"XXX\",\"end\":\"XXX\",\"image\":\"data:image/bmp;base64,XXX\"}}<br/><br/><b>更新日志：</b><br/>2018-04-26 二代身份证阅读器在线读卡服务Python版 V1.0，二代身份证阅读器在线读卡服务注册机；<br/>2019-05-21 身份证阅读器web读卡服务Golang版 V2.0,集成opencv摄像头web调用；<br/>2019-11-14 身份证阅读器web读卡服务C#版 V3.0；<br/>2020-04-12 身份证阅读器web读卡服务C++版 V4.0；<br/><br/><b><div style=\"text-align:right;\">高本教育二代身份证阅读器web服务C++版 V4.1<br/>2020-04-21 21:54:38</div></b>"), "text/html;charset=utf-8");
	});

	//svr.Get("/stop", [&](const Request& req, Response& res) {
	//	svr.stop();
	//});

	svr.listen("127.0.0.1", 14725);
}

std::string ReadIDCard(int iPort)
{
	std::stringstream ss;
	if ((iPort < 1 || iPort >16) && (iPort < 1001 || iPort > 1004))
		return "{\"code\":-1,\"msg\":\"无效的端口\"}";


	typedef int(__stdcall *PF_SDT_StartFindIDCard)(int pi_iPort, unsigned char* po_pbyManaInfo, int pi_iIfOpen);
	typedef int(__stdcall *PF_SDT_SelectIDCard)(int pi_iPort, unsigned char* po_pbyManaInfo, int pi_iIfOpen);
	typedef int(__stdcall *PF_SDT_ReadBaseMsg)(int pi_iPort, unsigned char* po_pbyCHMsg, unsigned int*  po_puiCHMsgLen, unsigned char* po_pbyPHMsg, unsigned int*  po_puiPHMsgLen, int pi_iIfOpen);
	typedef int(*PF_unpack)(char* pi_szSrcWltData, char* pi_szDstPicData, int pi_iIsSaveToBmp);

	PF_SDT_StartFindIDCard      m_pfn_SDT_StartFindIDCard;
	PF_SDT_SelectIDCard         m_pfn_SDT_SelectIDCard;
	PF_SDT_ReadBaseMsg          m_pfn_SDT_ReadBaseMsg;
	PF_unpack m_pfn_unpack;


	HINSTANCE m_hSubModuleA = LoadLibrary(_T("sdtapi.dll"));
	HINSTANCE m_hSubModuleB = LoadLibrary(_T("DLL_File.dll"));

	m_pfn_SDT_StartFindIDCard = (PF_SDT_StartFindIDCard)GetProcAddress(m_hSubModuleA, "SDT_StartFindIDCard");
	m_pfn_SDT_SelectIDCard = (PF_SDT_SelectIDCard)GetProcAddress(m_hSubModuleA, "SDT_SelectIDCard");
	m_pfn_SDT_ReadBaseMsg = (PF_SDT_ReadBaseMsg)GetProcAddress(m_hSubModuleA, "SDT_ReadBaseMsg");
	m_pfn_unpack = (PF_unpack)GetProcAddress(m_hSubModuleB, "unpack");

	if (NULL == m_hSubModuleA) {
		return "{\"code\":-2,\"msg\":\"程序不完整\"}";
	}
	else if (NULL == m_hSubModuleB) {
		return "{\"code\":-2,\"msg\":\"程序不完整\"}";
	}
	else if (NULL == m_pfn_SDT_StartFindIDCard) {
		return "{\"code\":-2,\"msg\":\"程序不完整\"}";
	}
	else if (NULL == m_pfn_SDT_SelectIDCard) {
		return "{\"code\":-2,\"msg\":\"程序不完整\"}";
	}
	else if (NULL == m_pfn_SDT_ReadBaseMsg) {
		return "{\"code\":-2,\"msg\":\"程序不完整\"}";
	}
	else if (NULL == m_pfn_unpack) {
		return "{\"code\":-2,\"msg\":\"程序不完整\"}";
	}

	//开始读卡
	int iResult = 0;
	int iIfOpen = 1;//自动打开设备标志。如果设置为1，则在接口内部自动实现打开设备和关闭设备，无需调用者再添加。

	char szSAMID[64 + 1] = { 0 };

	unsigned char byCHMsg[256 + 1] = { 0 };      //个人基本信息
	unsigned int uiCHMsgSize = 0;               //个人基本信息字节数
	unsigned char byPHMsg[1024 + 1] = { 0 };     //照片信息
	unsigned int uiPHMsgSize = 0;	            //照片信息字节数
	unsigned char byFPMsg[1024 + 1] = { 0 };     //指纹信息
	unsigned int uiFPMsgSize = 0;               //指纹信息字节数

	wchar_t wszName[15 + 1] = { 0 };          //姓名
	wchar_t wszSex[1 + 1] = { 0 };            //性别
	wchar_t wszNation[2 + 1] = { 0 };         //民族
	wchar_t wszBirth[8 + 1] = { 0 };         //出生日期
	wchar_t wszAddr[35 + 1] = { 0 };          //地址
	wchar_t wszID[18 + 1] = { 0 };            //身份证号
	wchar_t wszDept[15 + 1] = { 0 };          //签发机关
	wchar_t wszStart[8 + 1] = { 0 };         //有效期起始
	wchar_t wszEnd[8 + 1] = { 0 };           //有效期截至
	wchar_t BitmapFilePath[256 + 1] = { 0 };
	unsigned char byManaID[8] = { 0 };

	int iIsSaveToBmp = 1;
	unsigned char byBgrBuffer[38556] = { 0 };    //解码后图片BGR编码值
	unsigned char byBmpBuffer[38862] = { 0 };    //解码后图片RGB编码值

	//寻卡
	iResult = m_pfn_SDT_StartFindIDCard(iPort, byManaID, iIfOpen);
	if (0x9F != iResult)
	{
		ss << "{\"code\":-3,\"msg\":\"寻卡失败.ErrCode = " << iResult << "\"}";//128
		return ss.str();
	}
	//选卡
	memset(byManaID, 0, sizeof(byManaID));
	iResult = m_pfn_SDT_SelectIDCard(iPort, byManaID, iIfOpen);
	if (0x90 != iResult)
	{
		ss << "{\"code\":-4,\"msg\":\"选卡失败.ErrCode = " << iResult << "\"}";//144
		return ss.str();
	}
	iResult = m_pfn_SDT_ReadBaseMsg(iPort, byCHMsg, &uiCHMsgSize, byPHMsg, &uiPHMsgSize, iIfOpen);//采用只读卡信息和照片，不读指纹信息的接口
	if (0x90 != iResult)
	{
		ss << "{\"code\":-5,\"msg\":\"读取身份信息失败.ErrCode = " << iResult << "\"}";//144
		return ss.str();
	}


	//解码照片数据
	iResult = m_pfn_unpack((char*)byPHMsg, (char*)byBgrBuffer, iIsSaveToBmp);
	if (1 != iResult)
	{
		ss << "{\"code\":-6,\"msg\":\"照片解码失败.ErrCode = " << iResult << "\"}";//144
		return ss.str();
	}

	//截取个人信息数据。信息采用UNICODE存储，具体格式参可见《二代证机读信息说明.doc》
	memcpy_s(wszName, sizeof(wszName), &byCHMsg[0], 30);
	memcpy_s(wszSex, sizeof(wszSex), &byCHMsg[30], 2);
	memcpy_s(wszNation, sizeof(wszNation), &byCHMsg[32], 4);
	memcpy_s(wszBirth, sizeof(wszBirth), &byCHMsg[36], 16);
	memcpy_s(wszAddr, sizeof(wszAddr), &byCHMsg[52], 70);
	memcpy_s(wszID, sizeof(wszID), &byCHMsg[122], 36);
	memcpy_s(wszDept, sizeof(wszDept), &byCHMsg[158], 30);
	memcpy_s(wszStart, sizeof(wszStart), &byCHMsg[188], 16);
	memcpy_s(wszEnd, sizeof(wszEnd), &byCHMsg[204], 16);

	//只读方式打开图片
	std::ifstream f;
	f.open("zp.bmp", std::ios::in|std::ios::binary);
	if (!f) {
		ss << "{\"code\":-7,\"msg\":\"照片读取失败\"}";
		return ss.str();
}

	f.seekg(0, std::ios_base::end);     //设置偏移量至文件结尾
	std::streampos sp = f.tellg();      //获取文件大小
	int size = sp;

	char* buffer = (char*)malloc(sizeof(char)*size);
	f.seekg(0, std::ios_base::beg);     //设置偏移量至文件开头
	f.read(buffer, size);                //将文件内容读入buffer
	f.close();

	std::string imgBase64 = base64_encode(buffer, size);         //编码
	delete[] buffer;
	buffer = NULL;

	ss << "{\"code\":1,\"msg\":\"读卡成功\",\"data\":{\"name\":\"" << trim(WcharToChar(wszName));
	ss << "\",\"sex\":\"" << GetSexByCodeA(WcharToChar(wszSex).data());
	ss << "\",\"nation\":\"" << GetNationByCodeA(WcharToChar(wszNation).data());
	ss << "\",\"birth\":\"" << trim(WcharToChar(wszBirth));
	ss << "\",\"addr\":\"" << trim(WcharToChar(wszAddr));
	ss << "\",\"id\":\"" << trim(WcharToChar(wszID));
	ss << "\",\"dept\":\"" << trim(WcharToChar(wszDept));
	ss << "\",\"start\":\"" << trim(WcharToChar(wszStart));
	ss << "\",\"end\":\"" << trim(WcharToChar(wszEnd));
	ss << "\",\"image\":\"data:image/bmp;base64," << imgBase64 << "\"}}";
	return ss.str();
}

std::string WcharToChar(const wchar_t* wp)
{
	std::string ret;
	char *m_char;
	int len = WideCharToMultiByte(CP_ACP, 0, wp, wcslen(wp), NULL, 0, NULL, NULL);
	m_char = new char[len + 1];
	WideCharToMultiByte(CP_ACP, 0, wp, wcslen(wp), m_char, len, NULL, NULL);
	m_char[len] = '\0';
	ret = m_char;
	delete[] m_char;
	m_char = NULL;
	return ret;
}

std::string  GetSexByCodeA(const char* pi_pszSexCode)
{
	if (NULL == pi_pszSexCode)
		return "错误";
	else if (0 == strcmp("1", pi_pszSexCode))
		return "男";
	else if (0 == strcmp("2", pi_pszSexCode))
		return "女";
	else if (0 == strcmp("9", pi_pszSexCode))
		return "其他";
	else
		return "未知";
}

std::string GetNationByCodeA(const char* pi_pszNationCode)
{
	if (NULL == pi_pszNationCode)
		return "未知";
	else if (0 == strcmp("01", pi_pszNationCode))
		return "汉";
	else if (0 == strcmp("02", pi_pszNationCode))
		return "蒙古";
	else if (0 == strcmp("03", pi_pszNationCode))
		return "回";
	else if (0 == strcmp("04", pi_pszNationCode))
		return "藏";
	else if (0 == strcmp("05", pi_pszNationCode))
		return "维吾尔";
	else if (0 == strcmp("06", pi_pszNationCode))
		return "苗";
	else if (0 == strcmp("07", pi_pszNationCode))
		return "彝";
	else if (0 == strcmp("08", pi_pszNationCode))
		return "壮";
	else if (0 == strcmp("09", pi_pszNationCode))
		return "布依";
	else if (0 == strcmp("10", pi_pszNationCode))
		return "朝鲜";
	else if (0 == strcmp("11", pi_pszNationCode))
		return "满";
	else if (0 == strcmp("12", pi_pszNationCode))
		return "侗";
	else if (0 == strcmp("13", pi_pszNationCode))
		return "瑶";
	else if (0 == strcmp("14", pi_pszNationCode))
		return "白";
	else if (0 == strcmp("15", pi_pszNationCode))
		return "土家";
	else if (0 == strcmp("16", pi_pszNationCode))
		return "哈尼";
	else if (0 == strcmp("17", pi_pszNationCode))
		return "哈萨克";
	else if (0 == strcmp("18", pi_pszNationCode))
		return "傣";
	else if (0 == strcmp("19", pi_pszNationCode))
		return "黎";
	else if (0 == strcmp("20", pi_pszNationCode))
		return "傈僳";
	else if (0 == strcmp("21", pi_pszNationCode))
		return "佤";
	else if (0 == strcmp("22", pi_pszNationCode))
		return "畲";
	else if (0 == strcmp("23", pi_pszNationCode))
		return "高山";
	else if (0 == strcmp("24", pi_pszNationCode))
		return "拉祜";
	else if (0 == strcmp("25", pi_pszNationCode))
		return "水";
	else if (0 == strcmp("26", pi_pszNationCode))
		return "东乡";
	else if (0 == strcmp("27", pi_pszNationCode))
		return "纳西";
	else if (0 == strcmp("28", pi_pszNationCode))
		return "景颇";
	else if (0 == strcmp("29", pi_pszNationCode))
		return "柯尔克孜";
	else if (0 == strcmp("30", pi_pszNationCode))
		return "土";
	else if (0 == strcmp("31", pi_pszNationCode))
		return "达斡尔";
	else if (0 == strcmp("32", pi_pszNationCode))
		return "仫佬";
	else if (0 == strcmp("33", pi_pszNationCode))
		return "羌";
	else if (0 == strcmp("34", pi_pszNationCode))
		return "布朗";
	else if (0 == strcmp("35", pi_pszNationCode))
		return "撒拉";
	else if (0 == strcmp("36", pi_pszNationCode))
		return "毛南";
	else if (0 == strcmp("37", pi_pszNationCode))
		return "仡佬";
	else if (0 == strcmp("38", pi_pszNationCode))
		return "锡伯";
	else if (0 == strcmp("39", pi_pszNationCode))
		return "阿昌";
	else if (0 == strcmp("40", pi_pszNationCode))
		return "普米";
	else if (0 == strcmp("41", pi_pszNationCode))
		return "塔吉克";
	else if (0 == strcmp("42", pi_pszNationCode))
		return "怒";
	else if (0 == strcmp("43", pi_pszNationCode))
		return "乌孜别克";
	else if (0 == strcmp("44", pi_pszNationCode))
		return "俄罗斯";
	else if (0 == strcmp("45", pi_pszNationCode))
		return "鄂温克";
	else if (0 == strcmp("46", pi_pszNationCode))
		return "德昂";
	else if (0 == strcmp("47", pi_pszNationCode))
		return "保安";
	else if (0 == strcmp("48", pi_pszNationCode))
		return "裕固";
	else if (0 == strcmp("49", pi_pszNationCode))
		return "京";
	else if (0 == strcmp("50", pi_pszNationCode))
		return "塔塔尔";
	else if (0 == strcmp("51", pi_pszNationCode))
		return "独龙";
	else if (0 == strcmp("52", pi_pszNationCode))
		return "鄂伦春";
	else if (0 == strcmp("53", pi_pszNationCode))
		return "赫哲";
	else if (0 == strcmp("54", pi_pszNationCode))
		return "门巴";
	else if (0 == strcmp("55", pi_pszNationCode))
		return "珞巴";
	else if (0 == strcmp("56", pi_pszNationCode))
		return "基诺";
	else
		return "未知";
}

char * trim(char * ptr)
{
	int start, end, i;
	if (ptr)
	{
		for (start = 0; isspace(ptr[start]); start++)
			;
		for (end = strlen(ptr) - 1; isspace(ptr[end]); end--)
			;
		for (i = start; i <= end; i++)
			ptr[i - start] = ptr[i];
		ptr[end - start + 1] = '\0';
		return (ptr);
	}
	else
		return NULL;
}

std::string trim(std::string &s)
{
	std::string ret;
	if (!s.empty())

	{
		s.erase(0, s.find_first_not_of(" "));
		s.erase(s.find_last_not_of(" ") + 1);
		ret = s;
	}
	return ret;
}

bool check_port(const char * host, int port)
{
	httplib::Client cli(host, port);

	auto res = cli.Get("/");
	if (res && res->status == 200)
		return true;
	else
		return false;
}


std::string base64_encode(const char * bytes_to_encode, unsigned int in_len)
{
	std::string ret;
	int i = 0;
	int j = 0;
	unsigned char char_array_3[3];
	unsigned char char_array_4[4];

	while (in_len--)
	{
		char_array_3[i++] = *(bytes_to_encode++);
		if (i == 3)
		{
			char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
			char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
			char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
			char_array_4[3] = char_array_3[2] & 0x3f;
			for (i = 0; (i < 4); i++)
			{
				ret += base64_chars[char_array_4[i]];
			}
			i = 0;
		}
	}
	if (i)
	{
		for (j = i; j < 3; j++)
		{
			char_array_3[j] = '\0';
		}

		char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
		char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
		char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
		char_array_4[3] = char_array_3[2] & 0x3f;

		for (j = 0; (j < i + 1); j++)
		{
			ret += base64_chars[char_array_4[j]];
		}

		while ((i++ < 3))
		{
			ret += '=';
		}

	}
	return ret;
}

std::string string_To_UTF8(const std::string & str)
{
	int nwLen = ::MultiByteToWideChar(CP_ACP, 0, str.c_str(), -1, NULL, 0);

	wchar_t * pwBuf = new wchar_t[nwLen + 1];//一定要加1，不然会出现尾巴
	ZeroMemory(pwBuf, nwLen * 2 + 2);

	::MultiByteToWideChar(CP_ACP, 0, str.c_str(), str.length(), pwBuf, nwLen);

	int nLen = ::WideCharToMultiByte(CP_UTF8, 0, pwBuf, -1, NULL, NULL, NULL, NULL);

	char * pBuf = new char[nLen + 1];
	ZeroMemory(pBuf, nLen + 1);

	::WideCharToMultiByte(CP_UTF8, 0, pwBuf, nwLen, pBuf, nLen, NULL, NULL);

	std::string retStr(pBuf);

	delete[]pwBuf;
	delete[]pBuf;

	pwBuf = NULL;
	pBuf = NULL;

	return retStr;
}

std::string UTF8_To_string(const std::string & str)
{
	int nwLen = MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, NULL, 0);

	wchar_t * pwBuf = new wchar_t[nwLen + 1];//一定要加1，不然会出现尾巴
	memset(pwBuf, 0, nwLen * 2 + 2);

	MultiByteToWideChar(CP_UTF8, 0, str.c_str(), str.length(), pwBuf, nwLen);

	int nLen = WideCharToMultiByte(CP_ACP, 0, pwBuf, -1, NULL, NULL, NULL, NULL);

	char * pBuf = new char[nLen + 1];
	memset(pBuf, 0, nLen + 1);

	WideCharToMultiByte(CP_ACP, 0, pwBuf, nwLen, pBuf, nLen, NULL, NULL);

	std::string retStr = pBuf;

	delete[]pBuf;
	delete[]pwBuf;

	pBuf = NULL;
	pwBuf = NULL;

	return retStr;
}
