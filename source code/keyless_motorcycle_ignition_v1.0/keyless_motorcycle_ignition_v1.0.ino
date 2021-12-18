/*******************************************************************************
Program name: Keyless Motorcycle Ignition with a Ring (NFC)

Version: 1.0
Author: Cristian Ariel
Created: 2020-07-07
********************************************************************************/

#include <EEPROM.h>
#include <avr/sleep.h>
#include <Wire.h>
#include "soundsList.h" // File is in same directory as main source code

// Libraries for the NFC module
#include <PN532_I2C.h>
#include <PN532.h>
#include <NfcAdapter.h>

// Coment out next line to remove serial monitor commands
//#define DEBUG 
#ifdef DEBUG 
  #define DEBUG_PRINT(x) Serial.print(x)
  #define DEBUG_PRINTLN(x) Serial.println(x)
#else 
  #define DEBUG_PRINT(x)
  #define DEBUG_PRINTLN(x)
#endif

// Set pins to connect over I2C, SDA (pin 27) and SCL (pin 28)
PN532_I2C pn532i2c(Wire);  
PN532 nfcReader(pn532i2c); 

// Intialize pins as labeled on the Arduino Uno board
const byte KICKSTAND_SW        = 2;  // Connected to interrupt-capable pin
const byte CLOCK_PIN           = 4;  // To pin 15 of CD4017
const byte RELAY_DRIVER        = 5;  // To N-channel MOSFET driving relay controlling ignition switch
const byte READR_LIGHTSC_DRIVR = 7;  // To N-channel MOSFET
const byte BUZZER_PIN          = 12; // To N-channel MOSFET
const byte POWER_PIN           = 13; // To N-channel MOSFET driving P-channel MOSFET to turn on/off system

// Initialize global variables
byte scannedTagUid[] = {0 ,0 ,0 ,0 ,0 ,0 ,0 }; // Stores returned UID number from NFC reader
byte tagUidLength    = 0;                      // It can be 4 or 7 bytes depending on card type
const byte MASTER_TAG_UID[] = {4, 203, 105, 74, 40, 99, 128}; // Replace it with your tag UID
const byte MAX_NUM_TAGS     = 20; // Max number of tags that can be stored into EEPROM, limit is 128
int tagExistsFlagAddress    = 0; // Updated by VerifyRegularKeyTagMatchAndGetTagExistsFlagAddress() 
                                 // and used by DeleteTagFromEeprom()
bool masterTagUsedAsKey     = false;

// Timing variables in milliseconds
const unsigned int DISARMED_MODE_MAX_ACTIVE_TIME = 15000; // Time system stays on in Disarmed Mode
const unsigned int BUZZER_ARM_MD_MAX_ACTIVE_TIME = 10000; // Time background sound will play in Armed Mode
const unsigned int ERASE_MEMORY_MAX_ACTIVE_TIME  = 15000; 

// Timing variables for the reset functions
unsigned long eraseMemoryModeStartTime          = 0;
unsigned long bckgrndSoundDisarmedModeStartTime = 0;

// Ligth scanner speed options, ordered from slowest to fastest
const int ARMED_MODE_LGHT_S     = 75;
const int DISSARMED_MODE_LGHT_S = 30;  
const int SAV_DEL_MODE_LGHT_S   = 26;  // Save/Delete Tag Mode
const int ERASE_MEM_LGHT_S      = 10;  // Erase Memory Mode


/*******************************************************************************
*  Setup Function: runs only once                                              *
*******************************************************************************/
void setup () 
{
  pinMode(KICKSTAND_SW, INPUT);
  pinMode(CLOCK_PIN , OUTPUT);  
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(POWER_PIN, OUTPUT);  
  pinMode(RELAY_DRIVER, OUTPUT);
  pinMode(READR_LIGHTSC_DRIVR, OUTPUT);

  // Set pin states
  digitalWrite(RELAY_DRIVER, LOW);         // Keep MOSFET off
  digitalWrite(POWER_PIN, HIGH);           // Turn on MOSFET to keep system on
  digitalWrite(READR_LIGHTSC_DRIVR, HIGH); // Turn on MOSFET 

  #ifdef DEBUG
  Serial.begin(9600); // Set baud rate
  Serial.println("Keyless Motorcycle Ignition System");
  #endif                        
}

/*******************************************************************************
*  Loop Function: runs on forever                                              *
*******************************************************************************/
void loop ()
{ 
  DEBUG_PRINTLN("====================== Entering DISARMED MODE ======================");
 
  DisarmedMode();
  ArmedMode();
} 


enum SoundsNameList
{
  // Enumerators are converted automatically into constant integers
  // This enumeration contains the NAME of every sound used by PlaySound() 

  // Background sounds are played while we are in a specific "Mode"
  //
  ARMED_MODE_SOUND    = 1, // Slow
  DISARMED_MODE_SOUND = 2, // Normal
  SAV_DEL_MODE_SOUND  = 3, // Fast
  ERASE_MEM_SOUND     = 4, // Extra fast 

  TAG_SCANNED    = 5,  // Play every time we scan any tag 
  ACCEPTED_TAG = 6,    // "" if either regular key tag or the master tag is scanned.
                       //    It's also played when turning off motorcycle
  UNKNOWN_TAG  = 7,    // "" if either the tag is invalid (UID != 7) or unknown

  ENTER_SAV_DEL_MODE   = 8,  // Play when master tag is scanned
  EXIT_SAV_DEL_MODE    = 9,  // "" when master tag is scanned
  ENTER_ERASE_MEM_MODE = 10, // "" when brake pedal is pressed 
  EXIT_ERASE_MEM_MODE  = 11, 
  ENTER_SLEEP_MODE     = 12, // "" when kickstand switch is pressed

  TAG_SAVED        = 13, // Play when new tag is scanned
  TAG_DELETED      = 14, // "" when known tag is scanned
  MEM_ERASED       = 15, // "" when the master tag is scanned 
  MEMORY_FULL      = 16, // "" when we've reached the limit (MAX_NUM_TAGS)
  READER_NOT_FOUND = 17  // "" when reader is disconnected/offline
};

/*******************************************************************************
* DISARMED MODE: motorcycle system is off                                      *
*******************************************************************************/
void DisarmedMode ()
{
  NfcReaderConnectionHandler();

  while (true) { // Infinite loop
    DisarmedModeCountdownHandler(DISARMED_MODE_MAX_ACTIVE_TIME); // Check if it's time to turn system off
    SetSpeedAndTurnOnLightScanner(DISSARMED_MODE_LGHT_S);
    PlaySound(DISARMED_MODE_SOUND);
        
    bool tagFound = nfcReader.readPassiveTargetID(PN532_MIFARE_ISO14443A, &scannedTagUid[0], &tagUidLength);
    if (tagFound == true) {             
      PlaySound(TAG_SCANNED); // Everytime a tag is scanned

      #ifdef DEBUG
      PrintTagUid(scannedTagUid); 
      #endif

      if (tagUidLength == 7) {
        bool regularKeyTagWasScanned  = VerifyRegularKeyTagMatchAndGetTagExistsFlagAddress(scannedTagUid); 
        bool masterTagWasScanned = VerifyMasterTagMatch(scannedTagUid, MASTER_TAG_UID); 
        if (regularKeyTagWasScanned || masterTagWasScanned) { // known tag?
          digitalWrite(RELAY_DRIVER, HIGH); // Access has been granted, turn on motorycle!
          PlaySound(ACCEPTED_TAG); 
          
          // Check which kind of known tag we have
          if (regularKeyTagWasScanned == true) {
            DEBUG_PRINTLN("  =  Regular key tag scanned!!");
            return; // Go back to loop function and enter Armed Mode
          }
          else if (masterTagWasScanned == true) {
            DEBUG_PRINTLN("  =   This is the master tag!!");
            masterTagUsedAsKey = true;
            SaveDeleteTagMode();
            return; // Go back to loop function and enter Armed Mode
          }          
        }
        else {
          DEBUG_PRINTLN("  =  Unknown tag scanned!!");
          PlaySound(UNKNOWN_TAG);
        }
      } // End of if (tagUIDLenght == 7)
      else {
        DEBUG_PRINTLN("  =  Tag UID not supported!!"); 
        PlaySound(UNKNOWN_TAG);
      }

      DEBUG_PRINTLN("\nWaiting for a tag...\n");
      delay(600); // Wait a little before continuing, otherswise we will keep calling the functions
    } // End of if(tagFound)
  } // End of while(true)

}

void NfcReaderConnectionHandler ()
{
 // Check that reader is connected, if there is communication set up reader 
 // and continue normal program flow, otherwise turn off MCU.

  nfcReader.begin(); // Initialize communication with the reader module
  bool readerIsConnected = nfcReader.getFirmwareVersion(); // See if reader responds by requesting some info
  if (readerIsConnected == true) { 
    DEBUG_PRINTLN("Reader Found!!\n");
    DEBUG_PRINTLN("Waiting for a tag... \n");
  /*
    Set the max number of retry attempts to read from a card. 
    This prevents us from waiting forever for a card, which is the default behaviour 
    of the PN532. 0xFF to wait forever, 0x00..0xFE to timeout after x retries.
  */
    nfcReader.setPassiveActivationRetries(0x01); 
    nfcReader.SAMConfig(); // Configures board to read RFID tags 
  }
  else {
    DEBUG_PRINTLN("Reader Not Found!!\n");
    DEBUG_PRINTLN("SYSTEM OFF!!!");

    PlaySound(READER_NOT_FOUND);
    while (true) { // Endless loop
    digitalWrite(POWER_PIN, LOW); // Turn off MCU
    }
  }

}

void PlaySound (const int SOUND_NAME)
{   
  static unsigned long previousTimeTone = 0;    
  unsigned long currentTimeInMillis = millis(); // Counter module built-in into the MCU
  unsigned long timeElapsed = (currentTimeInMillis - previousTimeTone);

  // Note that each case label needs to be a constant
  //
  switch (SOUND_NAME)
  {
  case ARMED_MODE_SOUND:
    if (timeElapsed >= 2000) { // Computes the delay. Set speed by changing value on the right
      tone(BUZZER_PIN, 1600, 84);
      previousTimeTone = currentTimeInMillis; // Update the time for the next time around
    } 
    break;

  case DISARMED_MODE_SOUND:
    if (timeElapsed >= 900) {
      tone(BUZZER_PIN, 1220, 84);
      previousTimeTone = currentTimeInMillis; 
    }
    break;

  case SAV_DEL_MODE_SOUND:
    if (timeElapsed >= 800) {
      tone(BUZZER_PIN, 1220, 84);
      previousTimeTone = currentTimeInMillis; 
    }
    break;

  case ERASE_MEM_SOUND:
    if (timeElapsed >= 300) {
      tone(BUZZER_PIN, 130, 95);
      previousTimeTone = currentTimeInMillis; 
    }
    break;

  case TAG_SCANNED:
    PlayPreTone(BUZZER_PIN);
    break;

  case ACCEPTED_TAG: 
    PlayAcceptedTag(BUZZER_PIN);
    break;

  case UNKNOWN_TAG:
    PlayUnknwownTag(BUZZER_PIN);
    break;

  case ENTER_SAV_DEL_MODE:
    PlayEnterSaveDeleteTagMode(BUZZER_PIN);
    break;

  case EXIT_SAV_DEL_MODE:
    PlayExitSaveDeleteTagMode(BUZZER_PIN);
    break;

  case ENTER_ERASE_MEM_MODE:
    PlayEnterEraseMemoryMode(BUZZER_PIN);
    break;

  case EXIT_ERASE_MEM_MODE:
    PlayExitEraseMemoryMode(BUZZER_PIN);
    break;
  
  case ENTER_SLEEP_MODE:
    PlayEnterSleepMode(BUZZER_PIN);
    break;

  case TAG_SAVED:
    PlayTagSaved(BUZZER_PIN);
    break;

  case TAG_DELETED:
    PlayTagDeleted(BUZZER_PIN);
    break;

  case MEM_ERASED:
    PlayMemoryErased(BUZZER_PIN);
    break;

  case MEMORY_FULL:
    PlayMemoryFull(BUZZER_PIN); 
    break; 

  case READER_NOT_FOUND:
    PlayReaderNotFound(BUZZER_PIN);
    break;
  }
}

void DisarmedModeCountdownHandler (const unsigned int MAX_ACTIVE_TIME)
{
  // Make sure we record the starting time ONLY once, when first entering this function
  static unsigned long disarmedModeStartTime = 0;
  static bool startTimeWasRecorded = false;
  if (startTimeWasRecorded == false) {
    disarmedModeStartTime = millis(); // Updade it to current time
    startTimeWasRecorded = true;      // Toggle flag, so we record starting time once
  }

  // Check if it's time to turn off system while in Disarmed Mode 
  unsigned long disarmedModeElapsedTime = (millis() - disarmedModeStartTime); // Update time frequenly
  if (disarmedModeElapsedTime >= MAX_ACTIVE_TIME) { // See if time's up
    DEBUG_PRINTLN("SYSTEM OFF!!!");
    
    disarmedModeStartTime = false; // This not necessary. It's just for debbuging so before turning  
                                   // system off, reset variable for the next time we enter Disarmed Mode
    while (true) {
    digitalWrite(POWER_PIN, LOW);  // Turn off MCU
    }
  }

}

void SetSpeedAndTurnOnLightScanner (const unsigned int CLOCK_SPEED)
{
  static bool clockPolarity = LOW; // Sets clock pin of CD4017 either LOW or HIGH                         
  static unsigned long previousTimeClock = 0; // Tracks the number of seconds that have passed by using the timer 
  unsigned long timeElapsed = (millis() - previousTimeClock); // millis() is a counter module built-in into the MCU 
  if (timeElapsed >= CLOCK_SPEED) {         // Computes the 'delay'
    clockPolarity = !clockPolarity;         // Toggle clock signal variable
    previousTimeClock = millis();           // Update time for the next time around
    digitalWrite(CLOCK_PIN, clockPolarity); // Toggle clock signal pin
  }

} 

bool VerifyRegularKeyTagMatchAndGetTagExistsFlagAddress (byte scannedTagUid[])
{ 
  // Check if scanned tag was saved to EEPROM, if so, get its address 
  // Loop through all the tags. If tag found, loop to see if there is a match, if so, get flag address

  // Values of the 1st tag to start reading from
  int tagExistsFlagTempAddr = 0; // 1s byte address preceding each tag's full UID address. Update it each time through the loop
  int tagAddressIndex       = 1; // This is always the address right after the tagExistsFlagAddress
  int lastTagAddressIndex   = 7; // Holds tag's last UID byte address 

  for (byte i = 0; i < MAX_NUM_TAGS; i++) { // Loop through all the max number of tags    
    bool tagExistsFlag = EEPROM.read(tagExistsFlagTempAddr); // Read 1st byte preceding each tag UID address
    if (tagExistsFlag == true) { // Tag found, now make sure it's a match 
      byte uidArrayIndex      = 0;                               
      byte tagUidMatchCounter = 0;  // Keep track of the bytes that have matched our scanned tag.
      for (tagAddressIndex; tagAddressIndex <= lastTagAddressIndex; tagAddressIndex++, uidArrayIndex++) { // Loop through the UID number            
        if (scannedTagUid[uidArrayIndex] == EEPROM.read(tagAddressIndex)) { // Read EERPOM and look for an UID byte match
          tagUidMatchCounter++;   // Update counter
          if (tagUidMatchCounter == 7) { // If all bytes have matched, it's a regular key tag! 
            tagExistsFlagAddress = tagExistsFlagTempAddr; // Get EEPROM address of scanned tag (it's saved in a global variable)
                                                          // This variable is ONLY used as argument for DeleteTagFromEeprom()
            return true; // Regular key tag found, stop looping and exit function
          }
        }
        else { // At the 1st mistmach break current loop, and go read the next tag in EERPOM
          break;  
        }
      } // End of for (tagAddressIndex...
    } // End of if(tagExistsFlag... 

    // Update variables if no tag was found or if there is a mismatch, so that we're ready to read next tag
    tagExistsFlagTempAddr += 8;            // This is the next tagExistsFlagAddress
    // For the loops
    tagAddressIndex = tagExistsFlagTempAddr + 1; 
    lastTagAddressIndex  = tagExistsFlagTempAddr + 7;  
  } // End of for loop
  return false; // Unknown tag! Exit function

}

bool VerifyMasterTagMatch (byte scannedTagUid[], const byte MASTER_TAG_UID[]) 
{
  byte tagUidMatchCounter = 0; 
  for (byte i = 0; i < 7; i++) { // Loop through the UID number
    if (scannedTagUid[i] == MASTER_TAG_UID[i]) { // Look for a match
      tagUidMatchCounter++; 
      if (tagUidMatchCounter == 7) {
        return true;   // All bytes have matched, we got the master tag!
      }
    }    
  }
  return false; // Break function as soon as a mismatch is found

} 

/*******************************************************************************
* SAVE & DELETE TAG MODE: master tag was scanned                               *
*******************************************************************************/
void SaveDeleteTagMode () 
{
  #ifdef DEBUG
  DEBUG_PRINTLN("\n====================== Entering SAVE/DELETE TAG MODE ===============");
  PrintTagsSavedCounter();  
  PrintEepromContent();
  DEBUG_PRINTLN("\nPlease scan tag to add or delete...\n");  
  #endif

  PlaySound(ENTER_SAV_DEL_MODE);
  delay(1000); // Since we just scanned the master tag wait a little, otherwise will break the infinte loop 

  while (true) { // Infinite loop to scan a tag    
    PlaySound(SAV_DEL_MODE_SOUND);  
    SetSpeedAndTurnOnLightScanner(SAV_DEL_MODE_LGHT_S); 
    EnterEraseMemoryModeHadler(); // Check kickstand and enter Erase Memory Mode if it's up
        
    // Reset variable and scan again
    bool tagFound = nfcReader.readPassiveTargetID(PN532_MIFARE_ISO14443A, &scannedTagUid[0], &tagUidLength);  
    if (tagFound == true) {
       PlaySound(TAG_SCANNED);  // Everytime a tag is scanned

      #ifdef DEBUG
      PrintTagUid(scannedTagUid); 
      #endif
      
      if (tagUidLength == 7) { // Identify the type of tag
        bool regularKeyTagWasScanned  = VerifyRegularKeyTagMatchAndGetTagExistsFlagAddress(scannedTagUid); 
        bool masterTagWasScanned = VerifyMasterTagMatch(scannedTagUid, MASTER_TAG_UID);
        // Valid tag, now decide what to do with it...

        if (regularKeyTagWasScanned == false && masterTagWasScanned == false) { // It's a new tag, save it!
          bool eepromIsFull = CheckAvailableEepromSpace(); // But make sure we have enough space to save it first
          if (eepromIsFull == false) { 
              SaveTagToEeprom(UpdateAddrToWriteInto(), scannedTagUid); // Enough memory available, save it!
              PlaySound(TAG_SAVED);
          }
          else if (eepromIsFull == true){ 
            PlaySound(MEMORY_FULL);
            DEBUG_PRINT("\n");
            DEBUG_PRINT("\n____________ Memory Full!! _____________\n\n");
          } 
        }
        else if (masterTagWasScanned == true) { // Exit current mode
          PlaySound(EXIT_SAV_DEL_MODE);
          DEBUG_PRINTLN("  =   This is the master tag!!\n");
          DEBUG_PRINTLN("Exiting Save/Delete tag mode!");

          return; // Break infinite while loop and go back to Armed Mode
        }        
        else if (regularKeyTagWasScanned == true) { // Delete tag!
          DeleteTagFromEeprom(tagExistsFlagAddress); 
          PlaySound(TAG_DELETED);   
        }

        #ifdef DEBUG
        // Print some info after deleting/saving a tag
        PrintTagsSavedCounter();
        PrintEepromContent();
        #endif
      }
      else { // Tag not supported (UID != 7)
        DEBUG_PRINTLN("  = Tag UID not supported!!\n"); 
        PlaySound(UNKNOWN_TAG);
      }
      //delay(800); // We already added a small delay when playing a sound so add another delay if 
                    // you prefer so we don't add the same tag right after erasing it, if applicable.
    } // End of if(tagFound)
  } // End of infinite while loop

}

void EnterEraseMemoryModeHadler ()
{
  // Reads the state of the brake reed switch to enter Erase Memory Mode 

  bool kickstandSwitchPressed = !digitalRead(KICKSTAND_SW); // Pin is normally high, so change it normally low
  if (kickstandSwitchPressed == true) {
    DEBUG_PRINTLN("Entering Erase Memory Mode\n");
    EraseMemoryMode();
  }

}

/*******************************************************************************
* ERASE MEMORY MODE: kickstand reed switch triggered in Save/Delete Tag Mode   *
*******************************************************************************/
void EraseMemoryMode ()
{
  #ifdef DEBUG
  DEBUG_PRINTLN("\n====================== Entering ERASE MEMORY MODE ===============");
  PrintTagsSavedCounter();
  PrintEepromContent();
  DEBUG_PRINTLN("Please re-scan the master tag to erase all tags from EEPROM");
  DEBUG_PRINTLN("or scan any other tag to exit...\n");
  #endif

  PlaySound(ENTER_ERASE_MEM_MODE);
  ResetEraseMemoryCountdownTimer();

  while (true) { // Infinite loop
    if (IsTimeToExitEraseMemoryMode(ERASE_MEMORY_MAX_ACTIVE_TIME) == true) {
      PlaySound(EXIT_ERASE_MEM_MODE);
      break; // Break current infinite loop to exit function thereafter
    }

    PlaySound(ERASE_MEM_SOUND);
    SetSpeedAndTurnOnLightScanner(ERASE_MEM_LGHT_S);    
    
    bool tagFound = nfcReader.readPassiveTargetID(PN532_MIFARE_ISO14443A, &scannedTagUid[0], &tagUidLength); // Reset variable and scan again  
    if (tagFound == true) {
      PlaySound(TAG_SCANNED); // Everytime any tag is scanned
      bool masterTagWasScanned = VerifyMasterTagMatch(scannedTagUid, MASTER_TAG_UID);
      if (masterTagWasScanned == true) { // Erase tags      
        DEBUG_PRINTLN("Erasing all tags from EEPROM\n");
        EraseMemory(); 
        PlaySound(MEM_ERASED);
        PlaySound(EXIT_ERASE_MEM_MODE);
        DEBUG_PRINTLN("Done!!!\n");        

        #ifdef DEBUG
        // Again print some data before exiting
        PrintTagsSavedCounter();
        PrintEepromContent();
        #endif

        break; // Break current infinite loop to exit function thereafter
      } 
      else { // If a tag other than the master tag was scanned, then exit function        
        DEBUG_PRINTLN("Exiting Erase Memory Mode");

        PlaySound(ACCEPTED_TAG);
        PlaySound(EXIT_ERASE_MEM_MODE);
        delay(500); // Wait while moving tag away to avoid scanning the tag twice
        
        #ifdef DEBUG      
        DEBUG_PRINTLN("\n====================== Entering SAVE/DELETE TAG MODE ===============");
        PrintTagsSavedCounter(); 
        DEBUG_PRINTLN("\nPlease scan tag to add or delete...\n");  
        #endif
       
        break; // Break current infinite loop to exit function thereafter
      }
    } // End of if (tagFound...
  } // End of while loop
  return; // Go back to Save/Delete Tag Mode
}

void ResetEraseMemoryCountdownTimer ()
{
  eraseMemoryModeStartTime = millis(); // Update time
}

bool IsTimeToExitEraseMemoryMode (const unsigned int MAX_ACTIVE_TIME)
{
  // Start countdown timer and check if time's up
  unsigned long eraseMemoryModeElapsedTime = (millis() - eraseMemoryModeStartTime); // Keep updating time
  if (eraseMemoryModeElapsedTime >=  MAX_ACTIVE_TIME) { // See if time is up
    return true; // Time's up!!
  }
  else { 
    return false;
  }
}

void EraseMemory ()
{
  noInterrupts(); // Disable interrupts while we are erasing tags
  int tagExistsFlagAddress = 0;
  int tagAddressIndex      = 1; // This is always the address right after the tagExistsFlagAddress
  int lastTagAddressIndex  = 7;

  for (byte i = 0; i < MAX_NUM_TAGS; i++) {    // Loop through all the saved tags
    bool tagExistsFlag = EEPROM.read(tagExistsFlagAddress); // Read the byte preceding every tag UID number
    if (tagExistsFlag == true) {            
      EEPROM.update(tagExistsFlagAddress, false); // Before erasing the UID number, erase the tagExistsFlag 1st
      for (tagAddressIndex; tagAddressIndex <= lastTagAddressIndex; tagAddressIndex++) { // Loop through the UID number
        EEPROM.write(tagAddressIndex, 0);   // Erase by setting each byte to 0
      }
    } // End of if (tagExistsFlag...
    // Update variables to erase next tag
    tagExistsFlagAddress += 8;             
    tagAddressIndex = tagExistsFlagAddress + 1;
    lastTagAddressIndex  = tagExistsFlagAddress + 7;
  }

  interrupts(); // Re-enable them
}

bool CheckAvailableEepromSpace ()
{ 
  // Computes memory availabe in EEPROM. Returns true if memory is full 
   
  byte tagsSavedCounter = 0; // Store number of cards currently saved in EEPROM
  int tagExistsFlagAddress  = 0;  

  for (byte i = 0; i < MAX_NUM_TAGS; i++) { // Loop through all of our saved tags
    bool tagExistsFlag = EEPROM.read(tagExistsFlagAddress); // Read 1s byte preceding each tag's full UID address 
                                                    // tagExistsFlag state: true = used address and false = empty
    if (tagExistsFlag == true) {   // If we find a tag...
      tagsSavedCounter++; // Increase the counter
      if (tagsSavedCounter >= MAX_NUM_TAGS) { // If EEPROM is already filled up
        return true; 
      }
    } // End of if(tagExistsFlag)  
    tagExistsFlagAddress += 8; // No tag found, go read next address
  } // End of for loop
  return false; // Memory available

}

void SaveTagToEeprom (int eepromTagAddress, byte tagUid[])
{ 
  DEBUG_PRINT("  = Saving tag UID to EEPROM!!\n");
  noInterrupts(); 

  int tagExistsFlagAddress = eepromTagAddress - 1; // The nth address to write to - 1
  for (byte i = 0; i < 7; i++) {       // Loop through the tag UID and start writing to EEPROM
    EEPROM.write(eepromTagAddress, tagUid[i]);  
    eepromTagAddress++; // Go to the next EEPROM byte address after writing to it
  }

  EEPROM.write(tagExistsFlagAddress , true);  // After writing to EEPROM, update the tagExistsFlag for that address
  interrupts();  
  DEBUG_PRINT("Done!!\n\n");

}

int UpdateAddrToWriteInto ()
{
  // Look for the EEPROM address where the next tag 1st UID byte will be saved

  int addrToWriteInto = 1;  // The start EEPROM address where a tag's very 1st UID byte will be saved
                            // At first, this value is always the next after the tagExistsFlag address, hence 1.
  int tagExistsFlagAddress = 0; 

  for (byte i = 1; i <= MAX_NUM_TAGS; i++) { // Loop through all of the saved tags
    byte tagExistsFlag = EEPROM.read(tagExistsFlagAddress); // Read 1s byte preceding each tag full address 
                                                 // tagExistsFlag state: true = used address or false = empty
    if (tagExistsFlag == 0) {   // If address is available
      addrToWriteInto = tagExistsFlagAddress + 1; // Update address where the next tag UID number will be written to
                                                  // This value will ONLY be used by SaveTagToEeprom()
      break; // Break loop once we set the address to avoid changing it again
    }
    else {  // Otherwise, if it already contains a tag UID number (tagExistsFlag == true)
      tagExistsFlagAddress += 8; // Update tagExistsFlagAddress to read it next
    }
  } // End of for loop
  return addrToWriteInto; 

}

void DeleteTagFromEeprom (int tagExistsFlagAddress)
{ 
  // It's not necessary to erase the entire UID number, just the tagExistsFlag!
  DEBUG_PRINT("  = Erasing tag from EEPROM!!\n");
  noInterrupts(); // Disable interrupts 

  int tagAddressIndex = tagExistsFlagAddress + 1; // Keep updating it each time through the loop
  int lastTagAddressIndex = tagExistsFlagAddress + 7; 
  EEPROM.write(tagExistsFlagAddress, false);    // Before erasing, set tagExistsFlag to false (0)
  for (tagAddressIndex; tagAddressIndex <= lastTagAddressIndex; tagAddressIndex++) { // Loop through the UID number                       
    EEPROM.update(tagAddressIndex, 0); // Erase by setting each UID byte to false (0) if it isn't already               
  }

  interrupts(); // Enable interrupts again
  DEBUG_PRINT("Done!!\n\n");

}

/*******************************************************************************
* ARMED MODE: motorcycle system is on                                          *
*******************************************************************************/
void ArmedMode ()
{
  DEBUG_PRINTLN("\n====================== Entering ARMED MODE ========================");
  DEBUG_PRINTLN("Waiting for a tag...\n");  

  ResetArmedBackSoundCountdownTimer();

  while (true) { // Endless loop    
    PlayBackSoundArmedModeCountdownHandler(BUZZER_ARM_MD_MAX_ACTIVE_TIME); // Play background sound, and see if it's time to stop
    SetSpeedAndTurnOnLightScanner(ARMED_MODE_LGHT_S); 
    SleepModeFlagHandler(); // Check kickstand reed switch state to go to sleep

    bool tagFound = nfcReader.readPassiveTargetID(PN532_MIFARE_ISO14443A, &scannedTagUid[0], &tagUidLength);
    if (tagFound == true) { 
      PlaySound(TAG_SCANNED);  // Everytime a tag is scanned

      #ifdef DEBUG
      PrintTagUid(scannedTagUid);
      #endif 

      if (tagUidLength == 7) {
        bool regularKeyTagWasScanned  = VerifyRegularKeyTagMatchAndGetTagExistsFlagAddress(scannedTagUid);
        bool masterTagWasScanned = VerifyMasterTagMatch(scannedTagUid, MASTER_TAG_UID);

        if (regularKeyTagWasScanned == true) { // Turn off motorcycle
          DEBUG_PRINTLN("  =  Regular key tag scanned!!\n");
          DEBUG_PRINTLN("SYSTEM OFF!!!\n");

          PlaySound(ACCEPTED_TAG);
          while (true) { 
          digitalWrite(RELAY_DRIVER, LOW);
          digitalWrite(POWER_PIN, LOW);
          }
        }
        else if (masterTagWasScanned == true) { // Decide whether to turn off bike or enter Save/Delete Tag Mode
          DEBUG_PRINTLN("  =   This is the master tag!!");

          if (masterTagUsedAsKey == true) {     // Was it used to arm the motorcycle? If so, turn off system
            DEBUG_PRINTLN("SYSTEM OFF!!!\n");

            PlaySound(ACCEPTED_TAG);            
            while (true) { 
            digitalWrite(RELAY_DRIVER, LOW);
            digitalWrite(POWER_PIN, LOW);
            }
          }
          else if (masterTagUsedAsKey == false) { // Otherwise we want to save or delete a tag...            
            SaveDeleteTagMode();
            // Do this after exiting Save/Delete Tag Mode
            DEBUG_PRINTLN("\n====================== Entering ARMED MODE ========================"); 
            ResetArmedBackSoundCountdownTimer(); 
          }
          
        }
        else if (regularKeyTagWasScanned == false && masterTagWasScanned == false) {
          DEBUG_PRINTLN("  =  Unknown tag scanned!!"); 
          PlaySound(UNKNOWN_TAG);
        }
      } // End of if (tagUIDLenght...
      else { // If the tag UID length is not 7
        DEBUG_PRINTLN("  =  Tag UID not supported!!"); 
        PlaySound(UNKNOWN_TAG);
      }
      
      DEBUG_PRINTLN("\nWaiting for a tag...\n");
      delay(600); // Wait a little before continuing otherwise we will keep triggering the functions
    } // End of if (tagFound) 
  } // End of endless loop

}

void ResetArmedBackSoundCountdownTimer ()
{
  // Upadate variable to current time for the countdown handler function
  bckgrndSoundDisarmedModeStartTime = millis(); 
}

void PlayBackSoundArmedModeCountdownHandler (const unsigned int MAX_ACTIVE_TIME)
{
  // Play background sound or stop playing it if time's up

  unsigned long backgroundSoundTimeElapsed = (millis() - bckgrndSoundDisarmedModeStartTime); // Keep updating time
  if (backgroundSoundTimeElapsed >= MAX_ACTIVE_TIME) { // See if time is up
    noTone(BUZZER_PIN); // Stop any sound that was being played
  }
  else {
    PlaySound(ARMED_MODE_SOUND); // Start or continue playing sound
  }  

}

void SleepModeFlagHandler ()
{
  // Call sleep function if kickstand reed switch is triggered  

  bool kickstandSwitchActive = !digitalRead(KICKSTAND_SW); // Switch is normally high so invert it
  if (kickstandSwitchActive == true) {
    delay(20); // Wait to avoid false triggering or switch bouncing
    kickstandSwitchActive = !digitalRead(KICKSTAND_SW); // Check kickstand switch state again
    if(kickstandSwitchActive == true) { 
      SleepMode();
    }
  }
  
}

/*******************************************************************************
* SLEEP MODE: kickstand reed switch triggered in Disarmed Mode                *
*******************************************************************************/
void SleepMode () 
{
  // Turns off accesories and configures MCU before sending it to sleep 
  DEBUG_PRINTLN("\n====================== Entering SLEEP MODE ===============");

  PlaySound(ENTER_SLEEP_MODE);
  bool kickstandSwitchActive = !digitalRead(KICKSTAND_SW);
  if (kickstandSwitchActive == true) { // Re-check that kickstand switch was not de-pressed again
                                      // while sound was playing
  // Turn off a couple of things before going to sleep
  noTone(BUZZER_PIN);                
  digitalWrite(READR_LIGHTSC_DRIVR, LOW);  // Turn off NFC module and light scanner
  digitalWrite(CLOCK_PIN, HIGH);            // Make sure clock signal is always high to avoid leakeage

  set_sleep_mode(SLEEP_MODE_PWR_DOWN);  // Define power down sleep mode
  EIFR = 1;  // Clear interrupt flag before attaching interrupt
  attachInterrupt(digitalPinToInterrupt(KICKSTAND_SW), WakeUp, RISING); // Wake up when swtich is de-pressed
  sleep_mode();  // Set sleep mode (SE) bit, this is what really puts the ATmega to sleep 

  // Do this after waking up
  digitalWrite(READR_LIGHTSC_DRIVR, HIGH); // Turn accesories back on again 
  // Setup the reader again. Code below can't be inside WakeUp ISR because they need clocks to work (clocks use interrupts)
  nfcReader.begin();
  nfcReader.setPassiveActivationRetries(0x01); // Important! if not included, will read but stall the program
  nfcReader.SAMConfig();

  ResetArmedBackSoundCountdownTimer();

  DEBUG_PRINTLN("Exiting Sleep Mode!");
  DEBUG_PRINTLN("\n====================== Entering ARMED MODE ========================");
  DEBUG_PRINTLN("Waiting for a tag...\n");
  } // End of if (kickstandSwitchActive == true)

} 

void WakeUp ()
{
  // ISR wakes up MCU when kickstand reed switch is open  
  detachInterrupt(digitalPinToInterrupt(KICKSTAND_SW)); 
}



/*******************************************************************************
* All functions down below display information to the serial monitor           *
*******************************************************************************/
void PrintTagUid (byte tagUid[])
{ 
  DEBUG_PRINT("Scanned UID number: ");

  if (tagUidLength == 7) {
    for (byte i = 0; i < 7; i++) { // Loop through the array holding the UID number
      DEBUG_PRINT(tagUid[i]);
      DEBUG_PRINT(" ");
    } 
  }
  else { // UID length is 4
    for (byte  i = 0; i < 4; i++) { // Only print first 4 digits
      DEBUG_PRINT(tagUid[i]);
      DEBUG_PRINT(" ");
    }
  }

}

void PrintEepromContent ()
{
  DEBUG_PRINTLN("Printing EEPROM content....");

  int eepromData = 0;        // Holds data stored in the EEPROM address
  int eepromDataAddress = 0; // EEPROM address starting at 0
  for (byte i = 0; i < (MAX_NUM_TAGS + 1); i++) { // Print rows accoding to the max number of tags set  + 1 extra for debugging
    for (byte j = 0; j < 8; j++) { // Add 8 columms, starting at address 0
      eepromData = EEPROM.read(eepromDataAddress); // Read 1 byte             
      DEBUG_PRINT(eepromData); // Print the content of that EEPROM address. Change the
                               // the 2nd parameter to either HEX or DEC if you wish
      DEBUG_PRINT("\t");
      eepromDataAddress = eepromDataAddress + 1; // Go to next address  
      } // End of for loop (j columns)
      
    DEBUG_PRINTLN("\n"); // Add a new line   
    }  // End of for loop (i rows)
  DEBUG_PRINTLN("Done Reading!!\n");

}

void PrintTagsSavedCounter ()
{
  byte tagsSavedCounter = 0; // Number of cards stored in EEPROM currently, read it from EEPROM. Used twice, reset it again to avoid overflow
  int tagExistsFlagAddress = 0; // Use a local variable otherwise, it will mess with the other functions
  
  for (byte i = 0; i < MAX_NUM_TAGS; i++) { // Loop through all the max number of tags 
    bool tagExistsFlag = EEPROM.read(tagExistsFlagAddress); // Read the address preceding each tag's initial address. Status of tagExistsFlag either 0 = empty, or 1 = used 
    if (tagExistsFlag == true) { // If we find a tag...
      tagsSavedCounter++; // Increase the counter
    }
    tagExistsFlagAddress += 8; // Then go to the next checkbyte address 
  } // End of for loop
 
  DEBUG_PRINT("Tags Saved to EEPROM: ");
  DEBUG_PRINT(tagsSavedCounter);
  DEBUG_PRINT(" out of ");
  DEBUG_PRINTLN(MAX_NUM_TAGS);
}
