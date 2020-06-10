from svmhandle import remix
from jiexi import jx
from fenxi import *
from SVM import svm
from conf import conf_read
from SVM0 import svm0
import mysql.connector
from table import table
from scores import scores
import os
from natsort import natsorted

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

# ta = table()  # 实例化表类
# ta.login(mydb)  # 登录

# ta.drop_jx()  # 清空
# ta.drop_fx()
# ta.drop_dos()
# ta.drop_port()
# ta.drop_login()
# ta.drop_time()


# ta.create_jx()  # 创建各种表,第一次运行需要运行此项
# ta.create_fx()
# ta.create_dos()
# ta.create_port()
# ta.create_login()
# ta.create_time()

# remix()  # 随机生成训练用的样本集(大量数据)
# svm0()  # 使用样本集训练并保存模型，用时过长

IP = conf.get_ip()  # 读取配置文件中的IPC的ip
file_path = conf.get_file_path()  # 读取分析路径

_a = _b = 0
i = 0
files = ''
for root, dirs, files in os.walk(file_path):
    files = natsorted(files)
    print(files)

for fpcap in files:
    _fpcap = file_path + '\\' + fpcap

    a, b = jx(_fpcap, mydb)  # 解析数据包,并返回数据库起始插入id与插入长度
    _b = _b + b
    if (i == 0) | (_b != 0):
        _a = a
        i = i + 1
    if (_b == 0):
        print(_fpcap + '无可解析的数据包')
    else:
        print(_fpcap + "解析成功")

if (_b == 0):
    print('路径中无文件或文件中无可分析的数据包')
else:
    c = fx(IP, file_path, mydb, _a, _b)  # 分析数据包
    print(file_path + "分析成功")

    insert_dos(file_path, mydb, _a, _b)  # 可视化所用表
    print(file_path + "流量时序表生成成功")

    insert_port(IP, file_path, mydb, _a, _b)  # 可视化所用表
    print(file_path + "端口遍历表生成成功")

    insert_login(IP, file_path, mydb, _a, _b)  # 可视化所用表
    print(file_path + "远程登录表生成成功")

    d, e = insert_time(IP, file_path, mydb, _a, _b)  # 可视化所用表
    if e!=0:
        print(file_path + "非法时间操作记录表生成成功")
    else:
        print(file_path + "无非法时间操作")

    scores(mydb, file_path, c, d, e)  # 可视化所用表
    print(file_path + "更新分析表评分项成功")

    svm(file_path, mydb, c)  # 这个会导入实现训练好的模型，预测结果，这里导入样本集只是帮助分析结果做归一化
