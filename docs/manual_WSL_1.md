--------------------------------------------------------------------------------
# Installing and configuring the Windows Subsystem for Linux (WSL) 
This manual explains how to set up the Windows Subsytem for Linux (WSL), which will enable you to run a Linux operating system on your Windows PC.    

NOTE that this does **not** include installation of LDAS. Once the steps below are completed, you will need to return to the LDAS installation guide and continue with **[Installation on Linux](https://github.com/feathercode/LDAS/blob/master/README.md#installation-on-linux)**.  

## 1. Install WSL and a Linux distribution  
* The official full documentation is **[here](https://docs.microsoft.com/en-us/windows/wsl/install-win10)**. If the  link is broken, there is a copy **[here](https://github.com/feathercode/LDAS/blob/master/docs/manual_WSL_2.md)**.

	* Read the instructions carefully - **don't forget to reboot** when required
	* If you opt for WSL-2, you will need to **enable virtualization** in your computer BIOS 
	* Select **Ubuntu** when installing your Linux distribution from the Microsoft Store

	* This is a very concise summary of the steps for reference: 
```
		1. [Windows Power Shell - as admin]: dism.exe /online /enable-feature /featurename:Microsoft-Windows-Subsystem-Linux /all /norestart
		2. [restart]
		3. [ms-settings]:windowsupdate
		4. [Windows Power Shell - as admin]: dism.exe /online /enable-feature /featurename:VirtualMachinePlatform /all /norestart
		5. [restart]
		6. https://wslstorestorage.blob.core.windows.net/wslblob/wsl_update_x64.msi
		7. [Windows Power Shell - as admin]: wsl --set-default-version 2
		8. https://www.microsoft.com/store/apps/9n6svws3rx71
```


## 2. Install Windows tools for viewing postscript files  
* LDAS produces plots in postscript format, and provides the **xs-plotconvert1** tool to convert plots to jpg or pdf format.
* However, if you want to view postscript plots in Windows, install the free *ghostscript* and *GS-View* tools: 
	* http://www.ghostscript.com/download/gsdnld.html
	* http://www.ghostgum.com.au/software/gsview.htm

## 3. Install a free X-Windows server to handle graphical output 
If you want "live" graphical output from Linux image-viewers, text editors etc. to display in Windows, you can install one of several free X-windows servers: 
	* [VcXsrv](https://sourceforge.net/projects/vcxsrv/),
	* [Xming](https://sourceforge.net/projects/xming/files/latest/download).

VcxSrv is easy to install, and you will just need to run it before launching a WSL Linux session. **NOTE** that under WSL-2, you may need to select the "**Disable access control**" option when starting the server. 

## 4. In Linux, set graphical output to the correct display 
Start Ubuntu and run this command: 
```
sudo /etc/init.d/dbus start &> /dev/null
```

Then set the default display. You will be prompted for your Linux password.  
**Option1: for For WSL-1**...
```
echo -e "\n# SET DISPLAY FOR WSL-1\nexport DISPLAY=localhost:0.0" | sudo tee -a /etc/profile 
```
**Option2: for For WSL-2**...
```
z=$(awk '/nameserver / {print $2":0.0"; exit}' /etc/resolv.conf)
echo -e "\n# SET DISPLAY FOR WSL-2\nexport DISPLAY=$z \nexport LIBGL_ALWAYS_INDIRECT=1" | sudo tee -a /etc/profile
```

### Thats it! 

You should now be able to invoke a Ubuntu terminal window from the Windows Start Menu. If you need to view graphical output as well, make sure you start VcXSrv first, with the access-controls disabled.

