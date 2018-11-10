/*
   Typical pin layout used:
   -----------------------------------------------------------------------------------------
               MFRC522      Arduino       Arduino   Arduino    Arduino          Arduino
               Reader/PCD   Uno/101       Mega      Nano v3    Leonardo/Micro   Pro Micro
   Signal      Pin          Pin           Pin       Pin        Pin              Pin
   -----------------------------------------------------------------------------------------
   RST/Reset   RST          9             5         D9         RESET/ICSP-5     RST
   SPI SS      SDA(SS)      10            53        D10        10               10
   SPI MOSI    MOSI         11 / ICSP-4   51        D11        ICSP-4           16
   SPI MISO    MISO         12 / ICSP-1   50        D12        ICSP-1           14
   SPI SCK     SCK          13 / ICSP-3   52        D13        ICSP-3           15
*/

//Please Comment ALL SERIAL instructions when put to use, saving resource//
#include <EEPROM.h>     // We are going to read and write PICC's UIDs from/to EEPROM
#include <SPI.h>        // RC522 Module uses SPI protocol
#include <MFRC522.h>  // Library for Mifare RC522 Devices
#include <Servo.h>
Servo myservo;

/*
  For visualizing whats going on hardware we need some leds and to control door lock a relay and a wipe button
  (or some other hardware) Used common anode led,digitalWriting HIGH turns OFF led Mind that if you are going
  to use common cathode led or just seperate leds, simply comment out #define COMMON_ANODE,
*/

//#define COMMON_ANODE

#ifdef COMMON_ANODE
#define LED_ON LOW
#define LED_OFF HIGH
#else
#define LED_ON HIGH
#define LED_OFF LOW
#endif 

#define redLed 7    // Set Led Pins
#define greenLed 6
#define blueLed 5
int Sound = 2;
#define wipeB 3     // Button pin for WipeMode
#define detect 1    //switch detector

bool programMode = false;  // initialize programming mode to false

uint8_t successRead;    // Variable integer to keep if we have Successful Read from Reader

byte storedCard[4];   // Stores an ID read from EEPROM
byte readCard[4];   // Stores scanned ID read from RFID Module
byte masterCard[4];   // Stores master card's ID read from EEPROM

#define SS_PIN 10
#define RST_PIN 8
MFRC522 mfrc522(SS_PIN, RST_PIN);

void Alarm(int mode,int Sound){
  if(mode == 1){//initialize sound
    tone(Sound,261);
    delay(200);
    noTone(Sound);
    delay(30);
    tone(Sound,310);
    delay(200);
    noTone(Sound);
    delay(30);
    tone(Sound,350);
    delay(200);   
    noTone(Sound);
  }
  if(mode == 2){//correct sound
    tone(Sound,1500);
    delay(50);
    noTone(Sound);
    delay(25);
    tone(Sound,1500);
    delay(50);
    noTone(Sound);
  }
  if(mode == 3){//incorrect sound(alarm sound)
    for(int j = 1;j<=125;j++){
    for(int i = 400;i <1600+10*j;i+=16*j ){
      tone(Sound,i);
      delay(20);
    }
    for(int i = 1600+10*j;i >400;i-=50*j ){
      tone(Sound,i);
      delay(20);
    }
    }
    for(int i = 2850;i >120;i-=5+i/120 ){
      tone(Sound,i);
      delay(20);
    }
    noTone(Sound);
  }
  if(mode == 4){//programing on
    tone(Sound,1500);
    delay(100);
    noTone(Sound);
    delay(30);
    tone(Sound,1500);
    delay(100);
    noTone(Sound);
    delay(30);
    tone(Sound,1500);
    delay(100);   
    noTone(Sound);
  }
  if(mode == 5){//programing off
    tone(Sound,750);
    delay(100);
    noTone(Sound);
    delay(30);
    tone(Sound,500);
    delay(100);
    noTone(Sound);
    delay(30);
    tone(Sound,350);
    delay(100);   
    noTone(Sound);    
  }
  if(mode ==6){//add correctly
    tone(Sound,1500);
    delay(200);
    noTone(Sound);
    delay(30);
    tone(Sound,1500);
    delay(80);
    noTone(Sound);
    delay(30);
    tone(Sound,1500);
    delay(200);   
    noTone(Sound);
    delay(30);
    tone(Sound,1500);
    delay(200);   
    noTone(Sound);    
  }
  if(mode == 7){//remove correctly
    tone(Sound,400);
    delay(200);
    noTone(Sound);
    delay(30);
    tone(Sound,400);
    delay(80);
    noTone(Sound);
    delay(30);
    tone(Sound,400);
    delay(200);   
    noTone(Sound);
    delay(30);
    tone(Sound,400);
    delay(200);   
    noTone(Sound);    
  }
  if(mode == 8){//error
    noTone(Sound);
    tone(Sound,400);
    delay(200);
    noTone(Sound);
    delay(30);
    tone(Sound,400);
    delay(80);   
    noTone(Sound);  
    delay(30); 
  }
}

/////////////////////////////////////////  Access Granted    ///////////////////////////////////
/*
void granted ( uint16_t setDelay) {
  digitalWrite(blueLed, LED_OFF);   // Turn off blue LED
  digitalWrite(redLed, LED_OFF);  // Turn off red LED
  digitalWrite(greenLed, LED_ON);   // Turn on green LED
  digitalWrite(relay, LOW);     // Unlock door!
  delay(setDelay);          // Hold door lock open for given seconds
  digitalWrite(relay, HIGH);    // Relock door
  delay(1000);            // Hold green LED on for a second
}

///////////////////////////////////////// Access Denied  ///////////////////////////////////
void denied() {
  digitalWrite(greenLed, LED_OFF);  // Make sure green LED is off
  digitalWrite(blueLed, LED_OFF);   // Make sure blue LED is off
  digitalWrite(redLed, LED_ON);   // Turn on red LED
  delay(1000);
}
*/

///////////////////////////////////////// Get PICC's UID ///////////////////////////////////
uint8_t getID() {
  // Getting ready for Reading PICCs
  if ( ! mfrc522.PICC_IsNewCardPresent()) { //If a new PICC placed to RFID reader continue
    return 0;
  }
  if ( ! mfrc522.PICC_ReadCardSerial()) {   //Since a PICC placed get Serial and continue
    return 0;
  }
  // There are Mifare PICCs which have 4 byte or 7 byte UID care if you use 7 byte PICC
  // I think we should assume every PICC as they have 4 byte UID
  // Until we support 7 byte PICCs
  //Serial.println(F("Scanned PICC's UID:"));
  for ( uint8_t i = 0; i < 4; i++) {  //
    readCard[i] = mfrc522.uid.uidByte[i];
    //Serial.print(readCard[i], HEX);
  }
  //Serial.println("");
  mfrc522.PICC_HaltA(); // Stop reading
  return 1;
}

void ShowReaderDetails() {
  // Get the MFRC522 software version
  byte v = mfrc522.PCD_ReadRegister(mfrc522.VersionReg);
  //Serial.print(F("MFRC522 Software Version: 0x"));
  //Serial.print(v, HEX);
  if (v == 0x91)
    Serial.print(F(" = v1.0"));
  else if (v == 0x92)
    Serial.print(F(" = v2.0"));
  else
    Serial.print(F(" (unknown),probably a chinese clone?"));
  Serial.println("");
  // When 0x00 or 0xFF is returned, communication probably failed
  if ((v == 0x00) || (v == 0xFF)) {
    //Serial.println(F("WARNING: Communication failure, is the MFRC522 properly connected?"));
    //Serial.println(F("SYSTEM HALTED: Check connections."));
    // Visualize system is halted
    digitalWrite(greenLed, LED_OFF);  // Make sure green LED is off
    digitalWrite(blueLed, LED_OFF);   // Make sure blue LED is off
    digitalWrite(redLed, LED_ON);   // Turn on red LED
    while (true); // do not go further
  }
}

///////////////////////////////////////// Cycle Leds (Program Mode) ///////////////////////////////////
void cycleLeds() {
  digitalWrite(redLed, LED_OFF);  // Make sure red LED is off
  digitalWrite(greenLed, LED_ON);   // Make sure green LED is on
  digitalWrite(blueLed, LED_OFF);   // Make sure blue LED is off
  delay(200);
  digitalWrite(redLed, LED_OFF);  // Make sure red LED is off
  digitalWrite(greenLed, LED_OFF);  // Make sure green LED is off
  digitalWrite(blueLed, LED_ON);  // Make sure blue LED is on
  delay(200);
  digitalWrite(redLed, LED_ON);   // Make sure red LED is on
  digitalWrite(greenLed, LED_OFF);  // Make sure green LED is off
  digitalWrite(blueLed, LED_OFF);   // Make sure blue LED is off
  delay(200);
}

//////////////////////////////////////// Normal Mode Led  ///////////////////////////////////
void normalModeOn () {
  digitalWrite(blueLed, LED_ON);  // Blue LED ON and ready to read card
  digitalWrite(redLed, LED_OFF);  // Make sure Red LED is off
  digitalWrite(greenLed, LED_OFF);  // Make sure Green LED is off
}

//////////////////////////////////////// Read an ID from EEPROM //////////////////////////////
void readID( uint8_t number ) {
  uint8_t start = (number * 4 ) + 2;    // Figure out starting position
  for ( uint8_t i = 0; i < 4; i++ ) {     // Loop 4 times to get the 4 Bytes
    storedCard[i] = EEPROM.read(start + i);   // Assign values read from EEPROM to array
  }
}

///////////////////////////////////////// Add ID to EEPROM   ///////////////////////////////////
void writeID( byte a[] ) {
  if ( !findID( a ) ) {     // Before we write to the EEPROM, check to see if we have seen this card before!
    uint8_t num = EEPROM.read(0);     // Get the numer of used spaces, position 0 stores the number of ID cards
    uint8_t start = ( num * 4 ) + 6;  // Figure out where the next slot starts
    num++;                // Increment the counter by one
    EEPROM.write( 0, num );     // Write the new count to the counter
    for ( uint8_t j = 0; j < 4; j++ ) {   // Loop 4 times
      EEPROM.write( start + j, a[j] );  // Write the array values to EEPROM in the right position
    }
    successWrite();
    //Serial.println(F("Succesfully added ID record to EEPROM"));
  }
  else {
    failedWrite();
    //Serial.println(F("Failed! There is something wrong with ID or bad EEPROM"));
  }
}

///////////////////////////////////////// Remove ID from EEPROM   ///////////////////////////////////
void deleteID( byte a[] ) {
  if ( !findID( a ) ) {     // Before we delete from the EEPROM, check to see if we have this card!
    failedWrite();      // If not
    //Serial.println(F("Failed! There is something wrong with ID or bad EEPROM"));
  }
  else {
    uint8_t num = EEPROM.read(0);   // Get the numer of used spaces, position 0 stores the number of ID cards
    uint8_t slot;       // Figure out the slot number of the card
    uint8_t start;      // = ( num * 4 ) + 6; // Figure out where the next slot starts
    uint8_t looping;    // The number of times the loop repeats
    uint8_t j;
    uint8_t count = EEPROM.read(0); // Read the first Byte of EEPROM that stores number of cards
    slot = findIDSLOT( a );   // Figure out the slot number of the card to delete
    start = (slot * 4) + 2;
    looping = ((num - slot) * 4);
    num--;      // Decrement the counter by one
    EEPROM.write( 0, num );   // Write the new count to the counter
    for ( j = 0; j < looping; j++ ) {         // Loop the card shift times
      EEPROM.write( start + j, EEPROM.read(start + 4 + j));   // Shift the array values to 4 places earlier in the EEPROM
    }
    for ( uint8_t k = 0; k < 4; k++ ) {         // Shifting loop
      EEPROM.write( start + j + k, 0);
    }
    successDelete();
    //Serial.println(F("Succesfully removed ID record from EEPROM"));
  }
}

///////////////////////////////////////// Check Bytes   ///////////////////////////////////
bool checkTwo ( byte a[], byte b[] ) {   
  for ( uint8_t k = 0; k < 4; k++ ) {   // Loop 4 times
    if ( a[k] != b[k] ) {     // IF a != b then false, because: one fails, all fail
       return false;
    }
  }
  return true;  
}

///////////////////////////////////////// Find Slot   ///////////////////////////////////
uint8_t findIDSLOT( byte find[] ) {
  uint8_t count = EEPROM.read(0);       // Read the first Byte of EEPROM that
  for ( uint8_t i = 1; i <= count; i++ ) {    // Loop once for each EEPROM entry
    readID(i);                // Read an ID from EEPROM, it is stored in storedCard[4]
    if ( checkTwo( find, storedCard ) ) {   // Check to see if the storedCard read from EEPROM
      // is the same as the find[] ID card passed
      return i;         // The slot number of the card
    }
  }
}

///////////////////////////////////////// Find ID From EEPROM   ///////////////////////////////////
bool findID( byte find[] ) {
  uint8_t count = EEPROM.read(0);     // Read the first Byte of EEPROM that
  for ( uint8_t i = 1; i < count; i++ ) {    // Loop once for each EEPROM entry
    readID(i);          // Read an ID from EEPROM, it is stored in storedCard[4]
    if ( checkTwo( find, storedCard ) ) {   // Check to see if the storedCard read from EEPROM
      return true;
    }
    else {    // If not, return false
    }
  }
  return false;
}

///////////////////////////////////////// Write Success to EEPROM   ///////////////////////////////////
// Flashes the green LED 3 times to indicate a successful write to EEPROM
void successWrite() {
  digitalWrite(blueLed, LED_OFF);   // Make sure blue LED is off
  digitalWrite(redLed, LED_OFF);  // Make sure red LED is off
  digitalWrite(greenLed, LED_OFF);  // Make sure green LED is on
  delay(200);
  digitalWrite(greenLed, LED_ON);   // Make sure green LED is on
  delay(200);
  digitalWrite(greenLed, LED_OFF);  // Make sure green LED is off
  delay(200);
  digitalWrite(greenLed, LED_ON);   // Make sure green LED is on
  delay(200);
  digitalWrite(greenLed, LED_OFF);  // Make sure green LED is off
  delay(200);
  digitalWrite(greenLed, LED_ON);   // Make sure green LED is on
  delay(200);
}

///////////////////////////////////////// Write Failed to EEPROM   ///////////////////////////////////
// Flashes the red LED 3 times to indicate a failed write to EEPROM
void failedWrite() {
  Alarm(8,Sound);
  digitalWrite(blueLed, LED_OFF);   // Make sure blue LED is off
  digitalWrite(redLed, LED_OFF);  // Make sure red LED is off
  digitalWrite(greenLed, LED_OFF);  // Make sure green LED is off
  delay(200);
  digitalWrite(redLed, LED_ON);   // Make sure red LED is on
  delay(200);
  digitalWrite(redLed, LED_OFF);  // Make sure red LED is off
  delay(200);
  digitalWrite(redLed, LED_ON);   // Make sure red LED is on
  delay(200);
  digitalWrite(redLed, LED_OFF);  // Make sure red LED is off
  delay(200);
  digitalWrite(redLed, LED_ON);   // Make sure red LED is on
  delay(200);
}

///////////////////////////////////////// Success Remove UID From EEPROM  ///////////////////////////////////
// Flashes the blue LED 3 times to indicate a success delete to EEPROM
void successDelete() {
  digitalWrite(blueLed, LED_OFF);   // Make sure blue LED is off
  digitalWrite(redLed, LED_OFF);  // Make sure red LED is off
  digitalWrite(greenLed, LED_OFF);  // Make sure green LED is off
  delay(200);
  digitalWrite(blueLed, LED_ON);  // Make sure blue LED is on
  delay(200);
  digitalWrite(blueLed, LED_OFF);   // Make sure blue LED is off
  delay(200);
  digitalWrite(blueLed, LED_ON);  // Make sure blue LED is on
  delay(200);
  digitalWrite(blueLed, LED_OFF);   // Make sure blue LED is off
  delay(200);
  digitalWrite(blueLed, LED_ON);  // Make sure blue LED is on
  delay(200);
}

////////////////////// Check readCard IF is masterCard   ///////////////////////////////////
// Check to see if the ID passed is the master programing card
bool isMaster( byte test[] ) {
	return checkTwo(test, masterCard);
}

bool monitorWipeButton(uint32_t interval) {
  uint32_t now = (uint32_t)millis();
  while ((uint32_t)millis() - now < interval)  {
    // check on every half a second
    if (((uint32_t)millis() % 500) == 0) {
      if (digitalRead(wipeB) != LOW)
        return false;
    }
  }
  return true;
}

///////////////////////////////////////// Setup ///////////////////////////////////
void setup() {
  //pinMode(indoor,OUTPUT);
  pinMode(detect,INPUT);
  Alarm(1,Sound);
  myservo.attach(9);
  myservo.write(0); //0 degree
  //Arduino Pin Configuration
  pinMode(redLed, OUTPUT);
  pinMode(greenLed, OUTPUT);
  pinMode(blueLed, OUTPUT);
  pinMode(wipeB, INPUT_PULLUP);   // Enable pin's pull up resistor
  //Be careful how relay circuit behave on while resetting or power-cycling your Arduino
  digitalWrite(redLed, LED_OFF);  // Make sure led is off
  digitalWrite(greenLed, LED_OFF);  // Make sure led is off
  digitalWrite(blueLed, LED_OFF); // Make sure led is off

  //Protocol Configuration
  //Serial.begin(9600);  // Initialize serial communications with PC
  SPI.begin();           // MFRC522 Hardware uses SPI protocol
  mfrc522.PCD_Init();    // Initialize MFRC522 Hardware

  //If you set Antenna Gain to Max it will increase reading distance
  mfrc522.PCD_SetAntennaGain(mfrc522.RxGain_max);
  ShowReaderDetails();  // Show details of PCD - MFRC522 Card Reader details
  // Check if master card defined, if not let user choose a master card
  // This also useful to just redefine the Master Card
  // You can keep other EEPROM records just write other than 143 to EEPROM address 1
  // EEPROM address 1 should hold magical number which is '143'
  if (EEPROM.read(1) != 143) {
    //Serial.println(F("No Master Card Defined"));
    //Serial.println(F("Scan A PICC to Define as Master Card"));
    do {
      successRead = getID();            // sets successRead to 1 when we get read from reader otherwise 0
      digitalWrite(blueLed, LED_ON);    // Visualize Master Card need to be defined
      delay(200);
      digitalWrite(blueLed, LED_OFF);
      delay(200);
    }
    while (!successRead);                  // Program will not go further while you not get a successful read
    for ( uint8_t j = 0; j < 4; j++ ) {        // Loop 4 times
      EEPROM.write( 2 + j, readCard[j] );  // Write scanned PICC's UID to EEPROM, start from address 3
    }
    EEPROM.write(1, 143);                  // Write to EEPROM we defined Master Card.
    //Serial.println(F("Master Card Defined"));
  }
  //Serial.println(F("-------------------"));
  //Serial.println(F("Master Card's UID"));
  for ( uint8_t i = 0; i < 4; i++ ) {          // Read Master Card's UID from EEPROM
    masterCard[i] = EEPROM.read(2 + i);    // Write it to masterCard
    //Serial.print(masterCard[i], HEX);
  }
  //Serial.println("");
  //Serial.println(F("-------------------"));
  //Serial.println(F("Everything is ready"));
  //Serial.println(F("Waiting PICCs to be scanned"));
  cycleLeds();    // Everything ready lets give user some feedback by cycling leds
}

///////////////////////////////////////// Main Loop ///////////////////////////////////
void loop () {
  do {
    //EEPROM.clear();
    successRead = getID();  // sets successRead to 1 when we get read from reader otherwise 0
    if(!digitalRead(detect) and !programMode){
    digitalWrite(blueLed, LED_ON);
    Gate();
    digitalWrite(blueLed, LED_OFF);   // Turn off blue LED
    digitalWrite(redLed, LED_OFF);  // Turn off red LED
    digitalWrite(greenLed, LED_ON);   // Turn on green LED
      }
    // When device is in use if wipe button pressed for 10 seconds initialize Master Card wiping
    if (digitalRead(detect) == LOW) { // Check if button is pressed
      // Visualize normal operation is iterrupted by pressing wipe button Red is like more Warning to user
      digitalWrite(redLed, LED_ON);  // Make sure led is off
      digitalWrite(greenLed, LED_OFF);  // Make sure led is off
      digitalWrite(blueLed, LED_OFF); // Make sure led is off
      // Give some feedback
      //Serial.println(F("Wipe Button Pressed"));
      //Serial.println(F("Master Card will be Erased! in 10 seconds"));
      bool buttonState = monitorWipeButton(10000); // Give user enough time to cancel operation
      if (buttonState == true && digitalRead(detect) == LOW) {    // If button still be pressed, wipe EEPROM
        EEPROM.write(1, 0);                  // Reset Magic Number.
        //Serial.println(F("Master Card Erased from device"));
        //Serial.println(F("Please reset to re-program Master Card"));
        while (1);
      }
      //Serial.println(F("Master Card Erase Cancelled"));
    }
    if (programMode) {
      cycleLeds();              // Program Mode cycles through Red Green Blue waiting to read a new card
        //Wipe Code - If the Button (wipeB) Pressed while setup run (powered on) it wipes EEPROM
      if (!digitalRead(detect)) {  // when button pressed pin should get low, button connected to ground
        digitalWrite(redLed, LED_ON); // Red Led stays on to inform user we are going to wipe
        //Serial.println(F("Wipe Button Pressed"));
        //Serial.println(F("You have 10 seconds to Cancel"));
        //Serial.println(F("This will be remove all records and cannot be undone"));
        bool buttonState = monitorWipeButton(10000); // Give user enough time to cancel operation
        if (buttonState == true && digitalRead(detect)) {    // If button still be pressed, wipe EEPROM
          //Serial.println(F("Starting Wiping EEPROM"));
          for (uint16_t x = 0; x < EEPROM.length(); x = x + 1) {    //Loop end of EEPROM address
            if (EEPROM.read(x) == 0) {              //If EEPROM address 0
            // do nothing, already clear, go to the next address in order to save time and reduce writes to EEPROM
            }
            else {
              EEPROM.write(x, 0);       // if not write 0 to clear, it takes 3.3mS
            }
          }
          //Serial.println(F("EEPROM Successfully Wiped"));
          digitalWrite(redLed, LED_OFF);  // visualize a successful wipe
          delay(200);
          digitalWrite(redLed, LED_ON);
          delay(200);
          digitalWrite(redLed, LED_OFF);
          delay(200);
          digitalWrite(redLed, LED_ON);
          delay(200);
          digitalWrite(redLed, LED_OFF);
    }
    else {
      //Serial.println(F("Wiping Cancelled")); // Show some feedback that the wipe button did not pressed for 15 seconds
      digitalWrite(redLed, LED_OFF);
    }
  }
    }
    else {
      normalModeOn();     // Normal mode, blue Power LED is on, all others are off
      }
  }
  while (!successRead);   //the program will not go further while you are not getting a successful read
  if (programMode) {
    if ( isMaster(readCard) ) { //When in program mode check First If master card scanned again to exit program mode
      //Serial.println(F("Master Card Scanned"));
      Alarm(5,Sound);
      //Serial.println(F("Exiting Program Mode"));
      //Serial.println(F("-----------------------------"));
      programMode = false;
      return;
    }
    else {
      if ( findID(readCard) ) { // If scanned card is known delete it
        //Serial.println(F("I know this PICC, removing..."));
        deleteID(readCard);
        Alarm(7,Sound);
        //Serial.println("-----------------------------");
        //Serial.println(F("Scan a PICC to ADD or REMOVE to EEPROM"));
      }
      else {                    // If scanned card is not known add it
        //Serial.println(F("I do not know this PICC, adding..."));
        writeID(readCard);
        Alarm(6,Sound);
        //Serial.println(F("-----------------------------"));
        //Serial.println(F("Scan a PICC to ADD or REMOVE to EEPROM"));
      }
    }
  }
  else {
    if ( isMaster(readCard)) {    // If scanned card's ID matches Master Card's ID - enter program mode
      programMode = true;
      //Serial.println(F("Hello Master - Program Mode"));
      uint8_t count = EEPROM.read(0);   // Read the first Byte of EEPROM that
      Alarm(4,Sound);
      //Serial.print(F("I have "));     // stores the number of ID's in EEPROM
      //Serial.print(count);
      //Serial.print(F(" record(s) on EEPROM"));
      //Serial.println("");
      //Serial.println(F("Scan a PICC to ADD or REMOVE to EEPROM"));
      //Serial.println(F("Scan Master Card again to Exit Program Mode"));
      //Serial.println(F("-----------------------------"));
    }
    else {
      if ( findID(readCard) ) { // If not, see if the card is in the EEPROM
        Gate();
        digitalWrite(blueLed, LED_OFF);   // Turn off blue LED
        digitalWrite(redLed, LED_OFF);  // Turn off red LED
        digitalWrite(greenLed, LED_ON);   // Turn on green LED
        //Serial.println(F("Welcome, You shall pass"));
      }
      else {      // If not, show that the ID was not valid
        digitalWrite(greenLed, LED_OFF);  // Make sure green LED is off
        digitalWrite(blueLed, LED_OFF);   // Make sure blue LED is off
        digitalWrite(redLed, LED_ON);   // Turn on red LED
        //Serial.println(F("You shall not pass"));
        Alarm(3,Sound);
        digitalWrite(redLed, LED_OFF);
      }
    }
  }
}
void Gate(){
  Alarm(2,Sound);
  digitalWrite(blueLed, LED_ON);
  myservo.write(0); //0 degree
  myservo.write(57);
  delay(900);
  myservo.write(0);
  }