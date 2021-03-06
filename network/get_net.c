#define LINE_LEN 16
#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "wpcap.lib")
#include <stdio.h>
#include <stdlib.h>
#include <pcap.h>
#include <remote-ext.h>
#include <time.h>
#include <conio.h>
#include "../../config.h"

void packet_handler(u_char * param, const struct pcap_pkthdr * header, const  u_char *pkt_data/*,const u_char *param1*/);

int z = 1, k = 1;
int iNum = 1;//网络接口
int sec = 5;//保存间隔s
int min = 30;//捕获时长s
char name[100] = "\\catch_num1_data.pcap";//这个别改
char path[200] = "C:\\data\\pcapdata\\";//保存路径
char path1[200];
clock_t t1;
pcap_t * adhandle;

int main()
{
	pcap_if_t * alldevs, *device;
	int i = 0;
	u_int netmask;
	struct bpf_program fcode;
	//pcap_t * adhandle;
	char errbuf[PCAP_ERRBUF_SIZE];
	pcap_dumper_t *dumpfile;
	char packet_filter[] = "(ip and udp) or (ip and tcp) or (ip and icmp)";

	if (pcap_findalldevs_ex(PCAP_SRC_IF_STRING, NULL, &alldevs, errbuf) == -1)
	{
		fprintf(stderr, "unable to open network device:%s\n", errbuf);
		return 1;
	}
	for (device = alldevs; device != NULL; device = device->next)
	{
		if (i == 0)
		{
			printf("press CTRL + C to stop!\n\n");
			printf("The network equipment is as follows:\n");
		}
		printf("%d. %s\n", ++i, device->name);
		if (device->description)
			printf(" (%s)\n", device->description);
		else
			printf("No device description information!");
	}
	if (i == 0)
	{
		printf("\nPlease install WinPcap first!");
		return -1;
	}
	//printf("\nPlease select a network device interface:(1 - %d):", i);
	//scanf("%d", &iNum);
	//printf("\n请选择单个文件时间长度（秒）:");
	//scanf("%d", &sec);
	//printf("\n请选择捕获运行时间（分钟）:");
	//scanf("%d", &min);
	//printf("\n请选择保存路径(双\\分隔符):");
	//scanf("%s", &path);
	strcpy(path1, path);

	char adapter_num[10] = { '\0' };
	readini(CONF_PATH, "network", "adapter_num", adapter_num);
	iNum = atoi(adapter_num);

	if (iNum < 1 || iNum > i)
	{
		printf("Device does not exist!\n");
		pcap_freealldevs(alldevs);
		return -1;
	}

	for (device = alldevs, i = 0; i < iNum - 1; device = device->next, i++);

	if ((adhandle = pcap_open(device->name,
		65536,
		PCAP_OPENFLAG_PROMISCUOUS,
		1000,
		NULL,
		errbuf
	)) == NULL)
	{
		fprintf(stderr, "\nCan't open adapter！\n");

		pcap_freealldevs(alldevs);
		return -1;
	}

	if (pcap_datalink(adhandle) != DLT_EN10MB)
	{
		fprintf(stderr, "\nSystem network card link error!\n");
		pcap_freealldevs(alldevs);
		return -1;
	}

	if (device->addresses != NULL)
		netmask = ((struct sockaddr_in *)(device->addresses->netmask))->sin_addr.S_un.S_addr;
	else
		netmask = 0xffff00;
	if (pcap_compile(adhandle, &fcode, packet_filter, 1, netmask) < 0)
	{
		fprintf(stderr, "Can't monitor and filter the datagram!\n");
		pcap_freealldevs(alldevs);
		return -1;
	}

	if (pcap_setfilter(adhandle, &fcode) < 0)
	{
		fprintf(stderr, "Filter setting error!\n");
		pcap_freealldevs(alldevs);
		return -1;
	}
	printf("Listening to data packets passing through the %s...\n", device->description);
	pcap_freealldevs(alldevs);


	t1 = clock();

	strcat(path1, name);
	dumpfile = pcap_dump_open(adhandle, path1);

	if (dumpfile == NULL)
	{
		fprintf(stderr, "\nError opening output file\n");
		return -1;
	}

	pcap_loop(adhandle, 0, packet_handler, (unsigned char *)dumpfile);

	return 0;
}

void packet_handler(u_char *dumpfile, const struct pcap_pkthdr *pkt_header, const u_char *pkt_data)
{
	//printf("捕获到第%d批的共%d个数据包，第%u秒\n",k, z++,((clock()-t1)/ CLOCKS_PER_SEC));
	printf("catch %d packets\n", z++);
	if (clock() > (t1 + min * CLOCKS_PER_SEC)) {
		printf("catch stop\n");
		//system("pause");
		pcap_breakloop(adhandle);
		return;
	}
	else if (clock() > (t1 + k * sec * CLOCKS_PER_SEC)) {
		memset(path1, '\0', sizeof(path1));
		strcpy(path1, path);
		sprintf(name, "\\catch_num%d_data.pcap", ++k);
		strcat(path1, name);
		dumpfile = (u_char *)pcap_dump_open(adhandle, path1);
		pcap_dump(dumpfile, pkt_header, pkt_data);
		if (_kbhit()) {
			printf("catch stop\n");
			system("pause");
		}
		pcap_loop(adhandle, 0, packet_handler, (unsigned char *)dumpfile);
	}
	pcap_dump(dumpfile, pkt_header, pkt_data);
	return;
}
