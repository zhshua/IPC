
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
#define BUF_SIZE 1024
char message[BUF_SIZE + 10];

void ErrorHandling(char *message);
int RecvFile(char *dir);

int main(int argc, char *argv[])
{
    while(1)
    {
        Sleep(1000);
        char txt_dir[100] = "C:\\Users\\ZS\\Desktop\\data\\txtdata";
        printf("正在接收txtdata...\n");
        if(RecvFile(txt_dir) == 0)
            continue;
        //Sleep(1000);
        char pcap_dir[100] = "C:\\Users\\ZS\\Desktop\\data\\pcapdata";
        printf("正在接收pcapdata...\n");
        if(RecvFile(pcap_dir) == 0)
            continue;
        printf("所有文件接收成功\n");
        //Sleep(1000);
    }

    return 0;
}

int RecvFile(char *dir)
{
    Sleep(1000);
    WSADATA wsaData;
    SOCKET servSock;
    int len;
    int clntAdrSz;
    FILE *fp;
    char *server_ip = "192.168.0.102";//server ip
    char *port = "9190";

    SOCKADDR_IN servAdr,clntAdr;

    if(WSAStartup(MAKEWORD(2,2), &wsaData) != 0)
        printf("WSAStartup() error");

    servSock = socket(PF_INET, SOCK_DGRAM, 0);
    if(servSock == INVALID_SOCKET)
        printf("servSock() error");

    memset(&servAdr, 0, sizeof(servAdr));
    servAdr.sin_family = AF_INET;
    servAdr.sin_addr.s_addr = inet_addr(server_ip);
    servAdr.sin_port = htons(atoi(port));

    if(bind(servSock, (SOCKADDR*)&servAdr, sizeof(servAdr)) == SOCKET_ERROR)
        printf("bind() error");

    clntAdrSz = sizeof(clntAdr);

    recvfrom(servSock, message, BUF_SIZE, 0, (SOCKADDR*)&clntAdr, &clntAdrSz);//什么也不做,接收startstart
    sendto(servSock, "okok", 4, 0, (SOCKADDR*)&clntAdr, sizeof(clntAdr));//告诉发送端开始接收

    memset(message, 0, sizeof(message));
    len = recvfrom(servSock, message, BUF_SIZE, 0, (SOCKADDR*)&clntAdr, &clntAdrSz);//接收文件个数
    if(len == -1)
        printf("recvfrom() error\n");

    int path_num = atoi(message);

    for(int i = 0; i < path_num; i++)
    {
        memset(message, 0, sizeof(message));
        char path[100] = {'\0'};
        len = recvfrom(servSock, message, BUF_SIZE, 0, (SOCKADDR*)&clntAdr, &clntAdrSz);//接收文件名
        if(len == -1)
            printf("recvfrom() error\n");
        if(strcmp(message, "startstart") == 0)
        {
            sendto(servSock, "nono", 4, 0, (SOCKADDR*)&clntAdr, sizeof(clntAdr));
            closesocket(servSock);
            WSACleanup();
            return 0;
        }
        puts(message);

        strcat(path, dir);
        strcat(path, "\\");
        strcat(path, message);

        fp = fopen(path, "ab+");
        if(fp == NULL)
        {
            printf("file open error");
        }
        else
        {
            memset(message, 0, sizeof(message));
            len = recvfrom(servSock, message, BUF_SIZE, 0, (SOCKADDR*)&clntAdr, &clntAdrSz);//当前文件需要接收的次数
            if(len == -1)
                printf("recvfrom() error\n");
            if(strcmp(message, "startstart") == 0)
            {
                sendto(servSock, "nono", 4, 0, (SOCKADDR*)&clntAdr, sizeof(clntAdr));
                closesocket(servSock);
                WSACleanup();
                return 0;
            }
            int cnt = atoi(message);
            //printf("%d\n", cnt);
            while(cnt--)
            {
                memset(message, 0, sizeof(message));
                len = recvfrom(servSock, message, BUF_SIZE, 0, (SOCKADDR*)&clntAdr, &clntAdrSz);//分次接收当前文件
                if(len == -1)
                    printf("recvfrom() error\n");
                if(strcmp(message, "startstart") == 0)
                {
                    sendto(servSock, "nono", 4, 0, (SOCKADDR*)&clntAdr, sizeof(clntAdr));
                    closesocket(servSock);
                    WSACleanup();
                    return 0;
                }
                fwrite(message, len, 1, fp);
            }
            fclose(fp);
        }
    }

    closesocket(servSock);
    WSACleanup();
    return 1;
}

void ErrorHandling(char *message)
{
    fputs(message, stderr);
    fputc('\n', stderr);
    exit(1);
}
