# ESP01JQ6500

That code allows an ES01 to control a WSxx ledstrips with FastLed 
https://github.com/FastLED/FastLED

in combination of trigger soundfiles loaded on a JQ6500 chip.
https://github.com/sleemanj/JQ6500_Serial

it uses PubSubClient lib as MQTT environment
https://github.com/knolleary/pubsubclient

Someone has to install a MQTT Mosquitto Broker on the server side:
https://mosquitto.org/download/


The control (audio/light patterns) occurs on Wifi from a MQTT client
There are several ones but command line with MQTT Mosquitto (as client)
is a way to start.


Below some documentation for the syntax to trig a pattern (not updated)
  mosquitto_pub -t my_topic -m my_message 

exemple:
  mosquitto_pub -t ESP_XXXX/ledstate -m 239 (on déclenche le pattern n°239 sur cet ESP spécifiquement


-----

   mosquitto_pub -t welcome -m 0 //on obtient le nom de l'ESP_XXXX sur le topic welcome
   mosquitto_pub -t welcome -m hello world

-----

UPLOAD VIA FTDI232 works fine however a similar chip named YP-05 gives nothing back in Arduino monitor

-----
# JQ6500_ESP01-HelloWorld.ino
I finally realize that serial declaration syntax has changed, JQ6500 helloworld example is a bad guy
----
# JQ6500MQTTPLAYER.ino
this sketch uses its own ESP01 to trig sounfiles, no FastLED
