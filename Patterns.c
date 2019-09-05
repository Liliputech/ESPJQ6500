/*
  Fichier contenant les patterns
*/

//---------DemoReel100: addGlitter--------
void addGlitter( fract8 chanceOfGlitter);
{
  if ( random8() < chanceOfGlitter) {
    leds[ random16(NUM_LEDS) ] += CRGB::White;
  }
}


//-------FADE OUT--------
//anciennement fadeall()
void p0() {
  for (int i = 0; i < NUM_LEDS; i++) {
    leds[i].nscale8(250);
    FastLED.show();
    delay(10);
  }
  blocLoop = false;//close flag : la boucle ne continue pas plus que "hold" fois
  Serial.println("fadeall done!");
}


//--------FONCTION DE BOUCLAGE DES PATTERNS-------
//////////////////////////////////// PATTERNS p1() à p10() ///////////////////////////////////////
//-------allumage successif maintenu en avant--------
void p1()
{
  static int i = 0;
    // Set the i'th led to red
    leds[i] = CHSV(hue++, 255, 255);
    // Show the leds
    FastLED.show();
    // now that we've shown the leds, reset the i'th led to black
    // leds[i] = CRGB::Black;
    //fadeall();
    // Wait a little bit before we loop around and do it again
    delay(30);
  i = (i+1) % NUM_LEDS;
}

//---------allumage successif maintenu en arrière--------
void p2()
{
  static int i = 0;
    // Set the i'th led to red
    leds[i] = CHSV(hue++, 255, 255);
    // Show the leds
    FastLED.show();
    // now that we've shown the leds, reset the i'th led to black
    // leds[i] = CRGB::Black;
    //fadeall();
    // Wait a little bit before we loop around and do it again
    delay(30);
  i = (i+1) % NUM_LEDS;
}

//---------Path-drik.ino : allumage successif non maintenu en avant--------
void p3()
{
  // First slide the led in one direction
  static int i = 0;
  // Set the i'th led to White
  leds[i] = CRGB::Red;
  // Show the leds
  FastLED.show();
  // now that we've shown the leds, reset the i'th led to black
  leds[i] = CRGB::Black;
  // Wait a little bit before we loop around and do it again
  delay(5);
  i = (i + 1) % NUM_LEDS;
}

//---------Path-drik.ino : allumage non maintenu en arrière--------
void p4()
{
  static int i = 0 ;
  // Set the i'th led to White
  leds[i] = CRGB::Blue;
  // Show the leds
  FastLED.show();
  // now that we've shown the leds, reset the i'th led to black
  leds[i] = CRGB::Black;
  // Wait a little bit before we loop around and do it again
  delay(5);
  i = (i + 1) % NUM_LEDS;
}

//--------- DemoReel100: juggle--------
void p5() {
  // eight colored dots, weaving in and out of sync with each other
  fadeToBlackBy( leds, NUM_LEDS, 20);
  byte dothue = 0;
  for ( int i = 0; i < 8; i++) {
    leds[beatsin16( i + 7, 0, NUM_LEDS - 1 )] |= CHSV(dothue, 200, 255);
    dothue += 32;

    // send the 'leds' array out to the actual LED strip
    FastLED.show();
    delay(20);
  }
}

//---------DemoReel100: bpm--------
void p6() {
  // colored stripes pulsing at a defined Beats-Per-Minute (BPM)
  uint8_t BeatsPerMinute = 62;
  CRGBPalette16 palette = PartyColors_p;
  uint8_t beat = beatsin8( BeatsPerMinute, 64, 255);
  for ( int i = 0; i < NUM_LEDS; i++) { //9948
    leds[i] = ColorFromPalette(palette, gHue + (i * 2), beat - gHue + (i * 10));

    // send the 'leds' array out to the actual LED strip
    FastLED.show();
    delay(20);
  }
}
//---------DemoReel100: sinelon--------
void p7()
{
  // a colored dot sweeping back and forth, with fading trails
  fadeToBlackBy( leds, NUM_LEDS, 20);
  int pos = beatsin16( 13, 0, NUM_LEDS - 1 );
  leds[pos] += CHSV( gHue, 255, 192);

  // send the 'leds' array out to the actual LED strip
  FastLED.show();
  delay(20);
}

//---------DemoReel100: confetti--------
void p8()
{
  // random colored speckles that blink in and fade smoothly
  fadeToBlackBy( leds, NUM_LEDS, 10);
  int pos = random16(NUM_LEDS);
  leds[pos] += CHSV( gHue + random8(64), 200, 255);

  // send the 'leds' array out to the actual LED strip
  FastLED.show();
  delay(20);
}

//---------DemoReel100: rainbow--------
void p9()
{
  // FastLED's built-in rainbow generator
  fill_rainbow( leds, NUM_LEDS, gHue, 7);

  // send the 'leds' array out to the actual LED strip
  FastLED.show();
  delay(20);
}

//---------DemoReel100: rainbowWithGlitter--------
void p10()
{
  // built-in FastLED rainbow, plus some random sparkly glitter
  p9();
  addGlitter(80);

  // send the 'leds' array out to the actual LED strip
  FastLED.show();
  delay(20);
}

//TEST ROUGE
void p11() {
  //audiofile = 1;
  for (int i = 0; i < NUM_LEDS; i++) {
    leds[i] = CRGB::White;
    FastLED.show();
    leds[i] = CRGB::Black;
    delay(5);
  }
}
//TEST BLEU
void p12() {
  //audiofile = 2;
  for (int i = 0; i < NUM_LEDS; i++) {
    leds[i] = CRGB::White;
    FastLED.show();
    leds[i] = CRGB::Black;
    delay(5);
  }
}

//TEST VERT
void p13() {
  //audiofile = 3;
  for (int i = 0; i < NUM_LEDS; i++) {
    leds[i] = CRGB::White;
    FastLED.show();
    leds[i] = CRGB::Black;
    delay(5);
  }
}
/////////////////////////////////////////////////////////
