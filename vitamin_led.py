import datetime
from daemonize import Daemonize
import logging
import re
import serial
import signal
import sys
import time
import threading

pid_file = "/tmp/vitamin-led.pid"
log_file = "/tmp/vitamin-led.log"
syslog = "/var/log/system.log"

logger = logging.getLogger(__name__)
ser = None

def ts():
	return datetime.datetime.fromtimestamp(time.time()).strftime('%Y-%m-%d %H:%M:%S')

def serial_write(msg, error_wait=0, log_error=False):
	global ser
	try:
		ser.write("{}\n".format(msg))
	except Exception:
		try:
			ser = serial.Serial(sys.argv[1], 9600, timeout=1)
		except Exception as e:
			if log_error:
				logger.error("{}: cannot reopen serial connection: {}, command lost: {}.".format(ts(), e, msg))
			time.sleep(error_wait)
		else:
			ser.write("{}\n".format(msg))

def send_ticks():
	while True:
		serial_write("eachTick", error_wait=30)
		time.sleep(1)

def send_states():
	try:
		f = open(syslog)
	except Exception as e:
		logger.error("{}: cannot open syslog: {}. exit.".format(ts(), e))
		sys.exit(1)
	f.seek(0, 2)

	t = threading.Thread(target=send_ticks).start()

	while True:
    		latest_logs = f.readline()
    		if latest_logs:
			match = re.match(".+ Vitamin-R .+StateMachine: now in new state: <([a-zA-Z]+):.+", latest_logs)
			if match:
				command = match.group(1)
				logger.debug("{}: sending changed Vitamin-R state to serial: {}".format(ts(), command))
				serial_write(command, log_error=True)

if __name__ == "__main__":
	if len(sys.argv) != 2:
		print "usage: {} /dev/vitamin-led-usb\n".format(sys.argv[0])
		sys.exit(1)

	logger.setLevel(logging.DEBUG)
	logger.propagate = False
	fh = logging.FileHandler(log_file, "w")
	fh.setLevel(logging.DEBUG)
	logger.addHandler(fh)

	logger.info("{}: start watching syslog for Vitamin-R state changes...".format(ts()))

	daemon = Daemonize(
		app="vitamin-led",
		pid=pid_file,
		action=send_states,
		keep_fds=[fh.stream.fileno()]
	)
	daemon.start()
