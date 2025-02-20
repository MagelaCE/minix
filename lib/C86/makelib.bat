echo/
echo    Creating MXC86.LIB
lib mxc86+atol+ctype+close+creat+exec+exit+fork+fstat+open+read+signal+strcmp;
lib mxc86+sync+wait+write+syslib+call+message+printk+catchsig+getutil+sendrec;
lib mxc86+ZISWITCH+ZJSWITCH+ZLDIVMOD+ZLLSHIFT+ZLMUL+ZLRSSHFT+ZLRUSHFT;
echo/
echo LIB done. Check the .LST-files for errors
rem pause 
rem echo on
rem for %%f in (*.lst) do type %%f
