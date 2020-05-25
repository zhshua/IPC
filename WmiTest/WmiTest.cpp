﻿#include <iostream>
#include "WMIInfo.h"
#include <fstream>
#include "time.h"
#pragma comment(lib, "version.lib")
#include <sstream>
#include <WinSock2.h>
#include <Windows.h>
#include <cstring>
#pragma comment(lib,"WS2_32.lib")
#define _WINSOCK_DEPRECATED_NO_WARNINGS 1

using namespace std;

void writeTime(const char fileName[]);
int Split(char array[][500], const CString  source, const CString flag);
float getSize(char temp[]);
void getSha1(const char fileName[], char sha1[]);
int getPath(char path[], char servicePath[]);
void getSysInfo(CWmiInfo wmi);
void getSysStartTime(CWmiInfo wmi, char StartTime[20]);
void getErrorTime(char time[20]);
void getIP(char ip[20]);
void getDisk(CWmiInfo wmi);
void getMemory(CWmiInfo wmi);
void getTwinCatInfo(const char* dir);
void getPatchInfo(CWmiInfo wmi);
void getService(CWmiInfo wmi);
void getUserAccount(CWmiInfo wmi);
void lastTime(char name[], char time[]);
void getExeInfo(CWmiInfo wmi);
void getProcess(CWmiInfo wmi);
void getPort();
void getRegitHash();
void udpSend(char path[]);
void findFile(char path[]);



int main()
{
	while (true) {
		CWmiInfo wmi;
		wmi.InitWmi();
		getSysInfo(wmi);//获取ip地址，操作系统版本，CPU利用率，本次开机时间，最近一次异常断电时间
		cout << "系统信息采集完毕" << endl;
		getDisk(wmi);
		cout << "磁盘信息采集完毕" << endl;
		getMemory(wmi);
		cout << "内存信息采集完毕" << endl;
		getPatchInfo(wmi);
		cout << "补丁信息采集完毕" << endl;
		getService(wmi);
		cout << "服务信息采集完毕" << endl;
		getUserAccount(wmi);
		cout << "账户信息采集完毕" << endl;
		getExeInfo(wmi);
		cout << "应用程序信息采集完毕" << endl;
		getProcess(wmi);
		cout << "进程信息采集完毕" << endl;
		wmi.ReleaseWmi();
		char TwinCatPath[] = "C:\\TwinCAT";
		char fileName1[] = "C:\\数据\\工控系统文件.txt";
		writeTime(fileName1);
		char fileName2[] = "C:\\数据\\工控编程文件.txt";
		writeTime(fileName2);
		getTwinCatInfo(TwinCatPath);
		cout << "工控信息采集完毕" << endl;
		getPort();
		cout << "端口信息采集完毕" << endl;
		getRegitHash();
		cout << "注册表信息采集完毕" << endl;
		char path[] = "C:\\数据\\*.txt";
		findFile(path);
		char str[] = "overover";
		//udpSend(str);
		cout << "发送完成" << endl;
		Sleep(1800000);
	}
}

void writeTime(const char fileName[]) {
	time_t timep;
	time(&timep);
	char tmp[64];
	struct tm nowTime;
	localtime_s(&nowTime, &timep);
	strftime(tmp, sizeof(tmp), "%Y-%m-%d %H:%M:%S", &nowTime);
	ofstream file(fileName);
	if (file.is_open())
	{
		file << "获取时间=";
		file << tmp;
		file.close();
	}
}

int Split(char array[][500], const CString  source, const CString flag) {
	CString temp;
	int nowLen = 0;
	int preLen;
	int i = 0;
	while (true) {
		temp = "";
		preLen = nowLen;
		nowLen = source.Find(flag, preLen);
		if (nowLen == -1) { break; }
		temp = source.Mid(preLen, nowLen - preLen);
		nowLen += 1;
		strcpy_s(array[i], temp);
		i += 1;
	}
	return i;
}

float getSize(char temp[]) {
	int i = 1;
	INT64 size_b = temp[0] - '0';
	while (temp[i] != '\0') {
		size_b = size_b * 10;
		size_b = size_b + (temp[i] - '0');
		i++;
	}
	float size = size_b / 1024 / 1024;
	return size / 1024;
}

void getSha1(const char fileName[], char sha1[]) {
	char cmd[1024];
	strcpy_s(cmd, "certutil -hashfile \"");
	strcat_s(cmd, fileName);
	strcat_s(cmd, "\" SHA1");
	FILE* pp = _popen(cmd, "r");
	if (!pp) {
		return;
	}
	char temp[1024];
	fgets(temp, sizeof(temp), pp);
	fgets(temp, sizeof(temp), pp);
	int i = 0;
	int j = 0;
	while (temp[i] != '\n') {
		if (temp[i] != ' ') {
			sha1[j] = temp[i];
			j++;
		}
		i++;
	}
	sha1[j] = '\0';
	_pclose(pp);
}

int getPath(char path[], char servicePath[]) {
	int i = 0;
	int j = 0;
	while (true) {
		if (servicePath[i] == '\0') { break; }
		if (servicePath[i] == ' ' && servicePath[i - 1] == 'e') { break; }
		if (servicePath[i] != '\''&&servicePath[i]!='\"') {
			path[j] = servicePath[i];
			j++;
		}
		i++;
	}
	return j;
}

void getSysInfo(CWmiInfo wmi) {
    //获取ip地址，操作系统版本，CPU利用率，本次开机时间，最近一次异常断电时间
    CString strRetValue;
    wmi.GetSingleItemInfo(_T("Win32_Processor"), _T("LoadPercentage"), strRetValue);
    char CpuPre[1024];
    strncpy_s(CpuPre, 1024, (LPCTSTR)strRetValue, strlen(strRetValue) - 1);

    strRetValue = "";
    wmi.GetSingleItemInfo(_T("Win32_OperatingSystem"), _T("Caption"), strRetValue);
    char SysType[1024];
    strncpy_s(SysType, 1024, (LPCTSTR)strRetValue, strlen(strRetValue) - 1);

	char SysStartTime[20];
	getSysStartTime(wmi, SysStartTime);

	char ErrorEndTime[20];
	getErrorTime(ErrorEndTime);

	char ip[20];
	getIP(ip);

	char fileName[] = "C:\\数据\\系统信息.txt";
	writeTime(fileName);
	ofstream file(fileName, ios::out | ios::app);
	if (file.is_open())
	{
		file << "\nip地址=";
		file << ip;
		file << "\n操作系统=";
		file << SysType;
		file << "\nCPU利用率=";
		file << CpuPre;
		file << "%";
		file << "\n本次开机时间=";
		file << SysStartTime;
		file << "\n最近一次异常断点时间=";
		file << ErrorEndTime;
		file.close();
	}
}

void getSysStartTime(CWmiInfo wmi, char StartTime[20]) {
	CString strRetValue;
	wmi.GetSingleItemInfo(_T("Win32_OperatingSystem"), _T("LastBootUpTime"), strRetValue);
	char temp[1024];
	strcpy_s(temp, strRetValue);

	char time[20];
	strncpy_s(time, 20, temp, 4);//year
	strcat_s(time, "-");
	strncat_s(time, 20, temp + 4, 2);//month
	strcat_s(time, "-");
	strncat_s(time, 20, temp + 6, 2);//day
	strcat_s(time, " ");
	strncat_s(time, 20, temp + 8, 2);//hour
	strcat_s(time, ":");
	strncat_s(time, 20, temp + 10, 2);//min
	strcat_s(time, ":");
	strncat_s(time, 20, temp + 12, 2);//second

	strncpy_s(StartTime,20, time,19);
}

void getErrorTime(char time[20]) {
	char cmd[1024] = "wevtutil qe System /f:text /rd:true /c:1 /q:\"Event[System[(EventID=41)]]\"";
	FILE* pp = _popen(cmd, "r");
	if (!pp) {
		return;
	}
	char temp[1024] = "\0";
	fgets(temp, sizeof(temp), pp);
	if (temp[0] == '\0') {
		strncpy_s(time, 20, "无", 3);
	}
	else {
		for (int i = 0; i < 3; i++)
		{
			fgets(temp, sizeof(temp), pp);

		}
		int len = strlen(temp);
		char errorTime[50] = "\0";
		strncpy_s(errorTime, len, temp + 8, len - 13);
		for (int i = 0; i < 50; i++) {
			if (errorTime[i] == 'T') {
				errorTime[i] = ' ';
				break;
			}
		}
		strncpy_s(time, 20, errorTime, 19);
	}
}

void getIP(char ip[20]) {
	char cmd[1024] = "ipconfig";
	FILE* pp = _popen(cmd, "r");
	if (!pp) {
		return;
	}
	char temp[1024];
	for (int i = 0; i < 14; i++) {
		fgets(temp, sizeof(temp), pp);
	}
	strncpy_s(ip, 20, temp + 39, 15);
}

void getDisk(CWmiInfo wmi) {
	CString strRetValue;
	wmi.GetSingleItemInfo(_T("Win32_LogicalDisk"), _T("Name"), strRetValue);
	char diskName[10][500];
	int num = Split(diskName, strRetValue, "\n");
	int flag = 0;
	for (int i = 0; i < num; i++) {
		if (strcmp(diskName[i],"C:")!=0){
			if (strcmp(diskName[i], "D:") != 0) {
				flag = i;
				break;
			}
		}
	}
	int allNum=num;
	if (flag != 0) {
		num = flag;
	}
	strRetValue = "";
	wmi.GetSingleItemInfo(_T("Win32_LogicalDisk"), _T("Size"), strRetValue);
	char allSize[10][500];
	Split(allSize, strRetValue, "\n");

	strRetValue = "";
	wmi.GetSingleItemInfo(_T("Win32_LogicalDisk"), _T("FreeSpace"), strRetValue);
	char freeSize[10][500];
	Split(freeSize, strRetValue, "\n");

	char fileName[] = "C:\\数据\\磁盘信息.txt";
	ofstream file(fileName);
	if (file.is_open()) {
		for (int i = 0; i < num; i++) {
			float allSizeFloat = getSize(allSize[i]);
			float freeSizeFloat = getSize(freeSize[i]);
			float useSizeFloat = allSizeFloat - freeSizeFloat;
			float diskPre = useSizeFloat / allSizeFloat * 100;
			allSizeFloat = ((float)((int)((allSizeFloat + 0.005) * 100))) / 100;
			useSizeFloat = ((float)((int)((useSizeFloat + 0.005) * 100))) / 100;
			diskPre = ((float)((int)((diskPre + 0.005) * 100))) / 100;
			file << "磁盘名=";
			file << diskName[i];
			file << "\n总大小=";
			file << allSizeFloat;
			file << "G\n已用大小=";
			file << useSizeFloat;
			file << "G\n使用率=";
			file << diskPre;
			file << "%\n";
		}
		file.close();
	}

	strRetValue = "";
	wmi.GetSingleItemInfo(_T("Win32_DiskDrive"), _T("SerialNumber"), strRetValue);
	char SerialNumber[10][500];
	num = Split(SerialNumber, strRetValue, "\n");
	char fileName1[] = "C:\\数据\\外设信息.txt";
	ofstream file1(fileName1);
	if (file1.is_open()) {
		int j = 1;
		if (flag == 0) {
			file1 << "无外设插入";
		}
		else {
			for (int i = flag; i < allNum; i++) {
				file1 << "外设名:";
				file1 << diskName[i];
				file1 << "\n标识码:";
				file1 << SerialNumber[j];
				file1 << "\n";
				j++;
			}
		}
		file1.close();
	}
}

void getMemory(CWmiInfo wmi) {
	CString strRetValue;
	wmi.GetSingleItemInfo(_T("Win32_PhysicalMemory"), _T("Capacity"), strRetValue);
	char temp[1024];
	strncpy_s(temp, sizeof(temp), (LPCTSTR)strRetValue, strlen(strRetValue) - 1);
	float allSize = getSize(temp);

	strRetValue = "";
	wmi.GetSingleItemInfo(_T("Win32_PerfFormattedData_PerfOS_Memory"), _T("AvailableBytes"), strRetValue);
	strncpy_s(temp, sizeof(temp), (LPCTSTR)strRetValue, strlen(strRetValue) - 1);
	float freeSize = getSize(temp);

	float useSize = allSize - freeSize;
	float memoryPre = useSize / allSize * 100;

	allSize = ((float)((int)((allSize + 0.005) * 100))) / 100;
	memoryPre = ((float)((int)((memoryPre + 0.005) * 100))) / 100;

	char fileName[] = "C:\\数据\\内存.txt";
	writeTime(fileName);
	ofstream file(fileName);
	if (file.is_open())
	{
		file << "内存大小=";
		file << allSize;
		file << "G\n内存利用率=";
		file << memoryPre;
		file << "%";
		file.close();
	}
}

void getTwinCatInfo(const char* dir) {
	HANDLE hFind;
	WIN32_FIND_DATA findData;
	LARGE_INTEGER size;
	char dirNew[999];

	// 向目录加通配符，用于搜索第一个文件 
	strcpy_s(dirNew, dir);
	strcat_s(dirNew, "\\*.*");

	hFind = FindFirstFile(dirNew, &findData);
	do
	{
		if (strcmp(findData.cFileName, ".") == 0 || strcmp(findData.cFileName, "..") == 0)
			continue;
		// 是否是文件夹
		if (findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
		{
			// 将dirNew设置为搜索到的目录，并进行下一轮搜索 
			strcpy_s(dirNew, dir);
			strcat_s(dirNew, "\\");
			strcat_s(dirNew, findData.cFileName);
			getTwinCatInfo(dirNew);
		}
		else {
			int len = strlen(findData.cFileName);
			char suffixName[5];
			strncpy_s(suffixName, 5, findData.cFileName + (len - 4), 4);
			if (strcmp(suffixName, ".dll") == 0 || strcmp(suffixName, ".sys") == 0 || strcmp(suffixName, ".tsm") == 0) {
				char fileName[999];
				strcpy_s(fileName, dir);
				strcat_s(fileName, "\\");
				strcat_s(fileName, findData.cFileName);
				char sha1[80];
				getSha1(fileName, sha1);

				char writeFileName[] = "C:\\数据\\工控系统文件.txt";
				ofstream writeFile(writeFileName, ios::out | ios::app);
				if (writeFile.is_open())
				{
					writeFile << "\n\n文件名=";
					writeFile << findData.cFileName;
					writeFile << "\n文件路径=";
					writeFile << fileName;
					writeFile << "\n哈希值=";
					writeFile << sha1;
					writeFile.close();
				}
			}
			if (strcmp(suffixName, ".BAK") == 0 || strcmp(suffixName, ".pro") == 0 || strcmp(suffixName, ".SYM") == 0)
			{
				char fileName[999];
				strcpy_s(fileName, dir);
				strcat_s(fileName, "\\");
				strcat_s(fileName, findData.cFileName);
				char sha1[80];
				getSha1(fileName, sha1);

				char writeFileName[] = "C:\\数据\\工控编程文件.txt";
				ofstream writeFile(writeFileName, ios::out | ios::app);
				if (writeFile.is_open())
				{
					writeFile << "\n\n文件名=";
					writeFile << findData.cFileName;
					writeFile << "\n文件路径=";
					writeFile << fileName;
					writeFile << "\n哈希值=";
					writeFile << sha1;
					writeFile.close();
				}
			}
		}
	} while (FindNextFile(hFind, &findData));

	FindClose(hFind);
}

void getPatchInfo(CWmiInfo wmi) {
	CString strRetValue;
	char patchID[200][500];
	char patchFrom[200][500];
	char installTime[200][500];
	char fileName[] = "C:\\数据\\补丁.txt";

	wmi.GetSingleItemInfo(_T("Win32_QuickFixEngineering"), _T("HotFixID"), strRetValue);
	int len = Split(patchID, strRetValue, "\n");

	strRetValue = "";
	wmi.GetSingleItemInfo(_T("Win32_QuickFixEngineering"), _T("Caption"), strRetValue);
	Split(patchFrom, strRetValue, "\n");

	strRetValue = "";
	wmi.GetSingleItemInfo(_T("Win32_QuickFixEngineering"), _T("InstalledOn"), strRetValue);
	Split(installTime, strRetValue, "\n");

	ofstream file(fileName);
	for (int i = 0; i < len; i++) {
		if (file.is_open())
		{
			file << "补丁名称=";
			file << patchID[i];
			file << "\n补丁来源=";
			file << patchFrom[i];
			file << "\n安装日期=";
			file << installTime[i];
			file << "\n";
		}
	}
	file.close();
}

void getService(CWmiInfo wmi) {
	CString strRetValue;
	char serviceName[300][500];
	char serviceAddress[300][500];
	char fileName1[] = "C:\\数据\\系统服务.txt";
	char fileName2[] = "C:\\数据\\非系统服务.txt";

	char sysName1[] = "C:\\Windows\\system32";
	char sysName2[] = "C:\\Windows\\System32";

	wmi.GetSingleItemInfo(_T("Win32_Service"), _T("DisplayName"), strRetValue);
	int len1 = Split(serviceName, strRetValue, "\n");

	strRetValue = "";
	wmi.GetSingleItemInfo(_T("Win32_Service"), _T("PathName"), strRetValue);
	int len = Split(serviceAddress, strRetValue, "\n");

	ofstream file1(fileName1);
	ofstream file2(fileName2);
	for (int i = 0; i < len; i++) {
		char path[200];
		int j = getPath(path, serviceAddress[i]);
		path[j] = '\0';
		if (memcmp(path, sysName1, strlen(sysName1)) == 0 || memcmp(path, sysName2, strlen(sysName2)) == 0) {
			//是系统服务
			if (file1.is_open())
			{
				file1 << "服务名称=";
				file1 << serviceName[i];
				file1 << "\n启动路径=";
				file1 << path;
				file1 << "\n";
			}
		}
		else {
			if (file2.is_open())
			{
				file2 << "服务名称=";
				file2 << serviceName[i];
				file2 << "\n启动路径=";
				file2 << path;
				file2 << "\n";
			}
		}
	}
	file1.close();
	file2.close();
}

void getUserAccount(CWmiInfo wmi) {
	CString strRetValue;
	char name[100][500];
	char fileName[] = "C:\\数据\\账户.txt";

	wmi.GetSingleItemInfo(_T("Win32_UserAccount WHERE Disabled='False'"), _T("Name"), strRetValue);
	int len = Split(name, strRetValue, "\n");

	ofstream file(fileName);
	for (int i = 0; i < len; i++) {
		char time[1024];
		lastTime(name[i], time);
		if (file.is_open())
		{
			file << "账户名=";
			file << name[i];
			file << "\n上次登录时间=";
			file << time;
			file << "\n";
		}
	}
	file.close();
}

void lastTime(char name[], char time[]) {
	char cmd[1024];
	strcpy_s(cmd, "net user ");
	strcat_s(cmd, name);
	FILE* pp = _popen(cmd, "r");
	if (!pp) {
		return;
	}
	char temp[1024];
	for (int i = 0; i < 19; i++) {
		fgets(temp, sizeof(temp), pp);
	}
	int start = 0;
	for (int i = 0; i < 1024; i++) {
		if (temp[i] > '0' && temp[i] < '9') {
			start = i;
			break;
		}
	}
	int i = 0;
	while (temp[start] != '\0' && temp[start] != '\n') {
		time[i] = temp[start];
		i++;
		start++;
	}
	time[i] = '\0';
}

void getExeInfo(CWmiInfo wmi) {
	CString strRetValue;
	wmi.GetSingleItemInfo(_T("Win32_Product"), _T("Name"), strRetValue);
	char name[500][500];
	int len1 = Split(name, strRetValue, "\n");

	strRetValue = "";
	wmi.GetSingleItemInfo(_T("Win32_Product"), _T("Version"), strRetValue);
	char version[500][500];
	int len2 = Split(version, strRetValue, "\n");

	strRetValue = "";
	wmi.GetSingleItemInfo(_T("Win32_Product"), _T("Vendor"), strRetValue);
	char publish[500][500];
	int len3 = Split(publish, strRetValue, "\n");

	char fileName[] = "C:\\数据\\exe.txt";
	ofstream file(fileName);
	for (int i = 0; i < len1; i++) {
		if (file.is_open())
		{
			file << "应用名=";
			file << name[i];
			file << "\n版本号=";
			file << version[i];
			file << "\n发行商=";
			file << publish[i];
			file << "\n";
		}
	}
	file.close();
}

void getProcess(CWmiInfo wmi) {
	CString strRetValue;
	wmi.GetSingleItemInfo(_T("Win32_Process"), _T("Name"), strRetValue);
	char name[500][500];
	int len1 = Split(name, strRetValue, "\n");

	strRetValue = "";
	wmi.GetSingleItemInfo(_T("Win32_Process"), _T("ExecutablePath"), strRetValue);
	char path[500][500];
	int len2 = Split(path, strRetValue, "\n");

	char fileName[] = "C:\\数据\\进程.txt";
	ofstream file(fileName);
	for (int i = 3; i < len1; i++) {
		if (file.is_open())
		{
			file << "进程名=";
			file << name[i];
			file << "\n路径=";
			file << path[i];
			file << "\n";
		}
	}
	file.close();
}

void getPort() {
	char cmd[1024];
	strcpy_s(cmd, "netstat -an");
	FILE* pp = _popen(cmd, "r");
	if (!pp) {
		return;
	}
	char temp[1024];
	char portList[200][10];
	fgets(temp, sizeof(temp), pp);
	fgets(temp, sizeof(temp), pp);
	fgets(temp, sizeof(temp), pp);
	fgets(temp, sizeof(temp), pp);
	int i = 0;
	int line = 0;
	while (true) {
		strcpy_s(temp, "\0");
		fgets(temp, sizeof(temp), pp);
		if (0 == strcmp(temp, "")) {
			break;
		}
		else {
			line++;
			if (temp[9] == '[') {
				int j = 9;
				while (temp[j] != ']')
				{
					j++;
				}
				j = j + 2;
				int k = 0;
				while (temp[j] != ' ')
				{
					portList[i][k] = temp[j];
					j++;
					k++;
				}
				portList[i][k] = '\0';
			}
			else {
				int j = 16;
				while (temp[j] != ':')
				{
					j++;
				}
				j++;
				int k = 0;
				while (temp[j] != ' ') {
					portList[i][k] = temp[j];
					k++;
					j++;
				}
				portList[i][k] = '\0';
			}
		}
		i++;
	}
	int portArray[200] = { 0 };
	int port;
	stringstream stream;
	stream << portList[0];
	stream >> portArray[0];
	for (int i = 1; i < line; i++) {
		stringstream stream;
		stream << portList[i];
		stream >> port;
		int existFlag = 0;
		int j = 0;
		while (portArray[j] != 0) {
			if (portArray[j] == port) {
				existFlag = 1;
			}
			j++;
		}
		if (existFlag == 0) {
			portArray[j] = port;
		}
	}
	char fileName[] = "C:\\数据\\端口.txt";
	ofstream file(fileName);
	i = 0;
	while (portArray[i] != 0) {
		if (file.is_open())
		{
			file << "端口号:";
			file << portArray[i];
			file << "\n";
		}
		i++;
	}
	file.close();
}

void getRegitHash() {
	char cmd[1024];
	strcpy_s(cmd, "reg export HKCR C:\\HKCR.reg -y");
	FILE* pp = _popen(cmd, "r");
	if (!pp) {
		return;
	}
	char temp[1024];
	fgets(temp, sizeof(temp), pp);

	strcpy_s(cmd, "reg export HKCU C:\\HKCU.reg -y");
	pp = _popen(cmd, "r");
	if (!pp) {
		return;
	}
	fgets(temp, sizeof(temp), pp);

	strcpy_s(cmd, "reg export HKCC C:\\HKCC.reg -y");
	pp = _popen(cmd, "r");
	if (!pp) {
		return;
	}
	fgets(temp, sizeof(temp), pp);

	strcpy_s(cmd, "reg export HKLM C:\\HKLM.reg -y");
	pp = _popen(cmd, "r");
	if (!pp) {
		return;
	}
	fgets(temp, sizeof(temp), pp);

	strcpy_s(cmd, "reg export HKU C:\\HKU.reg -y");
	pp = _popen(cmd, "r");
	if (!pp) {
		return;
	}
	fgets(temp, sizeof(temp), pp);

	char sha1[5][80];
	getSha1("C:\\HKCR.reg", sha1[0]);
	getSha1("C:\\HKCU.reg", sha1[1]);
	getSha1("C:\\HKCC.reg", sha1[2]);
	getSha1("C:\\HKLM.reg", sha1[3]);
	getSha1("C:\\HKU.reg", sha1[4]);
	char fileName[] = "C:\\数据\\注册表.txt";
	ofstream file(fileName);
	if (file.is_open()){
		file << "HKCR表的sha1值:";
		file << sha1[0];
		file << "\nHKCU表的sha1值:";
		file << sha1[1];
		file << "\nHKCC表的sha1值:";
		file << sha1[2];
		file << "\nHKLM表的sha1值:";
		file << sha1[3];
		file << "\nHKU表的sha1值:";
		file << sha1[4];
	}
	file.close();
}

void udpSend(char path[]) {
	WSADATA wsd;
	SOCKET s;

	if (WSAStartup(MAKEWORD(2, 2), &wsd))
	{
		printf("WSAStartup failed!\n");
		return;
	}

	s = socket(AF_INET, SOCK_DGRAM, 0);
	if (s == INVALID_SOCKET)
	{
		printf("socket failed,Error Code:%d\n", WSAGetLastError());
		WSACleanup();
		return;
	}
	SOCKADDR_IN addr;
	SOCKET sockClient = socket(AF_INET, SOCK_DGRAM, 0);

	addr.sin_family = AF_INET;
	addr.sin_addr.S_un.S_addr = inet_addr("192.168.0.109");
	addr.sin_port = htons(1401);

	sendto(sockClient, path, 1024, 0, (sockaddr*)&addr, 1024);

	char msg[1024];
	ifstream file(path);
	if (file.is_open()) {
		while (!file.eof()) {
			file.getline(msg, 500);
			sendto(sockClient, msg, 1024, 0, (sockaddr*)&addr, 1024);
			Sleep(3);
		}
		file.close();

		sendto(sockClient, "over", 1024, 0, (sockaddr*)&addr, 1024);
	}
	closesocket(s);
	WSACleanup();
}

void findFile(char path[]) {
	HANDLE hFind;
	WIN32_FIND_DATA findData;
	LARGE_INTEGER size;
	hFind = FindFirstFile(path, &findData);
	if (hFind == INVALID_HANDLE_VALUE)
	{
		cout << "Failed to find first file!\n";
		return;
	}
	do
	{
		// 忽略"."和".."两个结果 
		if (strcmp(findData.cFileName, ".") == 0 || strcmp(findData.cFileName, "..") == 0)
			continue;
		char filePath[100] = "C:\\数据\\";
		strcat_s(filePath, findData.cFileName);
		udpSend(filePath);
	} while (FindNextFile(hFind, &findData));
}