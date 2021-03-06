#ifndef CONFIG_H_INCLUDED
#define CONFIG_H_INCLUDED

#define _CRT_SECURE_NO_WARNINGS 1
#define CONF_PATH "C:\\IPC_project\\conf.ini"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//去除字符串的左右两端的空格
char *s_trim(char *str)
{
    char *s = str;
    char *copied, *tail = NULL;
    if(str == NULL || *str == '\0')
        return str;
    if((str[0]  != ' ') && (str[strlen(str)-1] != ' '))
        return str;
    for(copied = str; *str; str++){
        if(*str  != ' ' && *str != '\t' && *str != '\r' &&  *str != '\n'){
            *copied++ =  *str;
            tail = copied;
        }
        else{
            if(tail)
                *copied++ = *str;
        }
    }
    if(tail)
        *tail = 0;
    else
        *copied = 0;
    return s;
}

//实现读取配置文件的一行，当行的开头为#时则跳过这行
void readline(char *buf, int size, FILE *fp)
{
    char c;
    int i = 0;
    while((c = fgetc(fp)) != EOF && c != '\n' && c != '#') {
        if(i >= size)
            break;
        buf[i++] = c;
    }
    while(c != '\n' && c != EOF)
        c = fgetc(fp);
}

int readini(const char *file, const char *sec, const char *param, char *value)
{
    FILE *fp = NULL;
    char buf[1024];
    char *s;
	if ((fp = fopen(file, "r")) == NULL) {
		printf("file open error\n");
		return -1;
	}
    while(!feof(fp)){
        memset(buf, 0, sizeof(buf));
        readline(buf, sizeof(buf), fp);
        s_trim(buf);
        if(buf[0] != '[')
            continue;
        for(s = buf+1; *s != ']'; s++);
        *s = '\0';
        s_trim(buf+1);
        if(strcmp(sec, buf+1) != 0)
            continue;
        while(!feof(fp)){
            memset(buf, 0, sizeof(buf));
            readline(buf, sizeof(buf), fp);
            s_trim(buf);
            if(buf[0] == '[')
               break;
            if(buf[0] == '\0' || buf[0] == '#')
                continue;
            for(s=buf; *s != '='; s++);
            *s = '\0';
            s_trim(buf);
            if (strcmp(param, buf) == 0) {
                memcpy(value, s+1, strlen(s+1));
                value[strlen(s+1)] = '\0';
                fclose(fp);
                return 0;
           }
        }
    }
    printf("%s read error!", param);
    fclose(fp);
    return -1;
}


#endif // CONFIG_H_INCLUDED
