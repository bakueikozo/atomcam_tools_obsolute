#!/bin/sh
LD_LIBRARY_PATH=/tmp/newroot/lib:/tmp/newroot/usr/lib:/lib:/usr/lib /tmp/newroot/lib/ld.so.1 /tmp/newroot/usr/bin/scp $*
