#!/bin/sh

# ex.
# CRUISE_LIST="move 180 90;detect 10;move 210 90;follow 10;move 240 90;sleep 10;"
#    move <pan> <tilt> Point the camera.
#    detect <timeout>  If an object is detected, wait.
#    follow <timeout>  If an object is detected, follow it.
#    sleep <timeout>   Sleep.

HACK_INI=/tmp/hack.ini
CRUISE=$(awk -F "=" '/CRUISE *=/ {print $2}' $HACK_INI)
CRUISE_LIST=$(awk -F "=" '/CRUISE_LIST *=/ {gusb(/^ */, "", $2);gsub(/ *; */, ";", $2);print $2}' $HACK_INI)
if [ "$CRUISE" != "on" ] ; then
  exit 0
fi

while : ; do
  IFS=";"
  for str in $CRUISE_LIST ; do
    cmd=${str%% *}
    param=${str#* }
    echo "$cmd :  $param"
    if [ "$cmd" = "move" ] ; then
      echo "$cmd $param" | /usr/bin/nc localhost 4000
    fi
    if [ "$cmd" = "detect" ] ; then
      while : ; do
        IFS=" "
        motion=`echo "waitMotion $param" | /usr/bin/nc localhost 4000`
        if [ "$motion" = "timeout" ] ; then
          break;
        fi
      done
    fi
    if [ "$cmd" = "follow" ] ; then
      while : ; do
        IFS=" "
        motion=`echo "waitMotion $param" | /usr/bin/nc localhost 4000`
        if [ "$motion" = "timeout" ] ; then
          break;
        fi
        set $motion
        if [ "$1" = "detect" ] ; then
          echo "move $6 $7"
          echo "move $6 $7" | /usr/bin/nc localhost 4000
        fi
      done
      IFS=";"
    fi
    if [ "$cmd" = "sleep" ] ; then
      sleep $param
    fi
  done
done
