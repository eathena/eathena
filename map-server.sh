#/bin/sh
#Hi my naem is Kirt and I liek anime

ulimit -Sc unlimited

while [ 1 ] ; do
if [ -f .stopserver ] ; then
echo server marked down >> servlog.txt
else
echo restarting server at time at `date +"%m-%d %H:%M:%S"`>> startlog.txt
echo last restart at time `date +"%m-%d %H:%M:%S"`> laststart.txt
mv core.* cores
./map-server_sql
fi

sleep 5

done
