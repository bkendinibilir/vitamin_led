import re
import serial
import signal
import sys
import time
import threading

def exit_program(msg, errorcode=0):
	print msg
	sys.exit(errorcode)

def send_ticks():
	while True:
		ser.write("eachTick\n")
		time.sleep(1)

def sigint_handler(signal, frame):
	print "closing logfile and serial connection. bye."
	f.close()
	ser.close()
        sys.exit(0)

def send_states():
   	f.seek(0, 2)
	while True:
    		latest_logs = f.readline()
    		if latest_logs:
			match = re.match(".+ Vitamin-R .+StateMachine: now in new state: <([a-zA-Z]+):.+", latest_logs)
			if match:
				command = match.group(1)
				print "sending changed Vitamin-R state to serial: {}".format(command)
				ser.write("{}\n".format(command));

if __name__ == "__main__":
	if len(sys.argv) != 2:
		exit_programm("usage: {} /dev/vitamin-led-usb\n".format(sys.argv[0]), 1)

	print "opening serial connection and watching syslog for Vitamin-R state changes..."

	ser = serial.Serial(sys.argv[1], 9600, timeout=1)
	if not ser:
		exit_programm("cannot open serial device {}.".format(sys.argv[1]), 1)

	f = open('/var/log/system.log')
	if not f:
		exit_programm("cannot open /var/log/syslog.".format(sys.argv[1]), 1)

	signal.signal(signal.SIGINT, sigint_handler)

	t = threading.Thread(target=send_ticks).start()

	send_states()	
