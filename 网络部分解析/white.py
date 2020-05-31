import pandas as pd
from conf import conf_read
import mysql.connector
from table import table


def whi(data, mydb):
    # conf = conf_read()
    # df = conf.get_whi()

    sqlcom = 'select * from 白名单'
    df = pd.read_sql(sqlcom, con=mydb)

    dx = data
    for i in range(data.shape[0]):
        for j in range(df.shape[0]):

            if (df.iloc[j, 0] == data.iloc[i, 4]) | (df.iloc[j, 0] == "*"):  # 验证源IP
                if (df.iloc[j, 1] == data.iloc[i, 2]) | (df.iloc[j, 1] == "*"):  # 验证源MAC
                    if (df.iloc[j, 2] == data.iloc[i, 7]) | (df.iloc[j, 2] == "*"):  # 验证源端口
                        if (str(df.iloc[j, 3]) == str(data.iloc[i, 11])) | (df.iloc[j, 3] == "*") | (
                                'NULL' == str(data.iloc[i, 11])):  # 验证源设备号
                            if (str(df.iloc[j, 4]) == str(data.iloc[i, 12])) | (df.iloc[j, 4] == "*") | (
                                    'NULL' == str(data.iloc[i, 11])):  # 验证源设备端口
                                dx = dx.drop([i])
                                continue
    return dx


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
    print("数据库登录成功")

    ta = table()  # 实例化表类
    ta.login(mydb)  # 登录

    # ta.drop_white()
    # ta.create_white()

    sqlcom = 'select * from 白名单'
    df = pd.read_sql(sqlcom, con=mydb)

    conf = conf_read()
    df1 = conf.get_whi()

    # print(df.iloc[0,0],df.iloc[0,1],df.iloc[0,2],str(df.iloc[0,3]),str(df.iloc[0,4]))
    # print('---')
    # print(df1.iloc[0,0],df1.iloc[0,1],df1.iloc[0,2],str(df1.iloc[0,3]),str(df1.iloc[0,4]))
    # whi(dt)
