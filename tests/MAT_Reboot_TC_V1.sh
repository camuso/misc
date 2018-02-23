#! /bin/sh  
#This script is used to automate the rebootscript test case.

# Create a Count value under /MAT/
if [ ! -d "/MAT" ]; then
  mkdir "/MAT/"
fi

MATPATH="/MAT"
TESTCASE=$1

case "$TESTCASE" in
     'reboottest') echo "#### System Reboot test " $2 " #### " >> $MATPATH/reboot.log; 
                   exit 0;;
     'systemdatetest') echo "#### System Date test " $2 " ####" >> $MATPATH/reboot.log; 
                   exit 0;;
esac

touch $MATPATH/count
echo 0 > $MATPATH/count

# Create a rebootscript.sh file under /etc/ and copy following contents

touch /$MATPATH/rebootscript.sh
echo '#! /bin/sh ' > $MATPATH/rebootscript.sh
echo 'count=`cat '$MATPATH'/count`' >> $MATPATH/rebootscript.sh
echo 'count=$(expr $count + 1)' >> $MATPATH/rebootscript.sh
echo 'echo $count > '$MATPATH'/count' >> $MATPATH/rebootscript.sh

echo 'if [ $count == 1 ]' >> $MATPATH/rebootscript.sh
echo 'then' >> $MATPATH/rebootscript.sh 
echo '  date > '$MATPATH'/date.txt' >> $MATPATH/rebootscript.sh
echo '  init 6' >> $MATPATH/rebootscript.sh
echo 'fi' >> $MATPATH/rebootscript.sh

echo 'sleep 60' >> $MATPATH/rebootscript.sh
echo 'CUR=`date | cut -c1-13`' >> $MATPATH/rebootscript.sh
echo 'PRV=`cat '$MATPATH'/date.txt | cut -c1-13`' >> $MATPATH/rebootscript.sh
echo 'case "$CUR" in' >> $MATPATH/rebootscript.sh
echo '  $PRV ) sh '$MATPATH'/MAT_Reboot_TC_V1.sh "systemdatetest" "PASSED";; ' >> $MATPATH/rebootscript.sh
echo '  *) sh '$MATPATH'/MAT_Reboot_TC_V1.sh "systemdatetest" "FAILED";; ' >> $MATPATH/rebootscript.sh
echo 'esac' >> $MATPATH/rebootscript.sh

#save date and time for next reboot
echo 'date >  '$MATPATH'/date.txt' >> $MATPATH/rebootscript.sh
echo 'if [ $count == 960 ]' >> $MATPATH/rebootscript.sh
echo 'then' >> $MATPATH/rebootscript.sh 
echo '  cp '$MATPATH'/rc.orignal /etc/rc.d/rc' >> $MATPATH/rebootscript.sh
echo '  sh '$MATPATH'/MAT_Reboot_TC_V1.sh "reboottest" "PASSED"' >> $MATPATH/rebootscript.sh
echo 'fi' >> $MATPATH/rebootscript.sh
echo 'init 6' >> $MATPATH/rebootscript.sh
chmod 777 $MATPATH/rebootscript.sh

# Append "$MATPATH/rebootscript.sh &" in /etc/rc.d/rc
file=/etc/rc.d/rc
lns=`cat $file | wc -l`
#echo $lns
#x=$MATPATH/rebootscript.sh
y=`expr $lns - 1`

#copy the script to /etc/rc.d/rc so that reboot can happen
sed ''$y'i'$MATPATH'/rebootscript.sh &' $file > $MATPATH/rc.txt
cp /etc/rc.d/rc $MATPATH/rc.orignal
cp $MATPATH/rc.txt /etc/rc.d/rc

# initiate the reboot to invoke the rebootscript.sh script file
init 6

exit 0
