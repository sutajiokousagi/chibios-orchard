#! /bin/sh
### BEGIN INIT INFO
# Provides:          orchard
# Required-Start:    $all
# Required-Stop:
# Default-Start:     2 3 4 5
# Default-Stop:
# Short-Description: Run Orchrad factory test
### END INIT INFO


PATH=/usr/local/sbin:/usr/local/bin:/usr/sbin:/usr/bin:/sbin:/bin

. /lib/init/vars.sh
. /lib/lsb/init-functions

do_start() {
	cd /home/pi/orchard-src/orchard/
	(./build/factory-test --swclk 20 --swdio 21 --elf /home/pi/orchard-src/orchard/build/orchard.elf --serial /dev/ttyAMA0 --red 6 --yellow 13 --green 19 --button 26 -vvvv > /tmp/orchard.log 2>&1 &)
	return 0
}

case "$1" in
    start)
	do_start
        ;;
    restart|reload|force-reload)
        echo "Error: argument '$1' not supported" >&2
        exit 3
        ;;
    stop)
        ;;
    *)
        echo "Usage: $0 start|stop" >&2
        exit 3
        ;;
esac
