import dpkt
import binascii
import time
import datetime
import mysql.connector


def addr2str(addrobj):  # 处理IP地址字段，返回为192.168.1.1的形式
    if len(addrobj) != 4:
        return "addr error!"
    else:
        return str(addrobj[0]) + "." + str(addrobj[1]) + "." + str(addrobj[2]) + "." + str(addrobj[3])


def addr3str(addrobj):  # 处理设备号字段，返回为192.168.1.1.1.1的形式
    if len(addrobj) != 6:
        return "addr error!"
    else:
        return str(addrobj[0]) + "." + str(addrobj[1]) + "." + str(addrobj[2]) + "." + str(addrobj[3]) + "." + str(
            addrobj[4]) + "." + str(addrobj[5])


def TCPorUDP(obj):  # 根据标志位判断传输层协议类型
    if obj == 0x01:
        return "ICMP"
    elif obj == 0x02:
        return "IGMP"
    elif obj == 0x06:
        return "TCP"
    elif obj == 0x08:
        return "EGP"
    elif obj == 0x09:
        return "IGP"
    elif obj == 0x11:
        return "UDP"
    elif obj == 0x8a:
        return "ARP"
    elif obj == 41:
        return "IPv6"
    elif obj == 89:
        return "OSPF"
    else:
        return "error"


def jx(fpcap, mydb):  # 待解析的pcap文件，解析结果保存路径，数据库登录信息
    # fvis = open(vis, "w")
    f = open(fpcap, "rb")
    pcap = dpkt.pcap.Reader(f)
    total = 0
    a = 0
    b = 0

    mycursor = mydb.cursor()

    for ts, buf in pcap:  # 循环解析
        # eth = dpkt.ethernet.Ethernet(buf)
        if buf[12] != 0x08 or buf[13] != 0x00:  # 如果不是ipv4数据包则跳过
            continue
        # print(buf[12]!=0x0800)
        # print(eth.data.data.data)
        leng = len(buf)  # 数据包长度
        total = total + leng  # 统计长度
        pktheader = buf[14:34]  # 提取网络层报头
        dstip = pktheader[16:20]  # 提取目的IP地址
        srcip = pktheader[12:16]  # 提取源IP地址

        trans_type = pktheader[9]  # 提取传输层协议类型

        timeArray = time.localtime(ts)  # 处理时间戳
        timeArray1 = time.strftime("%H:%M:%S", timeArray)
        timeArray3 = datetime.datetime.strptime(timeArray1, '%H:%M:%S')
        timeArray4 = (timeArray3).strftime('%H:%M:%S')
        # fvis.writelines(str(time.strftime("%Y-%m-%d,", timeArray)) + timeArray4)  # 保存处理后的时间

        ethheader = buf[0:14]  # 提取链路层报头
        dstmac = ethheader[0:6]  # 目的MAC地址
        srcmac = ethheader[6:12]  # 源MAC地址

        # fvis.writelines("," + str(binascii.b2a_hex(srcmac)).replace("b'", "").replace("'", "") + "," + str(
        #     binascii.b2a_hex(dstmac)).replace("b'", "").replace("'", ""))  # 保存处理后的MAC地址

        # fvis.writelines("," + str((addr2str(srcip))) + "," + str((addr2str(dstip))))  # 保存处理后的IP地址

        if trans_type == 0x11:  # UDP，处理udp端口，并保存文本及导入数据库
            udpheader = buf[34:42]
            srcport = udpheader[0:2]
            dstport = udpheader[2:4]
            udplen = udpheader[4:6]
            # fvis.writelines("," + TCPorUDP(trans_type) + "," + str(srcport[1] + srcport[0] * 16 * 16) + "," + str(
            #     dstport[1] + dstport[0] * 16 * 16) + ",NULL" + str(leng) + ",NULL" + ",NULL" + ",NULL" + ",NULL" +
            #                 "\n")
            sql = "INSERT INTO 解析表 (日期,时间,源MAC,目的MAC,源IP,目的IP,协议类型,源端口,目的端口,tcp标志位,包长度," \
                  "源设备号,源设备端口,目的设备号,目的设备端口,来源)" \
                  " VALUES (%s,%s,%s,%s,%s,%s,%s,%s,%s,%s, %s,%s,%s,%s,%s,%s)"
            val = [
                (str(time.strftime("%Y-%m-%d", timeArray)), timeArray4,
                 str(binascii.b2a_hex(srcmac)).replace("b'", "").replace("'", ""), str(
                    binascii.b2a_hex(dstmac)).replace("b'", "").replace("'", ""), str((addr2str(srcip))),
                 str((addr2str(dstip))),
                 TCPorUDP(trans_type), str(srcport[1] + srcport[0] * 16 * 16), str(
                    dstport[1] + dstport[0] * 16 * 16), "NULL", str(leng), "NULL", "NULL", "NULL", "NULL", fpcap)
            ]
        elif trans_type == 0x06:  # TCP，处理tcp端口和tcp标志位，并保存文本及导入数据库
            flag = buf[47]
            tcpheader = buf[34:54]
            srcport0 = tcpheader[0:2]
            srcport = str(srcport0[1] + srcport0[0] * 16 * 16)
            dstport0 = tcpheader[2:4]
            dstport = str(
                dstport0[1] + dstport0[0] * 16 * 16)
            if leng > 60 and ((srcport == "48898") | (dstport == "48898")):  # 解析应用层协议类型，48898是wireshark对AMS协议采用的判断端口
                type = "AMS"
                srceqno = addr3str(buf[60:66])
                srceqpo = str(buf[66] + buf[67] * 16 * 16)
                dsteqno = addr3str(buf[68:74])
                dsteqpo = str(buf[74] + buf[75] * 16 * 16)
                # fvis.writelines("," + type + "," + srcport + "," + dstport + "," + str(
                #     flag) + "," + str(leng) + "," + str(srceqno) + "," + str(srceqpo) + "," +
                #     str(dsteqno) + "," + str(
                #     dsteqpo) + "\n")
                sql = "INSERT INTO 解析表 (日期,时间,源MAC,目的MAC,源IP,目的IP,协议类型,源端口,目的端口,tcp标志位,包长度," \
                      "源设备号,源设备端口,目的设备号,目的设备端口,来源)" \
                      " VALUES (%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s, %s, %s)"
                val = [
                    (str(time.strftime("%Y-%m-%d", timeArray)), timeArray4,
                     str(binascii.b2a_hex(srcmac)).replace("b'", "").replace("'", ""), str(
                        binascii.b2a_hex(dstmac)).replace("b'", "").replace("'", ""), str((addr2str(srcip))),
                     str((addr2str(dstip))),
                     type, srcport, dstport, str(
                        flag), str(leng), str(srceqno), str(srceqpo), str(dsteqno), str(dsteqpo), fpcap)
                ]
            else:
                type = "TCP"
                # fvis.writelines("," + type + "," + srcport + "," + dstport + "," + str(
                #     flag) + "," + str(leng) + ",NULL" + ",NULL" + ",NULL" + ",NULL" + "\n")
                sql = "INSERT INTO 解析表 (日期,时间,源MAC,目的MAC,源IP,目的IP,协议类型,源端口,目的端口,tcp标志位,包长度," \
                      "源设备号,源设备端口,目的设备号,目的设备端口,来源)" \
                      " VALUES (%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s, %s, %s)"
                val = [
                    (str(time.strftime("%Y-%m-%d", timeArray)), timeArray4,
                     str(binascii.b2a_hex(srcmac)).replace("b'", "").replace("'", ""), str(
                        binascii.b2a_hex(dstmac)).replace("b'", "").replace("'", ""), str((addr2str(srcip))),
                     str((addr2str(dstip))),
                     type, srcport, dstport, str(
                        flag), str(leng), "NULL", "NULL", "NULL", "NULL", fpcap)
                ]
        else:  # 其他协议，没有解析端口和传输层标志位
            # fvis.writelines("," + TCPorUDP(trans_type) + ",NULL,NULL,NULL," + str(
            #     leng) + ",NULL" + ",NULL" + ",NULL" + ",NULL" + "\n")
            sql = "INSERT INTO 解析表  (日期,时间,源MAC,目的MAC,源IP,目的IP,协议类型,源端口,目的端口,tcp标志位,包长度," \
                  "源设备号,源设备端口,目的设备号,目的设备端口,来源)" \
                  " VALUES (%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s, %s, %s)"
            val = [
                (str(time.strftime("%Y-%m-%d", timeArray)), timeArray4,
                 str(binascii.b2a_hex(srcmac)).replace("b'", "").replace("'", ""), str(
                    binascii.b2a_hex(dstmac)).replace("b'", "").replace("'", ""), str((addr2str(srcip))),
                 str((addr2str(dstip))),
                 TCPorUDP(trans_type), "NULL", "NULL", "NULL", str(leng), "NULL", "NULL", "NULL", "NULL", fpcap)
            ]
        # id=id+1
        # if a == 1:
        #     print(mycursor.lastrowid)
        #     b = mycursor.lastrowid
        #     a = a + 1
        mycursor.executemany(sql, val)
        if a == 0:
            # print(mycursor.lastrowid)
            b = mycursor.lastrowid
        a = a + 1
        mydb.commit()  # 更新数据库
    # a=a+b-1
    f.close()
    # fvis.close()  # 释放内存
    # print("数据包解析完成,结果已保存至数据库"+table+"表以及当前文件夹vis.txt中")
    return b, a
