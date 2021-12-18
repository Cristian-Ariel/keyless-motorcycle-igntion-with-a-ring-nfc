
void PlayPreTone(const byte tonePin)
{
  tone(tonePin, 2900, 34);
  delay(90);
  tone(tonePin, 2900, 34);
  delay(600);  // A small delay so the sound doesn't overlap other sounds
}

void PlayAcceptedTag(const byte tonePin)
{
  tone(tonePin, 2900, 105);
  delay(100);
  tone(tonePin, 1700, 84);
  delay(200);
  tone(tonePin, 1700, 94);
  delay(594);
}

void PlayUnknwownTag(const byte tonePin)
{
  tone(tonePin, 112, 140);
  delay(260);
  tone(tonePin, 109, 146);
  delay(73);
  tone(tonePin, 110, 126);
  delay(94);
  tone(tonePin, 110, 108);
  delay(194);
  tone(tonePin, 110, 630); // Simultates a "warning" tone
  delay(1000);
}

void PlayEnterSaveDeleteTagMode(const byte tonePin)
{
  tone(tonePin, 1150, 84);
  delay(90);
  tone(tonePin, 1450, 84);
  delay(90);
  tone(tonePin, 1700, 84);
  delay(200);
  tone(tonePin, 1900, 94);
  delay(195);
  tone(tonePin, 2800, 105);
  delay(105);
}

void PlayExitSaveDeleteTagMode(const byte tonePin)
{
  tone(tonePin, 2800, 105);
  delay(105);
  tone(tonePin, 1900, 94);
  delay(195);
  tone(tonePin, 1700, 84);
  delay(200);
  tone(tonePin, 1450, 84);
  delay(90);
  tone(tonePin, 1150, 84);
  delay(500);
}

void PlayEnterEraseMemoryMode(const byte tonePin)
{
  tone(tonePin, 880, 8);
  delay(9);
  tone(tonePin, 2349, 95);
  delay(106);
  tone(tonePin, 1318, 43);
  delay(48);
  tone(tonePin, 880, 8);
  delay(9);
  tone(tonePin, 2349, 95);
  delay(106);
  tone(tonePin, 1318, 43);
  delay(48);
  tone(tonePin, 880, 8);
  delay(9);
  tone(tonePin, 2349, 95);
  delay(106);
  tone(tonePin, 1318, 43);
  delay(48);
  tone(tonePin, 880, 8);
  delay(9);
  tone(tonePin, 2349, 95);
  delay(106);
  tone(tonePin, 1318, 43);
  delay(48);
  tone(tonePin, 880, 8); 
  delay(9);
  tone(tonePin, 2349, 95);
  delay(106);
  tone(tonePin, 1318, 43);
  delay(48);
  tone(tonePin, 880, 8);
  delay(9);
  tone(tonePin, 2349, 95);
  delay(106);
  tone(tonePin, 1318, 43);
  delay(43); // Remember equal to the last duration above
}

void PlayExitEraseMemoryMode(const byte tonePin)
{
  // Same sound from PlayExitSaveDeleteTagMode(), change it if you prefer
  
  delay(400); // A short delay to differentiate whatever was being played before
  tone(tonePin, 2800, 105);
  delay(105);
  tone(tonePin, 1900, 94);
  delay(195);
  tone(tonePin, 1700, 84);
  delay(200);
  tone(tonePin, 1450, 84);
  delay(90);
  tone(tonePin, 1150, 84);
  delay(500);
}


void PlayEnterSleepMode(const byte tonePin)
{
  tone(tonePin, 2637, 100);
  delay(110);      
  tone(tonePin, 293, 10);
  delay(100);
  tone(tonePin, 2637, 80);
  delay(90);
  tone(tonePin, 622, 9);
  delay(10);
  tone(tonePin, 207, 10);
  
  delay(90);
  tone(tonePin, 2637, 100);
  delay(200);
}

void PlayTagSaved(const byte tonePin)
{
  tone(tonePin, 2093, 25);
  delay(30);
  tone(tonePin, 698, 9);
  delay(10);
  tone(tonePin, 233, 9);
  delay(70);
  tone(tonePin, 2349, 26);
  delay(30);
  tone(tonePin, 783, 9);
  delay(10);
  tone(tonePin, 261, 9);
  delay(70);
  tone(tonePin, 1975, 26);
  delay(30);
  tone(tonePin, 830, 9);
  delay(10);
  tone(tonePin, 277, 8);
  delay(80);
  tone(tonePin, 1864, 26);
  delay(30);
  tone(tonePin, 261, 9);
  delay(175);
  tone(tonePin, 1396, 35);
  delay(40);
  tone(tonePin, 277, 9);
  delay(70);
  tone(tonePin, 932, 35);
  delay(40);
  tone(tonePin, 311, 9);
  delay(180);
  tone(tonePin, 2489, 27);
  delay(30);
  tone(tonePin, 830, 9);
  delay(10);
  tone(tonePin, 277, 9);
  delay(40);
  tone(tonePin, 2793, 27);
  delay(30);
  tone(tonePin, 932, 9);
  delay(10); 
  tone(tonePin, 311, 9);
  delay(60);
  tone(tonePin, 2489, 35);
  delay(40);
  tone(tonePin, 349, 9);
  delay(70);
  tone(tonePin, 1567, 35);
  delay(40);
  tone(tonePin, 311, 9);
  delay(180); 
  tone(tonePin, 1760, 35); 
  delay(40);
  tone(tonePin, 349, 9);
  delay(70);
  tone(tonePin, 1174, 35);
  delay(40);
  tone(tonePin, 391, 9);
  delay(155);
  tone(tonePin, 1567, 26);
  delay(30);
  tone(tonePin, 415, 9);
  delay(30); 
  tone(tonePin, 415, 26);
  delay(50);
  tone(tonePin, 1567, 26);
  delay(30);
  tone(tonePin, 523, 9);
  delay(30);
  tone(tonePin, 622, 26);
  delay(50);
  tone(tonePin, 415, 26);
  delay(30);
  tone(tonePin, 523, 9);
  delay(20);
  tone(tonePin, 415, 35);
  delay(60);
  tone(tonePin, 523, 26);
  delay(50);
  tone(tonePin, 1864, 26);
  delay(30);
  tone(tonePin, 622, 9);
  delay(30);
  tone(tonePin, 415, 27);
  delay(50);
  tone(tonePin, 1244, 26);
  delay(30);
  tone(tonePin, 415, 9);
  delay(35);
  tone(tonePin, 523, 27);
  delay(50);
  tone(tonePin, 1864, 27);
  delay(30);
  tone(tonePin, 622, 9);
  delay(35);
  tone(tonePin, 415, 26);
  delay(50);
  tone(tonePin, 415, 26);
  delay(50); 
  tone(tonePin, 1567, 26);
  delay(30);
  tone(tonePin, 523, 9);
  delay(35);
  tone(tonePin, 622, 26);
  delay(60);
  tone(tonePin, 2093, 27);
  delay(30);
  tone(tonePin, 415, 9);
  delay(400);
}

void PlayTagDeleted(const byte tonePin)
{
  tone(tonePin, 1479, 35);
  delay(40);
  tone(tonePin, 932, 9);
  delay(10);
  tone(tonePin, 207, 17);
  delay(20);
  tone(tonePin, 311, 9);
  delay(140);
  tone(tonePin, 1396, 35);
  delay(40);
  tone(tonePin, 880, 9);
  delay(10);
  tone(tonePin, 195, 17);
  delay(150);
  tone(tonePin, 1318, 35);
  delay(40);
  tone(tonePin, 830, 9);
  delay(10);
  tone(tonePin, 184, 18); 
  delay(150);
  tone(tonePin, 783, 35);
  delay(40);
  tone(tonePin, 174, 26);
  delay(160);

  tone(tonePin, 311, 175);
  delay(200);
  tone(tonePin, 466, 10);
  delay(100);
  tone(tonePin, 554, 10);
  delay(100);
  tone(tonePin, 277, 10);
  delay(10);
  tone(tonePin, 369, 220);
  delay(240);
  tone(tonePin, 554, 8);
  delay(10);

  tone(tonePin, 5, 180);
  delay(180);
  tone(tonePin, 55, 105);
  delay(125);
  tone(tonePin, 55, 155);
  delay(205);
  tone(tonePin, 164, 25);
  delay(30);
  tone(tonePin, 55, 10);
  delay(10);
  tone(tonePin, 110, 10);
  delay(15);
  tone(tonePin, 55, 10);
  delay(10);
  tone(tonePin, 5, 180);
  delay(80);
}

void PlayMemoryErased(const byte tonePin)
{
  tone(tonePin, 1174, 52);
  delay(80);
  tone(tonePin, 220, 52);
  delay(80);
  tone(tonePin, 207, 44);
  delay(50);
  tone(tonePin, 1046, 9);
  delay(30);
  tone(tonePin, 195, 44);
  delay(80);
  tone(tonePin, 184, 35);
  delay(70);
  tone(tonePin, 174, 44);
  delay(150);
  tone(tonePin, 783, 44);
  delay(80);
  tone(tonePin, 739, 44);
  delay(80);
  tone(tonePin, 138, 35);
  delay(40);
  tone(tonePin, 146, 9);
  delay(10);
  tone(tonePin, 110, 89);
  delay(20);
  tone(tonePin, 130, 35);
  delay(70);
  tone(tonePin, 369, 44);
  delay(80);
  tone(tonePin, 116, 35);
  delay(50);
  tone(tonePin, 130, 9);
  delay(10);
  tone(tonePin, 97, 9);
  delay(20);
  tone(tonePin, 554, 52);
  delay(130);
  tone(tonePin, 87, 9);
  delay(10);
  tone(tonePin, 493, 114);
  delay(120);
  tone(tonePin, 97, 790);
  delay(1000);
}

void PlayMemoryFull(const byte tonePin)
{
  tone(tonePin, 112, 105);
  delay(200);
  tone(tonePin, 112, 630); // Simultates a "warning" tone
  delay(290);
  tone(tonePin, 75, 930);  // Simultates a "warning" tone
  delay(1200);
}

void PlayReaderNotFound(const byte tonePin)
{
  tone(tonePin, 2070, 1500); 
  delay(2000);
  tone(tonePin, 132, 1800); 
  delay(1800);
}
