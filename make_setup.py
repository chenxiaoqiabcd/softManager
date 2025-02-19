# encoding: utf-8

import os
import shutil

if __name__ == "__main__":
    os.system("update_softManager_skin.bat")
    os.system("update_downladTool_skin.bat")
    os.system("build.bat")

    if not os.path.exists("install_package"):
        os.mkdir("install_package")

    shutil.copy("bin\\Release\\softManager.exe", "install_package\\softManager.exe")
