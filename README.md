# clamav-gui
Thank you for downloading and using ClamAV-GUI:
 a tool that simplifies virus scanning on Linux.

 - This application provides a graphical user interface for clamav and
   freshclam with an integrated service menu for Konqueror/Dolphin
   to scan files directly from your file manager.

Requirements:
 - clamav
 - freshclam

  *For Ubuntu: 
  ```bash
  sudo apt install clamav clamav-freshclam
  ```

Installation:
 - If you use Debian based distributions, you can use the available DEB package in the github project
 - If you use RPM based distributions, you can use the available RPM package
 - Alternatively, you can build from source with cmake and cpack

Build Instructions:
 - **For creating AppImage:**
    appimagetool, patchelf and libfuse2 is needed
 - For building with qt6:
   * On Ubuntu: 
    ```bash
    sudo apt install cmake build-essential qt6-base-dev qt6-tools-dev libqt6core5compat6-dev 
    ```
 - Requires qt5 base development files, qtchooser, g++, and make
   *For Ubuntu <= 20.04: 
    ```bash
    sudo apt install qtchooser qt5-default g++ make
    ```
   *For Ubuntu >= 22.04: 
    ```bash
    sudo apt install qtchooser qtbase5-dev g++ make
    ```
 - Extract ClamAV-GUI-*.tar.gz
   ```bash 
   tar -xf ClamAV-GUI-*.tar.gz
   ```

 - Enter the generated folder
   ```bash
   cd ClamAV-GUI-x.x.x
   ```

   *For Ubuntu <= 20.04
    also change ( Qt::endl; --> endl; ) in file "setupfilehandler.cpp"

 - Build the package as root with 'qmake', 'make', and finally 'make install' (deprecated)
   ```bash 
   qmake && make && sudo make install
   ```

 - Build the packages with cmake and cpack (much easier and cleaner)
   ```bash
   mkdir build && cd build
   cmake ..
   make 
   cpack -G DEB (cpack -G RPM, cpack -G AppImage)
  
   ```
   Install the package 
 

Questions / Suggestions:
 - Report any problems or suggestions to
   Joerg Macedo da Costa Zopes <joerg.zopes@gmx.de>

Have fun with your virus-free Linux machine!

**INFO - Changes using clamav 1.0.7 or 1.4.1**

Version 1.0.7 and 1.4.1 of clamav have an issue with "freshclam". Freshclam needs the file "ca-bundle.crt" in the folder "/etc/pki/tls/certs" and neither the folder nor the file exists in Open SuSE.
On Open SuSE I found the file "ca-bundle.pem" in the folder "/etc/ssl" and a softlink to "/etc/pki/tls/certs/ca-bundle.crt" solves the problem.
