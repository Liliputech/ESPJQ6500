 
#include <Arduino.h>
#include <SoftwareSerial.h>
#include <JQ6500_Serial.h>

JQ6500_Serial mp3(0,2);

void setup() {
  mp3.begin(9600);
  mp3.reset();
  mp3.setVolume(20);
  mp3.setLoopMode(MP3_LOOP_ALL);
  mp3.play();  
}

void loop() {
  // Do nothing, it's already playing and looping :-)
}
