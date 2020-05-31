import pandas as pd
import datetime
from white import whi
import IPy
from io import StringIO
import math
import time
import mysql.connector


def fx(IP, file, mydb, startid, sum):  # 待分析数据路径，分析结果保存路径，数据库登录信息
    pd.set_option('display.width', None)

    pd.set_option('display.max_colwidth', 1000)

    mycursor = mydb.cursor()
    #
    # mycursor.execute("SELECT * FROM 解析表 LIMIT " + str(startid - 1) + "," + str(sum) + "; ")
    # last = mycursor.fetchall()
    # result1 = ''.join(str(last))
    # # print(result1)
    # # result1=result1.split()
    # result1 = result1.replace('[', '').replace(']', '').replace('(', '').replace('),', '\n').replace("'", '').replace(
    #     ')', '').replace(' ', '')
    # result1 = "日期,时间,源MAC,目的MAC,源IP,目的IP,协议类型,源端口,目的端口,tcp标志位,包长度,源设备号,源设备端口,目的设备号,目的设备端口,来源,id\n" + result1
    # # print(result1)
    # f = StringIO(result1)
    # df = pd.read_csv(f)
    # # df = pd.read_csv(data)  # 读取解析结果

    sqlcom = 'select * from 解析表 limit ' + str(startid - 1) + "," + str(sum)
    df = pd.read_sql(sqlcom, con=mydb)

    end = df.iloc[df.shape[0] - 1, 1]  # 提取起始与终止时间
    start = df.iloc[0, 1]
    start = datetime.datetime.strptime(start, '%H:%M:%S')
    end = datetime.datetime.strptime(end, '%H:%M:%S')
    sub = end - start  # 计算时间差

    df['包长度'] = df['包长度'].astype(int)

    flow = int(df.sum()['包长度'])  # 计算总流量大小（byte）

    if sub.seconds == 0:
        flowspeed = (flow / 1)  # 计算流量速度
    else:
        flowspeed = (flow / sub.seconds)  # 计算流量速度
    # print("流量大小：", flow, "byte")
    # print("流量速度", flowspeed)
    tcpcon = 0
    dt = df[(df.目的IP == IP) & ((df.协议类型 == "TCP") | (df.协议类型 == "AMS"))]  # 为TCP连接数与端口统计筛选范围

    for i in range(dt.shape[0]):
        if dt.iloc[i, 9] == 2:  # Flags标志位SYN-0x0002
            tcpcon = tcpcon + 1

    port = dt.groupby('目的端口').describe().reset_index()
    portnum = port.shape[0]  # 统计被访问端口的数量

    dt = df[(df.目的IP == IP) & ((df.目的端口 == 3389) | (df.源端口 == 3389))]  # 为远程连接筛选范围，3389为远程连接默认端口
    # dt = df[(df.目的端口 == 3389) | (df.srcport == 3389)]
    lg = lgout = lgsuccess = lgfail = m = 0
    conset = False
    lgset = False  # 设立两个表示连接，登录的状态标志位

    for j in range(dt.shape[0]):  # 根据数据包传输层flag标志位，及其出现顺序、间隔的逻辑判断远程登录状态
        if (dt.iloc[j, 9] == 20) & (lgset is False) & (conset is False):
            lg = lg + 1
            lgfail = lgfail + 1
            lgset = True
            m = 0
        if (dt.iloc[j, 9] == 2) & (0 < m < 2) & (lgset is True):
            lgsuccess = lgsuccess + 1
            lgfail = lgfail - 1
            conset = True
            lgset = False

        if (2 <= m) & (lgset is True):
            # lgfail = lgfail + 1
            lgset = False

        if (dt.iloc[j, 9] == 20) & (conset is True):
            lgout = lgout + 1
            conset = False
        m = m + 1
    # print(lg, lgsuccess, lgfail, lgout)

    dt=df

    for z in range(df.shape[0]):  # 白名单统计前的网段过滤

        if (((str(df.iloc[z, 4]) in IPy.IP("0.0.0.0-127.255.255.255")) is True) \
            | ((str(df.iloc[z, 4]) in IPy.IP("128.0.0.0-191.255.255.255")) is True) \
            | ((str(df.iloc[z, 4]) in IPy.IP("192.0.0.0-223.255.255.255")) is True)) & \
                (str(df.iloc[z, 5]) == IP):
            continue
        else:
            dt = dt.drop([z])
    dt=dt.reset_index(drop=True)

    if dt.shape[0] == 0:
        white = '当前网络无有效流量'  # 白名单检测，统计符合白名单的数据包数量
    else:
        white = 1 - (whi(dt, mydb).shape[0] / dt.shape[0])  # 白名单检测，统计符合白名单的数据包数量

    sql = "INSERT INTO 分析表 (流量,流量速度,登录次数,登录失败次数,端口遍历数,tcp连接数,起始时间,终止时间,合法数据包比例,来源) " \
          "VALUES (%s,%s,%s,%s,%s,%s,%s,%s,%s,%s)"

    val = [(str(flow), str(int(flowspeed)), str(lg), str(lgfail), str(portnum), str(tcpcon),
            str(start.strftime('%H:%M:%S')),
            str(end.strftime('%H:%M:%S')), str(white), str(file))]
    mycursor.executemany(sql, val)

    a = mycursor.lastrowid

    mydb.commit()  # 写入数据库

    # fresult = open(path, "w")
    # fresult.writelines(str(flow) + "," + str(flowspeed) + "," + str(lg) + ","
    #                    + str(lgfail) + "," + str(portnum) + "," + str(tcpcon) + "," + str(
    #     (start - datetime.datetime(1970, 1, 1)).seconds) + ","
    #                    + str((end - datetime.datetime(1970, 1, 1)).seconds))
    # fresult.close()  # 文本形式保存

    return a


def insert_dos(file, mydb, startid, sum):
    # df = pd.read_csv(data)  # 读取解析结果
    mycursor = mydb.cursor()

    mycursor.execute("SELECT * FROM 解析表 LIMIT " + str(startid - 1) + "," + str(sum) + "; ")
    last = mycursor.fetchall()
    result1 = ''.join(str(last))

    result1 = result1.replace('[', '').replace(']', '').replace('(', '').replace('),', '\n').replace("'", '').replace(
        ')', '').replace(' ', '')
    result1 = "日期,时间,源MAC,目的MAC,源IP,目的IP,协议类型,源端口,目的端口,tcp标志位,包长度,源设备号,源设备端口,目的设备号,目的设备端口,来源,id\n" + result1
    # print(result1)
    f = StringIO(result1)
    df = pd.read_csv(f)
    datee = df.iloc[0, 0]
    timee = df.iloc[0, 1]
    df1 = df.groupby('时间')

    df1 = df1.sum()['包长度']

    end = df.iloc[df.shape[0] - 1, 1]  # 提取起始与终止时间
    start = df.iloc[0, 1]
    start = datetime.datetime.strptime(start, '%H:%M:%S')
    end = datetime.datetime.strptime(end, '%H:%M:%S')
    sub = end - start  # 计算时间差

    # j = 0
    # tflow = 0
    v = 0
    mycursor = mydb.cursor()

    for i in range(int(sub.seconds) + 1):
        t = (start + datetime.timedelta(seconds=i)).strftime("%H:%M:%S")
        if i % 10 == 0:
            tflow = 0
            j = 0
            t = (start + datetime.timedelta(seconds=i)).strftime("%H:%M:%S")
            for j in range(10):
                if (i + j) <= int(sub.seconds):
                    if (start + datetime.timedelta(seconds=i + j)).strftime("%H:%M:%S") == df1.index[v]:
                        tflow = tflow + df1[v]
                        v = v + 1
            sql = "INSERT INTO 流量时序表 (日期,时间,流量,来源) VALUES (%s,%s,%s,%s)"
            val = (str(datee), t, str(tflow), str(file))
            mycursor.execute(sql, val)
            mydb.commit()  # 写入数据库
        # tflow = 0
        # if t == df1.index[j]:
        #     tflow = df1[j]
        #     j = j + 1
        # else:
        #     tflow = 0

        # sql = "INSERT INTO 流量时序表 (时间,流量,来源) VALUES (%s,%s,%s)"
        # val = (t, str(tflow), str(file))
        #
        # mycursor.execute(sql, val)
        # mydb.commit()  # 写入数据库


def insert_port(IP, file, mydb, startid, sum):
    # df = pd.read_csv(data)  # 读取解析结果
    mycursor = mydb.cursor()

    mycursor.execute("SELECT * FROM 解析表 LIMIT " + str(startid - 1) + "," + str(sum) + "; ")
    last = mycursor.fetchall()
    result1 = ''.join(str(last))
    # print(result1)
    # result1=result1.split()
    result1 = result1.replace('[', '').replace(']', '').replace('(', '').replace('),', '\n').replace("'", '').replace(
        ')', '').replace(' ', '')
    result1 = "日期,时间,源MAC,目的MAC,源IP,目的IP,协议类型,源端口,目的端口,tcp标志位,包长度,源设备号,源设备端口,目的设备号,目的设备端口,来源,id\n" + result1
    # print(result1)
    f = StringIO(result1)
    df = pd.read_csv(f)
    datee = df.iloc[0, 0]
    timee = df.iloc[0, 1]
    dt = df[(df.目的IP == IP)]
    dt = dt[['源IP', '目的端口']].reset_index(drop=True)
    # # dt=dt.groupby("srcport").value_count()
    # dt1=dt.groupby("源IP").describe().reset_index()
    # # dt=pd.Series(df)
    # # dt.sum()[]
    # dt = dt.groupby("源IP").describe().reset_index()
    # dt = dt["源IP"].value_counts()
    dt = dt.drop_duplicates(subset=['源IP', '目的端口'], keep='first').reset_index(drop=True)
    dt = dt.groupby('源IP').count()

    mycursor = mydb.cursor()
    # table = input("请输入保存预测结果的表名：")
    # table = "table2"
    # sql = "DROP TABLE IF EXISTS " + table  # 删除数据表
    # mycursor.execute(sql)
    # mycursor.execute(  # 创建数据表
    #     "CREATE TABLE " + table + " (IP VARCHAR(255),ports VARCHAR(255))")

    for i in range(dt.shape[0]):
        sql = "INSERT INTO 端口遍历表 (日期,时间,IP,遍历端口数,来源) VALUES (%s,%s,%s,%s,%s)"
        val = (str(datee), str(timee), dt.index[i], str(dt.iloc[i, 0]), str(file))

        mycursor.execute(sql, val)
        mydb.commit()  # 写入数据库


def insert_login(IP, file, mydb, startid, sum):
    # df = pd.read_csv(data)  # 读取解析结果
    mycursor = mydb.cursor()

    mycursor.execute("SELECT * FROM 解析表 LIMIT " + str(startid - 1) + "," + str(sum) + "; ")
    last = mycursor.fetchall()
    result1 = ''.join(str(last))
    # print(result1)
    # result1=result1.split()
    result1 = result1.replace('[', '').replace(']', '').replace('(', '').replace('),', '\n').replace("'", '').replace(
        ')', '').replace(' ', '')
    result1 = "日期,时间,源MAC,目的MAC,源IP,目的IP,协议类型,源端口,目的端口,tcp标志位,包长度,源设备号,源设备端口,目的设备号,目的设备端口,来源,id\n" + result1
    # print(result1)
    f = StringIO(result1)
    df = pd.read_csv(f)
    datee = df.iloc[0, 0]
    timee = df.iloc[0, 1]
    dt = df[(df.目的IP == IP) & ((df.目的端口 == 3389) | (df.源端口 == 3389))]  # 为远程连接筛选范围，3389为远程连接默认端口
    # dt = df[(df.目的端口 == 3389) | (df.srcport == 3389)]
    lg = lgout = lgsuccess = lgfail = m = 0
    conset = False
    lgset = False  # 设立两个表示连接，登录的状态标志位

    for j in range(dt.shape[0]):  # 根据数据包传输层flag标志位，及其出现顺序、间隔的逻辑判断远程登录状态
        if (dt.iloc[j, 9] == 20) & (lgset is False) & (conset is False):
            lg = lg + 1
            lgfail = lgfail + 1
            lgset = True
            m = 0
        if (dt.iloc[j, 9] == 2) & (0 < m < 2) & (lgset is True):
            lgsuccess = lgsuccess + 1
            lgfail = lgfail - 1
            conset = True
            lgset = False

        if (2 <= m) & (lgset is True):
            # lgfail = lgfail + 1
            lgset = False

        if (dt.iloc[j, 9] == 20) & (conset is True):
            lgout = lgout + 1
            conset = False
        m = m + 1

    # print(lg, lgsuccess, lgfail, lgout)

    # if lg != 0:
    mycursor = mydb.cursor()
    # table = "table3"
    # sql = "DROP TABLE IF EXISTS " + table  # 删除数据表
    # mycursor.execute(sql)
    # mycursor.execute(  # 创建数据表
    #     "CREATE TABLE " + table + " (login VARCHAR(255),
    #     loginsuccess VARCHAR(255),loginfail VARCHAR(255),loginout VARCHAR(255))")
    sql = "INSERT INTO 远程登录记录表 (日期,时间,登录,登录成功次数,登录失败次数,登出次数,来源) VALUES (%s,%s,%s,%s,%s,%s,%s)"

    val = (str(datee), str(timee), lg, lgsuccess, lgfail, lgout, str(file))

    mycursor.execute(sql, val)
    mydb.commit()  # 写入数据库


def insert_time(IP, file, mydb, startid, sum):
    # df = pd.read_csv(data)  # 读取解析结果
    mycursor = mydb.cursor()

    mycursor.execute("SELECT * FROM 解析表 LIMIT " + str(startid - 1) + "," + str(sum) + "; ")
    last = mycursor.fetchall()
    result1 = ''.join(str(last))
    # print(result1)
    # result1=result1.split()
    result1 = result1.replace('[', '').replace(']', '').replace('(', '').replace('),', '\n').replace("'", '').replace(
        ')', '').replace(' ', '')
    result1 = "日期,时间,源MAC,目的MAC,源IP,目的IP,协议类型,源端口,目的端口,tcp标志位,包长度,源设备号,源设备端口,目的设备号,目的设备端口,来源,id\n" + result1
    # print(result1)
    f = StringIO(result1)
    df = pd.read_csv(f)
    datee = df.iloc[0, 0]
    timee = df.iloc[0, 1]
    dt = df[(df.协议类型 != "ARP") & (df.源IP == IP) | (df.目的IP == IP)]
    dt = dt.reset_index(drop=True)

    for i in range(df.shape[0]):
        # print(df["时间"][i])
        if (datetime.datetime.strptime('13:53:18', '%H:%M:%S')
                < datetime.datetime.strptime(df["时间"][i], '%H:%M:%S') <
                datetime.datetime.strptime('13:53:18', '%H:%M:%S')):
            dt = dt.drop(index=i)

    dt = dt[["源IP", "时间", "目的MAC", "协议类型", "目的端口", "包长度"]].reset_index(drop=True)
    for i in range(dt.shape[0]):
        # dt.iloc[i,0]=int(str(dt.iloc[i,0]).replace(":",""))
        t = datetime.datetime.strptime(dt.iloc[i, 1], '%H:%M:%S')
        dt.iloc[i, 1] = (t - datetime.datetime(1900, 1, 1)).total_seconds()

    dtmin = dt.groupby("源IP").agg({'时间': 'min'})
    dtmax = dt.groupby("源IP").agg({'时间': 'max'})

    mycursor = mydb.cursor()

    sql = "INSERT INTO 非法操作记录表 (日期,时间,IP,起始时间,终止时间,来源) VALUES (%s,%s,%s,%s,%s,%s)"

    for i in range(dtmin.shape[0]):
        a = dtmin.index[i]
        m, s = divmod(dtmin.iloc[i, 0], 60)
        h, m = divmod(m, 60)
        b = ("%02d:%02d:%02d" % (h, m, s))
        m, s = divmod(dtmax.iloc[i, 0], 60)
        h, m = divmod(m, 60)
        c = ("%02d:%02d:%02d" % (h, m, s))
        val = (str(datee), str(timee), str(a), str(b), str(c), str(file))

        mycursor.execute(sql, val)

        if i == 0:
            x = mycursor.lastrowid

        mydb.commit()  # 写入数据库

    return x, i
