# encoding: utf-8

import os
import shutil
import paramiko
import requests

#############################################################################################################################################    
def insertCommitVersion():
    os.system("git log --pretty=format:\"%s\" -1 > 1.txt")

    commit_log = open("1.txt", "r", encoding='utf-8').read()

    url = "https://39.100.95.114:5443/insertCommitVersion"

    request_body = {'version': "1.0.1." + last_version, 'description': commit_log}

    requests.post(url, json = request_body, verify=False)

    os.remove("1.txt")

#############################################################################################################################################    
def connect():
    path = os.path.expandvars('%HOMEDRIVE%' + '%HOMEPATH%' + '\\.ssh\\id_rsa')
    private = paramiko.RSAKey.from_private_key_file(path)

    ssh = paramiko.SSHClient()
    ssh.set_missing_host_key_policy(paramiko.AutoAddPolicy())

    try:
        ssh.connect('39.100.95.114', 22, 'root', pkey=private)
    except paramiko.SSHException as e:
        print('SSH negotiation failed: %s' % e)

    return ssh

if __name__ == "__main__":
    last_version = os.getenv("GITHUB_RUN_NUMBER", "0")

    os.system("update_softManager_skin.bat")
    os.system("update_downladTool_skin.bat")
    os.system("build.bat")

    if not os.path.exists("install_package"):
        os.mkdir("install_package")

    shutil.copy("bin\\Release\\softManager.exe", "install_package\\softManager.exe")

    # 上传文件
    ssh = connect()

    sftp = paramiko.SFTPClient.from_transport(ssh.get_transport())

    sftp.put("./install_package/softManager.exe", "/root/soft_ware_manager/softManager.exe")

    ssh.close()

    # 更新版本号
    insertCommitVersion()