echo Executing NeXTTool to upload motortest_OSEK.rxe...
wineconsole /cygdrive/C/cygwin/nexttool/NeXTTool.exe /COM=usb -download=motortest_OSEK.rxe
wineconsole /cygdrive/C/cygwin/nexttool/NeXTTool.exe /COM=usb -listfiles=motortest_OSEK.rxe
echo NeXTTool is terminated.
