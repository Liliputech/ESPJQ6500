/*
 Fichier listing des entètes de patterns
*/

#include "Arduino.h"
#ifndef Patterns_h
#define Patterns_h

//---------DemoReel100: addGlitter--------
void addGlitter( fract8 chanceOfGlitter);

//-------FADE OUT--------
//anciennement fadeall()
void p0();

//-------allumage successif maintenu en avant--------
void p1();

//---------allumage successif maintenu en arrière------
void p2();

//---------Path-drik.ino : allumage successif non maintenu en avant--------
void p3();

//---------Path-drik.ino : allumage non maintenu en arrière--------
void p4();

//--------- DemoReel100: juggle--------
void p5();

//---------DemoReel100: bpm--------
void p6();

//---------DemoReel100: sinelon--------
void p7();

//---------DemoReel100: confetti--------
void p8();

//---------DemoReel100: rainbow--------
void p9();

//---------DemoReel100: rainbowWithGlitter--------
void p10();

//TEST ROUGE
void p11();

//TEST BLEU
void p12();

//TEST VERT
void p13();

#endif