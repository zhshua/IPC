from sklearn import svm
import numpy as np
from sklearn.model_selection import train_test_split
from sklearn.preprocessing import MinMaxScaler
import joblib


def svm0():
    np.set_printoptions(threshold=np.inf)

    data1 = np.loadtxt('svmdata.txt', dtype=int, delimiter=',')  # 读取样本集

    data1, z = np.split(data1, (8,), axis=1)  # 样本集数据与标签分离

    scaler = MinMaxScaler()  # 数据归一化
    scaler.fit(data1)
    scaler.data_max_
    data1 = scaler.transform(data1)

    data1 = np.hstack((data1, z))  # 合并归一化后的data1与标签

    x, y = np.split(data1, (8,), axis=1)  # 分离data1的数据与标签
    x_train, x_test, y_train, y_test = train_test_split(x, y, random_state=4, train_size=0.9)  # 分离训练集与样本集
    clf = svm.SVC(C=0.9, kernel='rbf', gamma=20, decision_function_shape='ovr')  # 创建分类器对象
    clf.fit(x_train, y_train.astype('int').ravel())  # 拟合分类器模型
    joblib.dump(clf, "train1_model.m")  # 保存模型

    print("模型训练成功，保存至train1_model.m")
