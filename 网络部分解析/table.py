import mysql.connector
from conf import conf_read


class table:

    def login(self, mydb):
        self.mycursor = mydb.cursor()

    def create_jx(self):
        self.mycursor.execute(
            "CREATE TABLE 解析表 ("
            "日期 VARCHAR(255), 时间 VARCHAR(255), 源MAC VARCHAR(255), "
            "目的MAC VARCHAR(255), 源IP VARCHAR(255),"
            " 目的IP VARCHAR(255), 协议类型 VARCHAR(255), "
            "源端口 VARCHAR(255),目的端口 VARCHAR(255),"
            "tcp标志位 VARCHAR(255), 包长度 VARCHAR(255),"
            "源设备号 VARCHAR(255),源设备端口 VARCHAR(255),目的设备号 VARCHAR(255),目的设备端口 VARCHAR(255),来源 VARCHAR(255),"
            "id INT AUTO_INCREMENT PRIMARY KEY)")

    def create_fx(self):
        self.mycursor.execute(
            "CREATE TABLE 分析表 ("
            "流量 VARCHAR(255), 流量速度 VARCHAR(255), 登录次数 VARCHAR(255), "
            "登录失败次数 VARCHAR(255),端口遍历数  VARCHAR(255),"
            " tcp连接数 VARCHAR(255), 起始时间 VARCHAR(255), 终止时间 VARCHAR(255),"
            "合法数据包比例 VARCHAR(255),预测攻击 VARCHAR(255),得分 VARCHAR(255),"
            "状态 VARCHAR(255),来源 VARCHAR(255),id INT AUTO_INCREMENT PRIMARY KEY)")

    def create_dos(self):
        self.mycursor.execute(
            "CREATE TABLE 流量时序表 (日期 VARCHAR(255),时间 VARCHAR(255),流量 VARCHAR(255),来源 VARCHAR(255))")

    def create_port(self):
        self.mycursor.execute(
            "CREATE TABLE 端口遍历表 (日期 VARCHAR(255),时间 VARCHAR(255),IP VARCHAR(255),遍历端口数 VARCHAR(255),来源 VARCHAR(255))")

    def create_login(self):
        self.mycursor.execute(
            "CREATE TABLE 远程登录记录表 (日期 VARCHAR(255),时间 VARCHAR(255),登录 VARCHAR(255),"
            "登录成功次数 VARCHAR(255),登录失败次数 VARCHAR(255),登出次数 VARCHAR(255),来源 VARCHAR(255))")

    def create_time(self):
        self.mycursor.execute(
            "CREATE TABLE 非法操作记录表 (日期 VARCHAR(255),时间 VARCHAR(255),"
            "IP VARCHAR(255),起始时间 VARCHAR(255),终止时间 VARCHAR(255),来源 VARCHAR(255),"
            "id INT AUTO_INCREMENT PRIMARY KEY)")

    def create_white(self):
        self.mycursor.execute(
            "CREATE TABLE 白名单 (IP VARCHAR(255),MAC地址 VARCHAR(255),端口 VARCHAR(255),设备号 VARCHAR(255),设备端口 VARCHAR(255),id INT AUTO_INCREMENT PRIMARY KEY)")

    def drop_jx(self):
        sql = "DROP TABLE IF EXISTS 解析表"
        self.mycursor.execute(sql)

    def drop_fx(self):
        sql = "DROP TABLE IF EXISTS 分析表"
        self.mycursor.execute(sql)

    def drop_dos(self):
        sql = "DROP TABLE IF EXISTS 流量时序表"
        self.mycursor.execute(sql)

    def drop_port(self):
        sql = "DROP TABLE IF EXISTS 端口遍历表"
        self.mycursor.execute(sql)

    def drop_login(self):
        sql = "DROP TABLE IF EXISTS 远程登录记录表"
        self.mycursor.execute(sql)

    def drop_time(self):
        sql = "DROP TABLE IF EXISTS 非法操作记录表"
        self.mycursor.execute(sql)

    def drop_white(self):
        sql = "DROP TABLE IF EXISTS 白名单"
        self.mycursor.execute(sql)


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
