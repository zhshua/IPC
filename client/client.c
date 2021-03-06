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
#define BUF_SIZE 512 //缓冲区大小
BYTE message[BUF_SIZE + 1]; // 存储传递的内容
char file_name[MAX][100]; // 存储目录下的所有文件名

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
	BYTE tos;//服务类型字段(8位)
	WORD tot_len;//16位IP数据报总长度
	WORD id;//16位标识字段（唯一表示主机发送的每一分数据报）
	WORD frag_off;//(3位分段标志+13位分段偏移数)
	BYTE ttl;//8位数据报生存时间
	BYTE protocol;//协议字段（8位）
	WORD check;//16位首部校验
	DWORD saddr; //源IP地址
	DWORD daddr; //目的IP地址
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
	Sleep(4000);
	int path_num = SaveFilePath(dir);

	char device[100] = { '\0' };
	readini(CONF_PATH, "client", "device", device);

	pcap_t *fp;
	char errbuf[PCAP_ERRBUF_SIZE];
	BYTE byszBuff[1024] = { 0 };

	/* 打开输出设备 */
	if ((fp = pcap_open(device,            // 设备名
		100,                // 要捕获的部分 (只捕获前100个字节)
		PCAP_OPENFLAG_PROMISCUOUS,  // 混杂模式
		1000,               // 读超时时间
		NULL,               // 远程机器验证
		errbuf              // 错误缓冲
	)) == NULL)
	{
		fprintf(stderr, "\nUnable to open the adapter. %s is not supported by WinPcap\n", device);
		return;
	}
	
	BYTE temp[30] = { 0 };
	sprintf(temp, "%d", path_num);
	int nDataLen = BuildPacket(byszBuff, sizeof(byszBuff), temp, strlen(temp));
	pcap_sendpacket(fp, byszBuff, nDataLen); // 发送传输文件个数
	
	for (int i = 0; i < path_num; i++)
	{
		memset(message, 0, sizeof(message));
		nDataLen = BuildPacket(byszBuff, sizeof(byszBuff), file_name[i], strlen(file_name[i]));
		pcap_sendpacket(fp, byszBuff, nDataLen); // 发送当前文件名
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
		fseek(resource, 0, SEEK_END); // 把resource移动到文件尾部
		int size = ftell(resource); // 计算resource对文件首的偏移量，即文件字节大小
		rewind(resource); // 让resource重新回到文件开头

		BYTE num[30] = { 0 };
		if (size % BUF_SIZE != 0)
			sprintf(num, "%d", size / BUF_SIZE + 1);
		else
			sprintf(num, "%d", size / BUF_SIZE);
		nDataLen = BuildPacket(byszBuff, sizeof(byszBuff), num, strlen(num));
		pcap_sendpacket(fp, byszBuff, nDataLen);// 发送当前文件需要接收的次数

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

	ethhdr* pEthHdr = (ethhdr*)pbyBuff;		//填充ether_header
	pEthHdr->dest_hwaddr[0] = 0xff;					//目标MAC地址，广播
	pEthHdr->dest_hwaddr[1] = 0xff;
	pEthHdr->dest_hwaddr[2] = 0xff;
	pEthHdr->dest_hwaddr[3] = 0xff;
	pEthHdr->dest_hwaddr[4] = 0xff;
	pEthHdr->dest_hwaddr[5] = 0xff;

	pEthHdr->source_hwaddr[0] = 0x00;				//这个最好获取一下真实的
	pEthHdr->source_hwaddr[1] = 0x01;
	pEthHdr->source_hwaddr[2] = 0x05;
	pEthHdr->source_hwaddr[3] = 0x3e;
	pEthHdr->source_hwaddr[4] = 0x25;
	pEthHdr->source_hwaddr[5] = 0x53;

	pEthHdr->frame_type = htons(0x0800);

	iphdr* pIPHdr = (iphdr*)(pEthHdr + 1);	//填充iphdr
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
	//计算checksum
	pIPHdr->check = 0;

	udphdr* pUDPHdr = (udphdr*)(pIPHdr + 1);	//填充udphdr
	ZeroMemory(pUDPHdr, sizeof(udphdr));
	pUDPHdr->dest = htons(atoi(dst_port));
	pUDPHdr->source = htons(atoi(src_port));
	pUDPHdr->check = 0;
	//pUDPHdr->check = checksum((WORD*)pUDPHdr, sizeof(udphdr));

	char* pUserData = (char*)(pUDPHdr + 1);
	memcpy(pUserData, DataBuff, nDataLen);
	//int nDataLen = strlen(pUserData);
	//printf("%d\n", nDataLen);
	//TODO: 向pUserData写入长度为nDataLen的用户数据
	pUDPHdr->len = htons(8 + nDataLen);

	//利用udp伪首部计算校验和
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