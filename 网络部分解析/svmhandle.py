import random
import math
from conf import conf_read
import datetime


def remix():
    fdata = open("svmdata.txt", "w")
    conf = conf_read()
    for i in range(1000000):  # 产生百万个样本集，由于特征量及标签过多，需要的大量的样本集才能保证模型精度，
        # 后续SVM训练模型花了10+小时，不过已经保存好模型

        x = 0

        flow = random.randrange(0, 100000)  # 流量
        login = random.randrange(0, 10)  # 登录次数
        fail = random.randrange(0, login + 1)  # 登录失败次数
        ports = random.randrange(0, 15)  # 访问端口次数
        tcp = random.randrange(3, 15)  # tcp连接数
        start = random.randrange(0, 86399)  # 起始时间（第一个数据包）
        end = random.randrange(start + 1, 86400)  # 截止时间（最后一个数据包）
        speed = math.ceil(flow / (end - start))  # 计算流量速度

        min_flow = int(conf.get_flow())
        min_speed = int(conf.get_speed())
        min_tcp = int(conf.get_tcp())
        min_login = int(conf.get_login())
        min_fail = int(conf.get_fail())
        min_ports = int(conf.get_ports())
        min_start = conf.get_start_time()
        min_end = conf.get_end_time()
        if (min_flow < flow) | (min_speed < speed) | (min_tcp < tcp):  # 打标签，采用四位二进制小数，
            # 从高到低：dos，远程登录，端口扫描，非法时间登录
            x = x + 8
        if (min_login < login) & (min_fail < fail):
            x = x + 4
        if min_ports < ports:
            x = x + 2
        if (start <
            (datetime.datetime.strptime(min_start, '%H:%M:%S') - datetime.datetime(1970, 1, 1)).seconds) \
                | ((datetime.datetime.strptime(min_end, '%H:%M:%S') - datetime.datetime(1970, 1, 1)).seconds
                   < end):
            x = x + 1

        fdata.writelines(str(flow) + "," + str(speed) + "," + str(login) + "," + str(fail) + "," + str(
            ports) + "," + str(tcp) + "," + str(start) + "," + str(end) + "," + str(x) + "\n")  # 保存样本集

    fdata.close()
    print("样本集生成成功")


if __name__ == "__main__":
    remix()
