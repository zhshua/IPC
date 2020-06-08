
/** vs环境配置
 *  右键solution -> Compiler -> C/C++ -> General -> SDL checks -> No
 *  右键solution -> Compiler -> C/C++ -> Preprocessor -> preprocessor definitions -> 添加"_CRT_SECURE_NO_WARNINGS"
 **/

//客户端发送文件，服务器端接收文件
#define _WINSOCK_DEPRECATED_NO_WARNINGS 1
#pragma comment(lib,"ws2_32.lib")
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>
#include <time.h>
#include <io.h>
#define MAX 20000

const int BUF_SIZE = 1024;//缓冲区
char message[BUF_SIZE + 1];//存储传递的内容

char file_name[MAX][1000];//存储目录下的所有文件名

void ErrorHandling(char *message);
int SaveFilePath(char *dir);//获取目录下待传输的所有文件
int TransferFile(char *dir);//udp传输文件

int main()
{
	while (1)
	{
		Sleep(2000);
		char txtdir[] = "C:\\data\\txtdata";//txt文件的根目录
		printf("正在传输txtdata...\n");
		if (TransferFile(txtdir) == 0)
			continue;
		//Sleep(2000);
		char pcapdir[] = "C:\\data\\pcapdata";//pcap文件的根目录
		printf("正在传输pcapdata...\n");
		if (TransferFile(pcapdir) == 0)
			continue;
		//Sleep(2000);
	}
	
	return 0;
}

int TransferFile(char *dir)
{
	Sleep(2000);
	int path_num = SaveFilePath(dir);

	WSADATA wsaData;
	SOCKET sock;

	FILE *fp;
	//char ch;
	//int index;
	char *server_ip = "192.168.0.102";//server ip
	char *port = "9190";//port

	SOCKADDR_IN servAdr;
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
		ErrorHandling("WSAStartup() error!");

	sock = socket(PF_INET, SOCK_DGRAM, 0);
	if (sock == INVALID_SOCKET)
		ErrorHandling("sock() error!");

	memset(&servAdr, 0, sizeof(servAdr));
	servAdr.sin_family = AF_INET;
	servAdr.sin_addr.s_addr = inet_addr(server_ip);
	servAdr.sin_port = htons(atoi(port));
	connect(sock, (SOCKADDR*)&servAdr, sizeof(servAdr));

	char start[30] = "startstart";

	send(sock, start, strlen(start), 0);//发送传输文件的请求
	memset(start, 0, sizeof(start));
	recv(sock, start, sizeof(start) - 1, 0);
	if (strcmp(start, "okok") != 0)
	{
		closesocket(sock);
		WSACleanup();
		return 0;

	}
		
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
			printf("正在传输第 %d 个文件...\n", i);
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
	printf("所有文件传输完成\n");
	

	closesocket(sock);
	WSACleanup();
	return 1;
}

void ErrorHandling(char *message)
{
	fputs(message, stderr);
	fputc('\n', stderr);
	exit(1);
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