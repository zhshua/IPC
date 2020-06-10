import re
import pandas as pd
from io import StringIO


class conf_read():
    f1 = open('C:\\Users\\ZS\\Desktop\\解析的\\配置.txt', encoding='utf8')

    data1 = f1.readlines()
    f1.close()
    results = []
    # get_ip=''
    for line in data1:
        ip = re.findall("IPCip=", line)
        host = re.findall("host=", line)
        user = re.findall("user=", line)
        passwd = re.findall("password=", line)
        database = re.findall("database=", line)
        file_path = re.findall("file_path=", line)
        save_path = re.findall("save_path=", line)
        flow = re.findall("min_flow=", line)
        speed = re.findall("min_speed=", line)
        tcp = re.findall("min_tcp=", line)
        login = re.findall("min_login=", line)
        fail = re.findall("min_fail=", line)
        ports = re.findall("min_ports=", line)
        start_time = re.findall("start_time=", line)
        end_time = re.findall("end_time=", line)
        # fpcap = re.findall("fpcap=", line)
        # data = re.findall("data=", line)
        # svmdata = re.findall("svmdata=", line)
        # svmtest = re.findall("svmtest=", line)

        if ip:
            _ip = line.replace('IPCip=', '').replace('\n', '')
        if host:
            _host = line.replace('host=', '').replace('\n', '')
        if user:
            _user = line.replace('user=', '').replace('\n', '')
        if passwd:
            _passwd = line.replace('password=', '').replace('\n', '')
        if database:
            _database = line.replace('database=', '').replace('\n', '')
        if file_path:
            _file_path = line.replace('file_path=', '').replace('\n', '')
        if save_path:
            _save_path = line.replace('save_path=', '').replace('\n', '')
        if flow:
            _flow = line.replace('min_flow=', '').replace('\n', '')
        if speed:
            _speed = line.replace('min_speed=', '').replace('\n', '')
        if tcp:
            _tcp = line.replace('min_tcp=', '').replace('\n', '')
        if login:
            _login = line.replace('min_login=', '').replace('\n', '')
        if fail:
            _fail = line.replace('min_fail=', '').replace('\n', '')
        if ports:
            _ports = line.replace('min_ports=', '').replace('\n', '')
        if start_time:
            _start_time = line.replace('start_time=', '').replace('\n', '')
        if end_time:
            _end_time = line.replace('end_time=', '').replace('\n', '')
        # if fpcap:
        #     _fpcap = line.replace('fpcap=', '').replace('\n','')
        # if data:
        #     _data = line.replace('data=', '').replace('\n','')
        # if svmdata:
        #     _svmdata = line.replace('svmdata=', '').replace('\n','')
        # if svmtest:
        #     _svmtest = line.replace('svmtest=', '').replace('\n','')

    # f1 = open('配置.txt', encoding='utf8')
    # keyStart = '--------------------白名单'
    # keyEnd = '--------------------结束'
    # buff = f1.read()
    # pat = re.compile(keyStart + '(.*?)' + keyEnd, re.S)
    # result1 = pat.findall(buff)
    # result1 = ''.join(result1)
    # f = StringIO(result1)
    # f = pd.read_csv(f)
    # f1.close()

    def get_ip(self):

        return self._ip

    def get_host(self):
        return self._host

    def get_user(self):
        return self._user

    def get_passwd(self):
        return self._passwd

    def get_database(self):
        return self._database

    def get_file_path(self):
        return self._file_path

    def get_save_path(self):
        return self._save_path

    def get_flow(self):
        return self._flow

    def get_speed(self):
        return self._speed

    def get_tcp(self):
        return self._tcp

    def get_login(self):
        return self._login

    def get_fail(self):
        return self._fail

    def get_ports(self):
        return self._ports

    def get_start_time(self):
        return self._start_time

    def get_end_time(self):
        return self._end_time

    # def get_whi(self):
    #     return self.f

    # def get_fpcap(self):
    #     return self._fpcap
    #
    # def get_data(self):
    #     return self._data
    #
    # def get_svmdata(self):
    #     return self._svmdata
    #
    # def get_svmtest(self):
    #     return self._svmtest

# if __name__ == "__main__":
#     test = conf_read()
#
#     print(test.get_svmtest())
#
