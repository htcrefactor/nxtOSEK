echo Executing NeXTTool to upload btslave.rxe...
wineconsole /cygdrive/C/cygwin/nexttool/NeXTTool.exe /COM=usb -download=btslave.rxe
wineconsole /cygdrive/C/cygwin/nexttool/NeXTTool.exe /COM=usb -listfiles=btslave.rxe
echo NeXTTool is terminated.
