#define HAVE_REMOTE
#define _CRT_SECURE_NO_WARNINGS
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <pcap.h>
#include <time.h>
#include <io.h>
#include <windows.h>
#include <string.h>
#include "../../config.h"
#pragma comment(lib, "wsock32.lib")
#pragma comment(lib, "wpcap.lib")

#define MAX 200
#define BUF_SIZE 512 //��������С
BYTE message[BUF_SIZE + 1]; // �洢���ݵ�����
char file_name[MAX][100]; // �洢Ŀ¼�µ������ļ���

typedef uint8_t BYTE;
typedef uint16_t WORD;
typedef uint32_t DWORD;

typedef struct
{
	BYTE	dest_hwaddr[6];
	BYTE	source_hwaddr[6];
	WORD	frame_type;
} ethhdr;

typedef struct
{
	BYTE ihl : 4,
		version : 4;
	BYTE tos;//���������ֶ�(8λ)
	WORD tot_len;//16λIP���ݱ��ܳ���
	WORD id;//16λ��ʶ�ֶΣ�Ψһ��ʾ�������͵�ÿһ�����ݱ���
	WORD frag_off;//(3λ�ֶα�־+13λ�ֶ�ƫ����)
	BYTE ttl;//8λ���ݱ�����ʱ��
	BYTE protocol;//Э���ֶΣ�8λ��
	WORD check;//16λ�ײ�У��
	DWORD saddr; //ԴIP��ַ
	DWORD daddr; //Ŀ��IP��ַ
} iphdr;

typedef struct
{
	WORD source;
	WORD dest;
	WORD len;
	WORD check;
} udphdr;

typedef struct
{
	DWORD source_address;
	DWORD dest_address;
	BYTE placeholder;
	BYTE protocol;
	WORD udp_length;
} pseudo_header;

//#define DEVICE "\\Device\\NPF_{2869B1AD-F535-49D7-8982-2069A831F9CB}"
//#define DEVICE "\\Device\\NPF_{B792697D-33AE-400A-BED8-943EB10E4BF0}" //xuniji
//#define DEVICE "\\Device\\NPF_{96E42224-0495-45B7-8AD6-9A90C6537BEC}"

void TransferFile(char *dir);
int SaveFilePath(char *dir);
unsigned short checksum(unsigned short * addr, int count);
int BuildPacket(BYTE *pbyBuff, int nBuffLen, BYTE *DataBuff, int nDataLen);

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

		//Sleep(2000);
		char txtdir[] = "C:\\data\\txtdata";//txt�ļ��ĸ�Ŀ¼
		printf("starting transfer txtdata...\n");
		TransferFile(txtdir);

		//Sleep(2000);
		char pcapdir[] = "C:\\data\\pcapdata";//pcap�ļ��ĸ�Ŀ¼
		printf("starting transfer pcapdata...\n");
		TransferFile(pcapdir);
		Sleep(sleep_time);
	}
}

void TransferFile(char *dir)
{
	Sleep(4000);
	int path_num = SaveFilePath(dir);

	char device[100] = { '\0' };
	readini(CONF_PATH, "client", "device", device);

	pcap_t *fp;
	char errbuf[PCAP_ERRBUF_SIZE];
	BYTE byszBuff[1024] = { 0 };

	/* ������豸 */
	if ((fp = pcap_open(device,            // �豸��
		100,                // Ҫ����Ĳ��� (ֻ����ǰ100���ֽ�)
		PCAP_OPENFLAG_PROMISCUOUS,  // ����ģʽ
		1000,               // ����ʱʱ��
		NULL,               // Զ�̻�����֤
		errbuf              // ���󻺳�
	)) == NULL)
	{
		fprintf(stderr, "\nUnable to open the adapter. %s is not supported by WinPcap\n", device);
		return;
	}
	
	BYTE temp[30] = { 0 };
	sprintf(temp, "%d", path_num);
	int nDataLen = BuildPacket(byszBuff, sizeof(byszBuff), temp, strlen(temp));
	pcap_sendpacket(fp, byszBuff, nDataLen); // ���ʹ����ļ�����
	
	for (int i = 0; i < path_num; i++)
	{
		memset(message, 0, sizeof(message));
		nDataLen = BuildPacket(byszBuff, sizeof(byszBuff), file_name[i], strlen(file_name[i]));
		pcap_sendpacket(fp, byszBuff, nDataLen); // ���͵�ǰ�ļ���
		puts(message);

		char path[100] = { '\0' };
		strcat(path, dir);
		strcat(path, "\\");
		strcat(path, file_name[i]);

		FILE *resource = fopen(path, "rb+");
		if (resource == NULL) {
			printf("file open error\n");
			return;
		}

		printf("starting transfer the num. %d file...\n", i);
		fseek(resource, 0, SEEK_END); // ��resource�ƶ����ļ�β��
		int size = ftell(resource); // ����resource���ļ��׵�ƫ���������ļ��ֽڴ�С
		rewind(resource); // ��resource���»ص��ļ���ͷ

		BYTE num[30] = { 0 };
		if (size % BUF_SIZE != 0)
			sprintf(num, "%d", size / BUF_SIZE + 1);
		else
			sprintf(num, "%d", size / BUF_SIZE);
		nDataLen = BuildPacket(byszBuff, sizeof(byszBuff), num, strlen(num));
		pcap_sendpacket(fp, byszBuff, nDataLen);// ���͵�ǰ�ļ���Ҫ���յĴ���

		for (int j = 0; j < size / BUF_SIZE; j++)
		{
			memset(message, 0, sizeof(message));
			fread(message, BUF_SIZE, 1, resource);
			nDataLen = BuildPacket(byszBuff, sizeof(byszBuff), message, BUF_SIZE);
			//printf("%d\n", nDataLen);
			pcap_sendpacket(fp, byszBuff, nDataLen);
			Sleep(10);
		}
		if (size % BUF_SIZE != 0)
		{
			memset(message, 0, sizeof(message));
			fread(message, size % BUF_SIZE, 1, resource);
			nDataLen = BuildPacket(byszBuff, sizeof(byszBuff), message, size % BUF_SIZE);
			pcap_sendpacket(fp, byszBuff, nDataLen);
		}
		fclose(resource);
	}
	printf("every file transfer over\n");
	pcap_close(fp);
}

unsigned short
checksum(unsigned short * addr, int count)
{
	long sum = 0;
	while (count > 1)
	{
		sum += *(unsigned short*)addr++;
		count -= 2;
	}

	if (count > 0)
	{
		char left_over[2] = { 0 };
		left_over[0] = *addr;
		sum += *(unsigned short*)left_over;
	}

	while (sum >> 16)
		sum = (sum & 0xffff) + (sum >> 16);

	return ~sum;
}

int BuildPacket(BYTE *pbyBuff, int nBuffLen, BYTE *DataBuff, int nDataLen)
{
	if (pbyBuff == NULL)
		return -1;

	char src_ip[20], dst_ip[20], src_port[10], dst_port[10];
	memset(src_ip, 0, sizeof(src_ip));
	memset(dst_ip, 0, sizeof(dst_ip));
	memset(src_port, 0, sizeof(src_port));
	memset(dst_port, 0, sizeof(dst_port));

	readini(CONF_PATH, "client", "src_ip", src_ip);
	readini(CONF_PATH, "client", "dst_ip", dst_ip);
	readini(CONF_PATH, "client", "src_port", src_port);
	readini(CONF_PATH, "client", "dst_port", dst_port);

	ethhdr* pEthHdr = (ethhdr*)pbyBuff;		//���ether_header
	pEthHdr->dest_hwaddr[0] = 0xff;					//Ŀ��MAC��ַ���㲥
	pEthHdr->dest_hwaddr[1] = 0xff;
	pEthHdr->dest_hwaddr[2] = 0xff;
	pEthHdr->dest_hwaddr[3] = 0xff;
	pEthHdr->dest_hwaddr[4] = 0xff;
	pEthHdr->dest_hwaddr[5] = 0xff;

	pEthHdr->source_hwaddr[0] = 0x00;				//�����û�ȡһ����ʵ��
	pEthHdr->source_hwaddr[1] = 0x01;
	pEthHdr->source_hwaddr[2] = 0x05;
	pEthHdr->source_hwaddr[3] = 0x3e;
	pEthHdr->source_hwaddr[4] = 0x25;
	pEthHdr->source_hwaddr[5] = 0x53;

	pEthHdr->frame_type = htons(0x0800);

	iphdr* pIPHdr = (iphdr*)(pEthHdr + 1);	//���iphdr
	ZeroMemory(pIPHdr, sizeof(pIPHdr));
	pIPHdr->version = 4;
	pIPHdr->ihl = 5;
	pIPHdr->tos = 0;
	pIPHdr->ttl = 0x80;
	pIPHdr->id = htons(0x1000);
	pIPHdr->frag_off = htons(0);
	pIPHdr->tot_len = htons(102);
	pIPHdr->protocol = 17;
	pIPHdr->saddr = inet_addr(src_ip);
	pIPHdr->daddr = inet_addr(dst_ip);
	//����checksum
	pIPHdr->check = 0;

	udphdr* pUDPHdr = (udphdr*)(pIPHdr + 1);	//���udphdr
	ZeroMemory(pUDPHdr, sizeof(udphdr));
	pUDPHdr->dest = htons(atoi(dst_port));
	pUDPHdr->source = htons(atoi(src_port));
	pUDPHdr->check = 0;
	//pUDPHdr->check = checksum((WORD*)pUDPHdr, sizeof(udphdr));

	char* pUserData = (char*)(pUDPHdr + 1);
	memcpy(pUserData, DataBuff, nDataLen);
	//int nDataLen = strlen(pUserData);
	//printf("%d\n", nDataLen);
	//TODO: ��pUserDataд�볤��ΪnDataLen���û�����
	pUDPHdr->len = htons(8 + nDataLen);

	//����udpα�ײ�����У���
	pUDPHdr->check = 0;
	pseudo_header psh;
	psh.source_address = inet_addr(src_ip);
	psh.dest_address = inet_addr(dst_ip);
	psh.placeholder = 0;
	psh.protocol = 17;
	psh.udp_length = htons(8 + nDataLen);

	int psize = sizeof(pseudo_header) + sizeof(udphdr) + nDataLen;
	char *pseudogram = malloc(psize);

	memcpy(pseudogram, (char*)&psh, sizeof(pseudo_header));
	memcpy(pseudogram + sizeof(pseudo_header), pUDPHdr, sizeof(udphdr) + nDataLen);
	pUDPHdr->check = checksum((WORD*)pseudogram, psize);


	int nPackLen = sizeof(ethhdr) + sizeof(iphdr) + sizeof(udphdr) + nDataLen;
	pIPHdr->tot_len = htons(nPackLen - sizeof(ethhdr));
	pIPHdr->check = checksum((WORD*)pIPHdr, sizeof(iphdr));

	return nPackLen;
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