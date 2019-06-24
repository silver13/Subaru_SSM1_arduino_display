# Subaru SSM1 reader

Uses an arduino mini pro to display values from the ECU of a Subaru Liberty / Legacy.

This project was made to suit my needs and is posted as an example to help someone who wants to do similar things. The main things to consider are, the locations to read vary between rom versions, and as such those locations should be changed to suit your car ecu version.

## Parts

* Arduino mini Pro (without USB so there is no conflict on serial lines)
* 0.96 OLED display SSD1306 based, connect to arduino i2c pins
* button to ground for display mode changing on pin 10 default
* ssm1 plug, a similar plug is under the Liberty 92 dash, connected to indicator / headlights or similar. I got one from the wreckers for $1. May require pin relocation ( easy )

I used 1k ( actually I think 750 as I was out of 1k ) resistors on the serial lines in order to protect the computer. Probably not needed, but easy protection compared to replacing a 30 Y.O. ecu
