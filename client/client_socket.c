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
#define BUF_SIZE 512 
char message[BUF_SIZE + 1]; 
char file_name[MAX][100]; 

void TransferFile(char *dir);
int SaveFilePath(char *dir);
void ErrorHandling(char *message);

/*****************file transform struct******************/
#define FILE_TARNSFORM_BUFFER_SIZE 640 
typedef struct fileSeqHead
{
	int name;
	int sequence;
}fileSeqHead;
typedef struct file
{
	fileSeqHead fileHead;
	char buff[FILE_TARNSFORM_BUFFER_SIZE];

}file;
/*****************file transform struct******************/


int main()
{
	char read_buf[20] = { '\0' };
	readini(CONF_PATH, "client", "delete", read_buf);
	int delete_data = atoi(read_buf);

	memset(read_buf, 0, sizeof(read_buf));
	readini(CONF_PATH, "client", "get_txtdata", read_buf);
	int get_txtdata = atoi(read_buf);

	memset(read_buf, 0, sizeof(read_buf));
	readini(CONF_PATH, "client", "get_pcapdata", read_buf);
	int get_pcapdata = atoi(read_buf);

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

	while (1)
	{
		Sleep(2000);
		system(arp);
		

		char delete_txt_cmd[] = "del /q C:\\data\\txtdata\\*";
		if (delete_data)
			system(delete_txt_cmd);

		char delete_pcap_cmd[] = "del /q C:\\data\\pcapdata\\*";
		if (delete_data)
			system(delete_pcap_cmd);

		printf("starting get txt data...\n");

		char get_wmi_cmd[50];
		if (debug)
			strcpy(get_wmi_cmd, "C:\\IPC_project\\WmiTest_debug.exe");
		else
			strcpy(get_wmi_cmd, "C:\\IPC_project\\WmiTest.exe");
		//printf("%d\n", debug);

		if (get_txtdata)
			system(get_wmi_cmd);

		printf("starting get pcap data...\n");
		char get_net_cmd[] = "C:\\IPC_project\\network.exe";
		printf("start pcap...\n");
		Sleep(10000);
		if (get_pcapdata)
			system(get_net_cmd);


		Sleep(2000);
		system(arp);
		char txtdir[] = "C:\\data\\txtdata";//txt????????
		printf("starting transfer txtdata...\n");
		TransferFile(txtdir);

		
		//Sleep(2000);
		if (get_pcapdata)
		{
			char pcapdir[] = "C:\\data\\pcapdata";//pcap????????
			printf("starting transfer pcapdata...\n");
			TransferFile(pcapdir);
		}
		
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
	SOCKADDR_IN servAdr;
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
		ErrorHandling("WSAStartup() error!");
		return;
	}
	sock = socket(PF_INET, SOCK_DGRAM, 0);
	if (sock == INVALID_SOCKET) {
		ErrorHandling("sock() error!");
		WSACleanup();
		return;
	}

	memset(&servAdr, 0, sizeof(servAdr));
	servAdr.sin_family = AF_INET;
	servAdr.sin_addr.s_addr = inet_addr(dst_ip);
	servAdr.sin_port = htons(atoi(dst_port));
	connect(sock, (SOCKADDR*)&servAdr, sizeof(servAdr));


	file filesum = { 0 };
	filesum.fileHead.name = 0x5A5A5A5A;
	filesum.fileHead.sequence = path_num;
	sendto(sock, (char *)&filesum, sizeof(file), 0, (SOCKADDR*)&servAdr, sizeof(servAdr));


	for (int i = 0; i < path_num; ++i)
	{
		file Curfile = { 0 };
		char path[100] = { '\0' };
		strcat(path, dir);
		strcat(path, "\\");
		strcat(path, file_name[i]);

		fp = fopen(path, "rb");
		if (fp == NULL) {
			printf("file open error");
			closesocket(sock);
			WSACleanup();
			return;
		}
		Curfile.fileHead.name = i;
		memcpy(Curfile.buff, file_name[i], strlen(file_name[i]));
		Curfile.fileHead.sequence = 0;
		sendto(sock, (char *)&Curfile, sizeof(file), 0, (SOCKADDR*)&servAdr, sizeof(servAdr));
		int curFilePartNum = 1;  
		while (!feof(fp)) {
			int len = fread(Curfile.buff, 1, FILE_TARNSFORM_BUFFER_SIZE, fp);
			Curfile.fileHead.sequence = curFilePartNum;
			sendto(sock, (char *)&Curfile, len + sizeof(fileSeqHead), 0, (SOCKADDR*)&servAdr, sizeof(servAdr));
			curFilePartNum++;
		}
		Curfile.fileHead.sequence = -1;
		memset(Curfile.buff, 0, FILE_TARNSFORM_BUFFER_SIZE);
		sendto(sock, (char *)&Curfile, sizeof(fileSeqHead), 0, (SOCKADDR*)&servAdr, sizeof(servAdr));

		fclose(fp);
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