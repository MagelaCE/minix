echo off
if exist atol.obj goto atol
echo    compiling: atol
cc1 -di8088 atol >atol.lst
if errorlevel 1 goto atol
cc2 atol >>atol.lst
if errorlevel 1 goto atol
cc3 atol>>atol.lst
if errorlevel 1 goto atol
cc4 atol>>atol.lst
:atol
if exist ctype.obj goto ctype
echo    compiling: ctype
cc1 -di8088 ctype >ctype.lst
if errorlevel 1 goto ctype
cc2 ctype >>ctype.lst
if errorlevel 1 goto ctype
cc3 ctype>>ctype.lst
if errorlevel 1 goto ctype
cc4 ctype>>ctype.lst
:ctype
if exist close.obj goto close
echo    compiling: close
cc1 -di8088 close >close.lst
if errorlevel 1 goto close
cc2 close >>close.lst
if errorlevel 1 goto close
cc3 close>>close.lst
if errorlevel 1 goto close
cc4 close>>close.lst
:close
if exist creat.obj goto creat
echo    compiling: creat
cc1 -di8088 creat >creat.lst
if errorlevel 1 goto creat
cc2 creat >>creat.lst
if errorlevel 1 goto creat
cc3 creat>>creat.lst
if errorlevel 1 goto creat
cc4 creat>>creat.lst
:creat
if exist exec.obj goto exec
echo    compiling: exec
cc1 -di8088 exec >exec.lst
if errorlevel 1 goto exec
cc2 exec >>exec.lst
if errorlevel 1 goto exec
cc3 exec>>exec.lst
if errorlevel 1 goto exec
cc4 exec>>exec.lst
:exec
if exist exit.obj goto exit
echo    compiling: exit
cc1 -di8088 exit >exit.lst
if errorlevel 1 goto exit
cc2 exit >>exit.lst
if errorlevel 1 goto exit
cc3 exit>>exit.lst
if errorlevel 1 goto exit
cc4 exit>>exit.lst
:exit
if exist fork.obj goto fork
echo    compiling: fork
cc1 -di8088 fork >fork.lst
if errorlevel 1 goto fork
cc2 fork >>fork.lst
if errorlevel 1 goto exit
cc3 fork>>fork.lst
if errorlevel 1 goto fork
cc4 fork>>fork.lst
:fork
if exist fstat.obj goto fstat
echo    compiling: fstat
cc1 -di8088 fstat >fstat.lst
if errorlevel 1 goto fstat
cc2 fstat >>fstat.lst
if errorlevel 1 goto fstat
cc3 fstat>>fstat.lst
if errorlevel 1 goto fstat
cc4 fstat>>fstat.lst
:fstat
if exist open.obj goto open
echo    compiling: open
cc1 -di8088 open >open.lst
if errorlevel 1 goto open
cc2 open >>open.lst
if errorlevel 1 goto open
cc3 open>>open.lst
if errorlevel 1 goto open
cc4 open>>open.lst
:open
if exist read.obj goto read
echo    compiling: read
cc1 -di8088 read >read.lst
if errorlevel 1 goto read
cc2 read >>read.lst
if errorlevel 1 goto read
cc3 read>>read.lst
if errorlevel 1 goto read
cc4 read>>read.lst
:read
if exist signal.obj goto signal
echo    compiling: signal
cc1 -di8088 signal >signal.lst
if errorlevel 1 goto signal
cc2 signal >>signal.lst
if errorlevel 1 goto signal
cc3 signal>>signal.lst
if errorlevel 1 goto signal
cc4 signal>>signal.lst
:signal
if exist strcmp.obj goto strcmp
echo    compiling: strcmp
cc1 -di8088 strcmp >strcmp.lst
if errorlevel 1 goto strcmp
cc2 strcmp >>strcmp.lst
if errorlevel 1 goto strcmp
cc3 strcmp>>strcmp.lst
if errorlevel 1 goto strcmp
cc4 strcmp>>strcmp.lst
:strcmp
if exist sync.obj goto sync
echo    compiling: sync
cc1 -di8088 sync >sync.lst
if errorlevel 1 goto sync
cc2 sync >>sync.lst
if errorlevel 1 goto sync
cc3 sync>>sync.lst
if errorlevel 1 goto sync
cc4 sync>>sync.lst
:sync
if exist wait.obj goto wait
echo    compiling: wait
cc1 -di8088 wait >wait.lst
if errorlevel 1 goto wait
cc2 wait >>wait.lst
if errorlevel 1 goto wait
cc3 wait>>wait.lst
if errorlevel 1 goto wait
cc4 wait>>wait.lst
:wait
if exist write.obj goto write
echo    compiling: write
cc1 -di8088 write >write.lst
if errorlevel 1 goto write
cc2 write >>write.lst
if errorlevel 1 goto write
cc3 write>>write.lst
if errorlevel 1 goto write
cc4 write>>write.lst
:write
if exist syslib.obj goto syslib
echo    compiling: syslib
cc1 -di8088 syslib >syslib.lst
if errorlevel 1 goto syslib
cc2 syslib >>syslib.lst
if errorlevel 1 goto syslib
cc3 syslib>>syslib.lst
if errorlevel 1 goto syslib
cc4 syslib>>syslib.lst
:syslib
if exist call.obj goto call
echo    compiling: call
cc1 -di8088 call >call.lst
if errorlevel 1 goto call
cc2 call >>call.lst
if errorlevel 1 goto call
cc3 call>>call.lst
if errorlevel 1 goto call
cc4 call>>call.lst
:call
if exist message.obj goto message
echo    compiling: message
cc1 -di8088 message >message.lst
if errorlevel 1 goto message
cc2 message >>message.lst
if errorlevel 1 goto message
cc3 message>>call.lst
if errorlevel 1 goto message
cc4 message>>message.lst
:message
if exist printk.obj goto printk
echo    compiling: printk
cc1 -di8088 printk >printk.lst
if errorlevel 1 goto printk
cc2 printk >>printk.lst
if errorlevel 1 goto printk
cc3 printk>>printk.lst
if errorlevel 1 goto printk
cc4 printk>>printk.lst
:printk
if exist catchsig.obj goto catchsig
echo    assembling: catchsig
masm catchsig,,nul,nul >catchsig.lst
:catchsig
if exist getutil.obj goto getutil
echo    assembling: getutil
masm getutil,,nul,nul >getutil.lst
:getutil
if exist sendrec.obj goto sendrec
echo    assembling: sendrec
masm sendrec,,nul,nul >sendrec.lst
:sendrec
if exist ZISWITCH.OBJ goto ZISWITCH
echo    assembling: ZISWITCH
masm ZISWITCH,,nul,nul
:ZISWITCH
if exist ZJSWITCH.OBJ goto ZJSWITCH
echo    assembling: ZJSWITCH
masm ZJSWITCH,,nul,nul
:ZJSWITCH
if exist ZLDIVMOD.OBJ goto ZLDIVMOD
echo    assembling: ZLDIVMOD
masm ZLDIVMOD,,nul,nul
:ZLDIVMOD
if exist ZLLSHIFT.OBJ goto ZLLSHIFT
echo    assembling: ZLLSHIFT
masm ZLLSHIFT,,nul,nul
:ZLLSHIFT
if exist ZLMUL.OBJ goto ZLMUL
echo    assembling: ZLMUL
masm ZLMUL,,nul,nul
:ZLMUL
if exist ZLRSSHFT.OBJ goto ZLRSSHFT
echo    assembling: ZLRSSHFT
masm ZLRSSHFT,,nul,nul
:ZLRSSHFT
if exist ZLRUSHFT.OBJ goto ZLRUSHFT
echo    assembling: ZLRUSHFT
masm ZLRUSHFT,,nul,nul
:ZLRUSHFT
makelib.bat
