#include <Arduino.h>
#include "CommunicationCtrl.h"

#define MASTER

CommunicationCtrl *communicate;

void setup() 
{
  Serial.begin(9600);
  communicate = new CommunicationCtrl();
  
}


void loop() 
{
  communicate->loop();
}