#!/bin/bash

PORT=2331
DEVICE_ID=
CFG=cli_programmer.ini
JLINKLOG=jlink.log
TARGET_RESET_CMD=
JLINK_PATH=

while true; do
  case $1 in
    --cfg)
      CFG=$2
      shift
      shift
      ;;
    --id)
      DEVICE_ID=$2
      shift
      shift
      ;;
    --port)
      PORT=$2
      shift
      shift
      ;;
    --log)
      JLINKLOG=$2
      shift
      shift
      ;;
    --trc)
      TARGET_RESET_CMD=$2
      shift
      shift
      ;;
    --jlink_path)
      JLINK_PATH="$2/"
      shift
      shift
      ;;
    *)
      break
      ;;
  esac
done

# For compatibility with old syntax with just device id on command line
if [ -z $DEVICE_ID ] ; then DEVICE_ID=$1; fi

base_dir="`dirname "$0"`"
sdkroot=${SDKROOT:=$(pushd "$base_dir/../../.." >/dev/null; pwd; popd >/dev/null)}
QSCR=$sdkroot/utilities/scripts/qspi
CLI_PROGRAMMER=$sdkroot/binaries/cli_programmer
JLINKGDB=$JLINK_PATH"JLinkGDBServer"
JLINKCOMMANDER=$JLINK_PATH"JLinkExe"

if [ -z $DEVICE_ID ] ; then
  DEVICE_ID=`$JLINKCOMMANDER -CommandFile "$sdkroot/utilities/scripts/qspi/jlink_showemulist.script" | sed -n 's/^.*Serial number: \([0-9]*\),.*$/\1/p' | tail -n 1`
fi

if [ ! -x "$CLI_PROGRAMMER" ] ; then
        echo "cli_programmer not found. Please make sure it is built and installed in $(dirname "$CLI_PROGRAMMER")"
        exit 1
fi

# If local ini file does not exits make cli_programmer do one
if [ ! -r "${CFG}" -o ! -s "${CFG}" ] ; then
  "$CLI_PROGRAMMER" --save "${CFG}" >/dev/null 2>&1
fi

# Set telnet and swo ports
SWOPORT=$((PORT + 1))
TELNETPORT=$((PORT + 2))

# Add device selection to ini file
awk <"${CFG}" >"${CFG}.new" -F= -v id=$DEVICE_ID -v gdb="$JLINKGDB" -v port=$PORT -v swoport=$SWOPORT -v telnetport=$TELNETPORT -v jlinklog=$JLINKLOG -v trc="$TARGET_RESET_CMD" '
   /^target_reset_cmd/ {
    if (length($0) < 20) {
      sub("=.*", "= " trc)
    }
    if (length(id) == 0 && length(trc) > 0) {
      sub("-selectemubysn [0-9]*", "")
    } else if (length(trc) > 0) {
      i = index($0, "-selectemubysn")
      if (i == 0) {
        sub("[ ]*$", " -selectemubysn " id)
      } else {
        sub("[ ]*-selectemubysn [0-9]*", " -selectemubysn " id)
      }
    }
   }
  /^port *=/ { print "port = " port; next }
  /^gdb_server_path/ {
    if (length($2) < 2) {
      sub("=.*", "= " gdb " -if swd -device Cortex-M0 -endian little -speed 4000 -singlerun -log " jlinklog)
    }
    if (length(id) == 0) {
      sub("-select usb=[0-9]*", "")
    } else {
      i = index($0, "-select usb=")
      if (i == 0) {
        sub("[ ]*$", " -select usb=" id)
      } else {
        sub("[ ]*-select usb=[0-9]*", " -select usb=" id)
      }
    }
    i = index($0, "-port ")
    if (i == 0) {
      sub("[ ]*$", " -port " port)
    } else {
      sub("[ ]*-port [0-9]*", " -port " port)
    }
    i = index($0, "-swoport ")
    if (i == 0) {
      sub("[ ]*$", " -swoport " swoport)
    } else {
      sub("[ ]*-swoport [0-9]*", "")
    }
    i = index($0, "-telnetport ")
    if (i == 0) {
      sub("[ ]*$", " -telnetport " telnetport)
    } else {
      sub("[ ]*-telnetport [0-9]*", "")
    }
    i = index($0, "-log ")
    if (i == 0) {
      sub("[ ]*$", " -log " jlinklog)
    } else {
      sub("[ ]*-log [^ ]*", " -log " jlinklog)
    }
  }
  { print }
'
mv "${CFG}.new" "${CFG}"

