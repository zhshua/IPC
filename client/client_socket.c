#define HAVE_REMOTE
#define _CRT_SECURE_NO_WARNINGS
#define _WINSOCK_DEPRECATED_NO_WARNINGS 1
#pragma comment(lib,"ws2_32.lib")
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>
#include <time.h>
#include <io.h>
#include "../config.h"

#define MAX 200
#define BUF_SIZE 512 //缓冲区大小
char message[BUF_SIZE + 1]; // 存储传递的内容
char file_name[MAX][100]; // 存储目录下的所有文件名

void TransferFile(char *dir);
int SaveFilePath(char *dir);
void ErrorHandling(char *message);


int main()
{
	char read_buf[20] = { '\0' };
	readini(CONF_PATH, "client", "delete", read_buf);
	int delete_data = atoi(read_buf);

	memset(read_buf, 0, sizeof(read_buf));
	readini(CONF_PATH, "client", "get_data", read_buf);
	int get_data = atoi(read_buf);

	memset(read_buf, 0, sizeof(read_buf));
	readini(CONF_PATH, "client", "sleep_time", read_buf);
	int sleep_time = atoi(read_buf);

	memset(read_buf, 0, sizeof(read_buf));
	readini(CONF_PATH, "client", "debug", read_buf);
	int debug = atoi(read_buf);

	char arp_mac[20] = { '\0' };
	memset(arp_mac, 0, sizeof(arp_mac));
	readini(CONF_PATH, "client", "arp_mac", arp_mac);

	char arp_ip[30] = { '\0' };
	memset(arp_ip, 0, sizeof(arp_ip));
	readini(CONF_PATH, "client", "arp_ip", arp_ip);
	
	char arp[50] = { '\0' };
	sprintf(arp, "arp -s %s %s", arp_ip, arp_mac);
	//sprintf(arp, "%s ", arp_ip);
	//sprintf(arp, "%s", arp_mac);
	//puts(arp);
	system(arp);

	while (1)
	{
		Sleep(2000);
		
		char delete_txt_cmd[] = "del /q C:\\data\\txtdata\\*";
		if(delete_data)
		system(delete_txt_cmd);

		char delete_pcap_cmd[] = "del /q C:\\data\\pcapdata\\*";
		if(delete_data)
		system(delete_pcap_cmd);

		printf("starting get txt data...\n");

		char get_wmi_cmd[50];
		if (debug)
		strcpy(get_wmi_cmd, "C:\\IPC_project\\WmiTest_debug.exe");
		else
		strcpy(get_wmi_cmd, "C:\\IPC_project\\WmiTest.exe");
		//printf("%d\n", debug);
		if(get_data)
		system(get_wmi_cmd);

		printf("starting get pcap data...\n");
		char get_net_cmd[] = "C:\\IPC_project\\network.exe";
		if(get_data)
		system(get_net_cmd);
		
		Sleep(2000);
		char txtdir[] = "C:\\data\\txtdata";//txt文件的根目录
		printf("starting transfer txtdata...\n");
		TransferFile(txtdir);

		//Sleep(2000);
		char pcapdir[] = "C:\\data\\pcapdata";//pcap文件的根目录
		printf("starting transfer pcapdata...\n");
		TransferFile(pcapdir);
		Sleep(sleep_time);
	}
}

void TransferFile(char *dir)
{
	Sleep(2000);
	int path_num = SaveFilePath(dir);

	char src_ip[20], dst_ip[20], src_port[10], dst_port[10];
	memset(src_ip, 0, sizeof(src_ip));
	memset(dst_ip, 0, sizeof(dst_ip));
	memset(src_port, 0, sizeof(src_port));
	memset(dst_port, 0, sizeof(dst_port));

	readini(CONF_PATH, "client", "src_ip", src_ip);
	readini(CONF_PATH, "client", "dst_ip", dst_ip);
	readini(CONF_PATH, "client", "src_port", src_port);
	readini(CONF_PATH, "client", "dst_port", dst_port);

	WSADATA wsaData;
	SOCKET sock;

	FILE *fp;
	//char ch;
	//int index;
	//char *server_ip = "192.168.17.5";//server ip
	//char *port = "9190";//port

	SOCKADDR_IN servAdr;
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
		ErrorHandling("WSAStartup() error!");

	sock = socket(PF_INET, SOCK_DGRAM, 0);
	if (sock == INVALID_SOCKET)
		ErrorHandling("sock() error!");

	memset(&servAdr, 0, sizeof(servAdr));
	servAdr.sin_family = AF_INET;
	servAdr.sin_addr.s_addr = inet_addr(dst_ip);
	servAdr.sin_port = htons(atoi(dst_port));
	connect(sock, (SOCKADDR*)&servAdr, sizeof(servAdr));

	char temp[30];
	sprintf(temp, "%d", path_num);
	sendto(sock, temp, strlen(temp), 0, (SOCKADDR*)&servAdr, sizeof(servAdr));//发送文件个数

	for (int i = 0; i < path_num; i++)
	{
		memset(message, 0, sizeof(message));

		sendto(sock, file_name[i], strlen(file_name[i]), 0, (SOCKADDR*)&servAdr, sizeof(servAdr));//发送当前文件名

		char path[100] = { '\0' };
		strcat(path, dir);
		strcat(path, "\\");
		strcat(path, file_name[i]);
		//puts(path);

		fp = fopen(path, "rb+");
		if (fp == NULL) {
			printf("file open error");
		}
		else
		{
			//printf("starting transfer %d file...\n", i);
			fseek(fp, 0, SEEK_END);
			int size = ftell(fp);
			rewind(fp);

			char num[30];
			if (size % BUF_SIZE != 0)
				sprintf(num, "%d", size / BUF_SIZE + 1);
			else
				sprintf(num, "%d", size / BUF_SIZE);
			sendto(sock, num, strlen(num), 0, (SOCKADDR*)&servAdr, sizeof(servAdr));//发送当前文件需要接收的次数
																					//printf("%d\n", size);
			for (int j = 0; j < size / BUF_SIZE; j++)
			{
				fread(message, BUF_SIZE, 1, fp);
				sendto(sock, message, BUF_SIZE, 0, (SOCKADDR*)&servAdr, sizeof(servAdr));//分次发送当前文件
				memset(message, 0, sizeof(message));
				Sleep(10);
			}
			if (size % BUF_SIZE != 0)
			{
				fread(message, size % BUF_SIZE, 1, fp);
				sendto(sock, message, size % BUF_SIZE, 0, (SOCKADDR*)&servAdr, sizeof(servAdr));
			}
			fclose(fp);
		}
	}
	printf("every file transfer over\n");

	closesocket(sock);
	WSACleanup();
	return;
}

int SaveFilePath(char *dir)
{
	memset(file_name, 0, sizeof(file_name));
	int num = 0;
	long   hFile = 0;
	struct _finddata_t fileinfo;
	char p[700] = { 0 };
	strcpy(p, dir);
	strcat(p, "\\*");
	if ((hFile = _findfirst(p, &fileinfo)) != -1)
	{
		do
		{
			if ((fileinfo.attrib & _A_SUBDIR))
			{
				if (strcmp(fileinfo.name, ".") != 0 && strcmp(fileinfo.name, "..") != 0)
					continue;
			}
			else
			{
				strcat(file_name[num++], fileinfo.name);
			}
		} while (_findnext(hFile, &fileinfo) == 0);
		_findclose(hFile);
	}
	return num;
}

void ErrorHandling(char *message)
{
	fputs(message, stderr);
	fputc('\n', stderr);
	exit(1);
}