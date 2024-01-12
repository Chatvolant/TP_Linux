
cible 1 : 
	echo "1" > /sys/class/leds/fpga_led1/brightness
	sleep 1
	echo "1" > /sys/class/leds/fpga_led2/brightness
	sleep 1
	echo "1" > /sys/class/leds/fpga_led3/brightness
	sleep 1
	echo "1" > /sys/class/leds/fpga_led4/brightness

	sleep 3

	echo "0" > /sys/class/leds/fpga_led1/brightness
	sleep 1
	echo "0" > /sys/class/leds/fpga_led2/brightness
	sleep 0
	echo "0" > /sys/class/leds/fpga_led3/brightness
	sleep 1
	echo "0" > /sys/class/leds/fpga_led4/brightness