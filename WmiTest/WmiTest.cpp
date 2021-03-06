#define _WINSOCK_DEPRECATED_NO_WARNINGS 1
#include "../config.h"
#include "WMIInfo.h"
#include <iostream>
#include <fstream>
#include <time.h>
#pragma comment(lib, "version.lib")
#include <sstream>
#include <WinSock2.h>
#include <Windows.h>
#include <cstring>
#pragma comment(lib,"WS2_32.lib")

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



int main()
{
	CWmiInfo wmi;
	wmi.InitWmi();
	
	getSysInfo(wmi);//获取ip地址，操作系统版本，CPU利用率，本次开机时间，最近一次异常断电时间
	cout << "sysinfo over" << endl;
	Sleep(1000);

	getDisk(wmi);
	cout << "disk over" << endl;
	Sleep(1000);

	getMemory(wmi);
	cout << "memory over" << endl;
	Sleep(2000);
	
	
	getPatchInfo(wmi);
	cout << "patch over" << endl;
	Sleep(2000);
	
	
	getService(wmi);
	cout << "service over" << endl;
	Sleep(2000);
	
	
	
	getUserAccount(wmi);
	cout << "user over" << endl;
	Sleep(2000);

	getExeInfo(wmi);
	cout << "app over" << endl;
	Sleep(2000);

	getProcess(wmi);
	cout << "process over" << endl;
	Sleep(2000);
	
	
	wmi.ReleaseWmi();
	
	char TwinCatPath[] = "C:\\TwinCAT";
	char fileName1[] = "C:\\data\\txtdata\\industrial_system_files.txt";
	//writeTime(fileName1);
	char fileName2[] = "C:\\data\\txtdata\\industrial_programming_files.txt";
	//writeTime(fileName2);
	getTwinCatInfo(TwinCatPath);
	cout << "TwinCatInfo over" << endl;
	Sleep(2000);

	getPort();
	cout << "port over" << endl;
	Sleep(2000);

	getRegitHash();
	cout << "regedit over" << endl;
	Sleep(2000);
	
	//return 0;
}

void getPatchInfo(CWmiInfo wmi) {
	CString strRetValue;
	char patchID[500][500];
	char patchFrom[500][500];
	char installTime[500][500];
	
	char fileName[] = "C:\\data\\txtdata\\patch.txt";

	/*
	CString strClassMem[] = { _T("HotFixID"), _T("Caption"), _T("InstalledOn") };
	wmi.GetGroupItemInfo(_T("Win32_QuickFixEngineering"), strClassMem, 3, strRetValue);
	cout << strRetValue << endl;*/

	strRetValue = "";
	wmi.GetSingleItemInfo(_T("Win32_QuickFixEngineering"), _T("HotFixID"), strRetValue);
	int len = Split(patchID, strRetValue, "\n");
	//cout << len << endl;

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
			file << "name:";
			file << patchID[i];
			file << "\npath:";
			file << patchFrom[i];
			file << "\ninstall_time:";
			file << installTime[i];
			file << "\n";
			//cout << patchID[i] << endl;
		}
	}
	file.close();
}

void getService(CWmiInfo wmi) {
	CString strRetValue;
	char serviceName[500][500];
	char serviceAddress[500][500];
	char fileName1[] = "C:\\data\\txtdata\\system_services.txt";
	char fileName2[] = "C:\\data\\txtdata\\notsystem_services.txt";

	char sysName1[] = "C:\\Windows\\system32";
	char sysName2[] = "C:\\Windows\\System32";

	/*CString strClassMem[] = { _T("DisplayName"), _T("PathName") };
	wmi.GetGroupItemInfo(_T("Win32_Service"), strClassMem, 2, strRetValue);
	cout << strRetValue << endl;*/

	wmi.GetSingleItemInfo(_T("Win32_Service"), _T("DisplayName"), strRetValue);
	int len1 = Split(serviceName, strRetValue, "\n");

	strRetValue = "";
	wmi.GetSingleItemInfo(_T("Win32_Service"), _T("PathName"), strRetValue);
	int len = Split(serviceAddress, strRetValue, "\n");

	ofstream file1(fileName1);
	ofstream file2(fileName2);
	for (int i = 0; i < len; i++) {
		char path[500];
		int j = getPath(path, serviceAddress[i]);
		path[j] = '\0';
		if (memcmp(path, sysName1, strlen(sysName1)) == 0 || memcmp(path, sysName2, strlen(sysName2)) == 0) {
			//是系统服务
			if (file1.is_open())
			{
				file1 << "name:";
				file1 << serviceName[i];
				file1 << "\npath:";
				file1 << path;
				file1 << "\n";
			}
		}
		else {
			if (file2.is_open())
			{
				file2 << "name:";
				file2 << serviceName[i];
				file2 << "\npath:";
				file2 << path;
				file2 << "\n";
			}
		}
	}
	file1.close();
	file2.close();
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
		file << "get_time:";
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
	//Sleep(500);
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
	int ret = readini(CONF_PATH, "client", "src_ip", ip);
	//cout << "ip...." << endl;
	//getIP(ip);

	char fileName[] = "C:\\data\\txtdata\\sysinfo.txt";
	writeTime(fileName);
	ofstream file(fileName, ios::out | ios::app);
	if (file.is_open())
	{
		file << "\nip:";
		file << ip;
		file << "\nsystype:";
		file << SysType;
		file << "\ncpu_usage:";
		file << CpuPre;
		file << "%";
		file << "\nstartTime:";
		file << SysStartTime;
		file << "\nerrorTime:";
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
		strncpy_s(time, 20, "no", 3);
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
	char flag[8];
	while (true) {
		fgets(temp, sizeof(temp), pp);
		strncpy_s(flag, temp, 7);
		if (strcmp(flag, "   IPv4") == 0) {
			break;
		}
	}
	//cout << "222" << endl;
	int i = 0;
	while (temp[i + 39] != '\n') {
		ip[i] = temp[i + 39];
		i++;
	}
	ip[i] = '\0';
	//cout << ip << endl;
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

	char fileName[] = "C:\\data\\txtdata\\disk.txt";
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
			file << "name:";
			file << diskName[i];
			file << "\ntotal_size:";
			file << allSizeFloat;
			file << "G\nused_size:";
			file << useSizeFloat;
			file << "G\ndisk_usage:";
			file << diskPre;
			file << "%\n";
		}
		file.close();
	}

	strRetValue = "";
	wmi.GetSingleItemInfo(_T("Win32_DiskDrive"), _T("SerialNumber"), strRetValue);
	char SerialNumber[10][500];
	num = Split(SerialNumber, strRetValue, "\n");
	char fileName1[] = "C:\\data\\txtdata\\mobile_device.txt";
	ofstream file1(fileName1);
	if (file1.is_open()) {
		int j = 1;
		if (flag == 0) {
			file1 << "no mobile_device insert";
		}
		else {
			for (int i = flag; i < allNum; i++) {
				file1 << "name:";
				file1 << diskName[i];
				file1 << "\nserial_number:";
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
	char memory_size[10][500];
	double allSize = 0;
	wmi.GetSingleItemInfo(_T("Win32_PhysicalMemory"), _T("Capacity"), strRetValue);
	
	int len = Split(memory_size, strRetValue, "\n");
	for (int i = 0; i < len; i++)
	{
		allSize += getSize(memory_size[i]);
	}

	char temp[1024];
	strRetValue = "";
	wmi.GetSingleItemInfo(_T("Win32_PerfFormattedData_PerfOS_Memory"), _T("AvailableBytes"), strRetValue);
	//cout << strRetValue << endl;
	strncpy_s(temp, sizeof(temp), (LPCTSTR)strRetValue, strlen(strRetValue) - 1);
	double freeSize = getSize(temp);

	double useSize = allSize - freeSize;
	double memoryPre = useSize / allSize * 100;


	allSize = ((double)((int)((allSize + 0.005) * 100))) / 100;
	memoryPre = ((double)((int)((memoryPre + 0.005) * 100))) / 100;

	char fileName[] = "C:\\data\\txtdata\\memory.txt";
	writeTime(fileName);
	ofstream file(fileName);
	if (file.is_open())
	{
		file << "memory_size:";
		file << allSize;
		file << "G\nmemory_usage:";
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
		//Sleep(100);
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
				char sha1[100];
				getSha1(fileName, sha1);

				char writeFileName[] = "C:\\data\\txtdata\\industrial_system_files.txt";
				ofstream writeFile(writeFileName, ios::out | ios::app);
				if (writeFile.is_open())
				{
					writeFile << "name:";
					writeFile << findData.cFileName;
					writeFile << "\npath:";
					writeFile << fileName;
					writeFile << "\nsha1:";
					writeFile << sha1;
					writeFile << "\n";
					//cout << findData.cFileName << endl;
					writeFile.close();
				}
			}
			if (strcmp(suffixName, ".BAK") == 0 || strcmp(suffixName, ".pro") == 0 || strcmp(suffixName, ".SYM") == 0)
			{
				char fileName[999];
				strcpy_s(fileName, dir);
				strcat_s(fileName, "\\");
				strcat_s(fileName, findData.cFileName);
				char sha1[100];
				getSha1(fileName, sha1);

				char writeFileName[] = "C:\\data\\txtdata\\industrial_programming_files.txt";
				ofstream writeFile(writeFileName, ios::out | ios::app);
				if (writeFile.is_open())
				{
					writeFile << "name:";
					writeFile << findData.cFileName;
					writeFile << "\npath:";
					writeFile << fileName;
					writeFile << "\nsha1:";
					writeFile << sha1;
					writeFile << "\n";
					writeFile.close();
				}
			}
		}
	} while (FindNextFile(hFind, &findData));

	FindClose(hFind);
}



void getUserAccount(CWmiInfo wmi) {
	CString strRetValue;
	char name[100][500];
	char fileName[] = "C:\\data\\txtdata\\login_user.txt";

	wmi.GetSingleItemInfo(_T("Win32_UserAccount WHERE Disabled='False'"), _T("Name"), strRetValue);
	int len = Split(name, strRetValue, "\n");

	ofstream file(fileName);
	for (int i = 0; i < len; i++) {
		char time[1024];
		lastTime(name[i], time);
		if (file.is_open())
		{
			file << "user:";
			file << name[i];
			file << "\nlast_login_time:";
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
	/*CString strRetValue;
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
	int len3 = Split(publish, strRetValue, "\n");*/
	char name[5][200], version[5][200], publish[5][200];
	strcpy(name[0], "honst.exe");
	strcpy(version[0], "14.0.25420");
	strcpy(publish[0], "Microsoft Corporation");

	char fileName[] = "C:\\data\\txtdata\\application.txt";
	ofstream file(fileName);
	for (int i = 0; i < 1; i++) {
		if (file.is_open())
		{
			file << "name:";
			file << name[i];
			file << "\nversion:";
			file << version[i];
			file << "\npublish:";
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

	char fileName[] = "C:\\data\\txtdata\\process.txt";
	ofstream file(fileName);
	for (int i = 3; i < len1; i++) {
		if (file.is_open())
		{
			file << "name:";
			file << name[i];
			file << "\npath:";
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
	char fileName[] = "C:\\data\\txtdata\\network_port.txt";
	ofstream file(fileName);
	i = 0;
	while (portArray[i] != 0) {
		if (file.is_open())
		{
			file << "port:";
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
	Sleep(3000);

	strcpy_s(cmd, "reg export HKCU C:\\HKCU.reg -y");
	pp = _popen(cmd, "r");
	if (!pp) {
		return;
	}
	fgets(temp, sizeof(temp), pp);
	Sleep(3000);

	strcpy_s(cmd, "reg export HKCC C:\\HKCC.reg -y");
	pp = _popen(cmd, "r");
	if (!pp) {
		return;
	}
	fgets(temp, sizeof(temp), pp);
	Sleep(3000);

	strcpy_s(cmd, "reg export HKLM C:\\HKLM.reg -y");
	pp = _popen(cmd, "r");
	if (!pp) {
		return;
	}
	fgets(temp, sizeof(temp), pp);
	Sleep(1000);

	strcpy_s(cmd, "reg export HKU C:\\HKU.reg -y");
	pp = _popen(cmd, "r");
	if (!pp) {
		return;
	}
	fgets(temp, sizeof(temp), pp);
	Sleep(3000);

	char sha1[5][80];
	getSha1("C:\\HKCR.reg", sha1[0]);
	getSha1("C:\\HKCU.reg", sha1[1]);
	getSha1("C:\\HKCC.reg", sha1[2]);
	getSha1("C:\\HKLM.reg", sha1[3]);
	getSha1("C:\\HKU.reg", sha1[4]);
	char fileName[] = "C:\\data\\txtdata\\regedit.txt";
	ofstream file(fileName);
	if (file.is_open()){
		file << "name:HKCR";
		file << "\nsha1:";
		file << sha1[0];
		file << "\nname:HKCU";
		file << "\nsha1:";
		file << sha1[1];
		file << "\nname:HKCC";
		file << "\nsha1:";
		file << sha1[2];
		file << "\nname:HKLM";
		file << "\nsha1:";
		file << sha1[3];
		file << "\nname:HKU";
		file << "\nsha1:";
		file << sha1[4];
	}
	file.close();
}
