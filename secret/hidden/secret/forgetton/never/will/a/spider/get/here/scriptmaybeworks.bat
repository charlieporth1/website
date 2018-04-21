echo running script 
echo before this ran did you run this command in CMD Command Promt adb devices and accept the computer as a new device on the phone
echo press y for yes n for no 
SET /P AREYOUSURE=Are you sure (Y/[N])?
IF /I "%AREYOUSURE%" NEQ "Y" GOTO END
echo adb 

adb devices
adb shell rm /data/system/gesture.key
echo done if this worked reboot the phone 
