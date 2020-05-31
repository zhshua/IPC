from sklearn import svm
import numpy as np
from sklearn.model_selection import train_test_split
from sklearn.preprocessing import MinMaxScaler
import joblib
import pandas as pd
import datetime


def svm(path1, mydb, jx_startid):
    np.set_printoptions(threshold=np.inf)

    data1 = np.loadtxt("svmdata.txt", dtype=int, delimiter=',')  # 读取样本集，这里样本集不做模型训练，用于data2的同步归一化

    sqlcom = 'select * from 分析表 LIMIT ' + str(jx_startid - 1) + ',1; '
    df = pd.read_sql(sqlcom, con=mydb)
    start_s = (datetime.datetime.strptime(df.iloc[0, 6], '%H:%M:%S') - datetime.datetime(1970, 1, 1)).seconds
    end_s = (datetime.datetime.strptime(df.iloc[0, 7], '%H:%M:%S') - datetime.datetime(1970, 1, 1)).seconds
    data2 = np.array(
        [df.iloc[0, 0], df.iloc[0, 1], df.iloc[0, 2], df.iloc[0, 3], df.iloc[0, 4], df.iloc[0, 5], start_s, end_s])

    data1, z = np.split(data1, (8,), axis=1)  # 分离data1的数据和标签
    data1 = np.insert(data1, data1.shape[0], data2, axis=0)  # 插入data2到data1的数据中

    scaler = MinMaxScaler()  # 数据归一化
    scaler.fit(data1)
    scaler.data_max_
    data1 = scaler.transform(data1)
    data2 = data1[(data1.shape[0] - 1), 0:8]  # 提取归一化后的data2
    # data1 = np.delete(data1, data1.shape[0] - 1, axis=0)  # 删除data1中的data2
    # data1 = np.hstack((data1, z))  # 合并data1的数据标签

    clf = joblib.load("train_model.m")  # 读取模型
    data2 = np.array(data2).reshape(1, -1)  # 二维化data2
    label = clf.predict(data2)  # 预测标签

    # result = path1 + '\\' + path2.replace('_fx.txt', '.pcap') + "网络状态存在"
    result = "网络状态存在"
    if 0 == label:
        result = "网络状态安全。"
    if 8 <= label:
        result = result + " dos攻击 "
        label = label - 8
    if 4 <= label < 8:
        result = result + " 远程登录攻击 "
        label = label - 4
    if 2 <= label < 4:
        result = result + " 端口遍历攻击 "
        label = label - 2
    if 1 <= label < 2:
        result = result + " 非法时间操作 "

    mycursor = mydb.cursor()

    sql = "UPDATE 分析表 SET 预测攻击=%s WHERE id=%s"
    val = [str(result), str(jx_startid)]

    mycursor.execute(sql, val)
    mydb.commit()  # 写入数据库

    print(path1 + "使用模型导入完成svm算法预测成功，预测结果保存至分析表中")
    print("预测结果：", result)
