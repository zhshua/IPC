
/** vs��������
 *  �Ҽ�solution -> Compiler -> C/C++ -> General -> SDL checks -> No
 *  �Ҽ�solution -> Compiler -> C/C++ -> Preprocessor -> preprocessor definitions -> ���"_CRT_SECURE_NO_WARNINGS"
 **/

//�ͻ��˷����ļ����������˽����ļ�
#define _WINSOCK_DEPRECATED_NO_WARNINGS 1
#pragma comment(lib,"ws2_32.lib")
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>
#include <time.h>
#include <io.h>
#define MAX 20000

const int BUF_SIZE = 1024;//������
char message[BUF_SIZE + 1];//�洢���ݵ�����

char file_name[MAX][1000];//�洢Ŀ¼�µ������ļ���

void ErrorHandling(char *message);
int SaveFilePath(char *dir);//��ȡĿ¼�´�����������ļ�
void TransferFile(char *dir);//udp�����ļ�

int main()
{
	char txtdir[] = "C:\\Users\\ZS\\Desktop\\project\\txtdata";//txt�ļ��ĸ�Ŀ¼
	printf("���ڴ���txtdata...\n");
	TransferFile(txtdir);
	Sleep(2000);
	char pcapdir[] = "C:\\Users\\ZS\\Desktop\\project\\pcapdata";//pcap�ļ��ĸ�Ŀ¼
	printf("���ڴ���pcapdata...\n");
	TransferFile(pcapdir);
	return 0;
}

void TransferFile(char *dir)
{
	int path_num = SaveFilePath(dir);

	WSADATA wsaData;
	SOCKET sock;

	FILE *fp;
	//char ch;
	//int index;
	char *server_ip = "192.168.0.103";//server ip
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

	char temp[30];
	sprintf(temp, "%d", path_num);
	sendto(sock, temp, strlen(temp), 0, (SOCKADDR*)&servAdr, sizeof(servAdr));//�����ļ�����

	for (int i = 0; i < path_num; i++)
	{
		memset(message, 0, sizeof(message));

		sendto(sock, file_name[i], strlen(file_name[i]), 0, (SOCKADDR*)&servAdr, sizeof(servAdr));//���͵�ǰ�ļ���

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
			printf("���ڴ���� %d ���ļ�...\n", i);
			fseek(fp, 0, SEEK_END);
			int size = ftell(fp);
			rewind(fp);

			char num[30];
			if (size % BUF_SIZE != 0)
				sprintf(num, "%d", size / BUF_SIZE + 1);
			else
				sprintf(num, "%d", size / BUF_SIZE);
			sendto(sock, num, strlen(num), 0, (SOCKADDR*)&servAdr, sizeof(servAdr));//���͵�ǰ�ļ���Ҫ���յĴ���
			//printf("%d\n", size);
			for (int j = 0; j < size / BUF_SIZE; j++)
			{
				fread(message, BUF_SIZE, 1, fp);
				sendto(sock, message, BUF_SIZE, 0, (SOCKADDR*)&servAdr, sizeof(servAdr));//�ִη��͵�ǰ�ļ�
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
	printf("�����ļ��������\n");
	

	closesocket(sock);
	WSACleanup();
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