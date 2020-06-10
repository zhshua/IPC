from conf import conf_read
import mysql.connector
import pandas as pd


def scores(mydb, path, fx_startid, time_startid, time_len):
    mycursor = mydb.cursor()
    conf = conf_read()
    sqlcom = 'select * from 分析表 limit ' + str(fx_startid - 1) + ",1"
    df = pd.read_sql(sqlcom, con=mydb)

    if time_len != 0:
        sqlcom1 = 'select * from 非法操作记录表 limit ' + str(time_startid - 1) + "," + str(time_len + 1)
        df1 = pd.read_sql(sqlcom1, con=mydb)
        time = df1.shape[0]
    else:
        time = 0
    flow = int(df.iloc[0, 0])
    speed = int(float(df.iloc[0, 1]))
    tcp = int(df.iloc[0, 2])
    login = int(df.iloc[0, 3])
    fail = int(df.iloc[0, 4])
    ports = int(df.iloc[0, 5])
    if (is_Chinese(df.iloc[0, 8]) == False):
        whi = int(float(df.iloc[0, 8]))
        min_flow = int(conf.get_flow())
        min_speed = int(conf.get_speed())
        min_tcp = int(conf.get_tcp())
        min_login = int(conf.get_login())
        min_fail = int(conf.get_fail())
        min_ports = int(conf.get_ports())

        dos_score = (((min_flow - flow) / (min_flow + flow) + ((min_speed - speed) / (min_speed + speed)) + (
                (min_tcp - tcp) / (min_tcp + tcp)))) / 3 * 100
        ports_score = ((min_ports - ports) / min_ports) * 100
        login_score = ((min_login - login) / min_login + (min_fail - fail) / min_fail) / 2 * 100
        time_score = ((- 1) / (time + 1)) * 100
        white_score = (whi * 2 - 1) * 600
        total_score = int((dos_score + ports_score + login_score + time_score + white_score) / 10)

        if total_score <= -50:
            result = "网络状态糟糕"
        if -50 < total_score <= 0:
            result = "网络状态较差"
        if 0 < total_score <= 50:
            result = "网络状态一般"
        if 50 < total_score <= 100:
            result = "网络状态良好"
    else:
        total_score = 100
        result = '状态良好'

    # print(int(dos_score),int(ports_score),int(login_score),int(time_score),int(white_score),int(total_score))

    # sql = "INSERT INTO 评分表 (得分,状态,来源) VALUES (%s,%s,%s)"
    sql = "UPDATE 分析表 SET 得分=%s,状态=%s WHERE id=%s"
    # print(sql)
    val = (str(total_score), str(result), str(fx_startid))
    mycursor.execute(sql, val)
    mydb.commit()  # 写入数据库


def is_Chinese(word):
    for ch in word:
        if '\u4e00' <= ch <= '\u9fff':
            return True
    return False


if __name__ == "__main__":
    conf = conf_read()

    mydb = mysql.connector.connect(
        host=conf.get_host(),
        user=conf.get_user(),
        passwd=conf.get_passwd(),
        database=conf.get_database(),
        use_unicode=True,
        charset="utf8"
    )  # 登录数据库
    scores(mydb, 2)
