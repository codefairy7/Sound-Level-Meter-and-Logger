#include <UTFTGLUE.h>
#include <SoftwareSerial.h>

UTFTGLUE myGLCD(0, A2, A1, A3, A4, A0);
SoftwareSerial USB(51, 53);

const int sampleWindow = 50;                              // Sample window width in mS (50 mS = 20Hz)
unsigned int sample;

byte USB_Byte;
int timeOut = 2000;
 
#define SENSOR_PIN A0
 
void setup ()  
{   
  pinMode (SENSOR_PIN, INPUT); // Set the signal pin as input  
  Serial.begin(9600);
  USB.begin(9600);
  myGLCD.InitLCD();
  myGLCD.clrScr();
  myGLCD.invertDisplay(1);
  myGLCD.setFont(BigFont);
  myGLCD.setColor(0, 191, 99);
  myGLCD.fillRect(0, 0, 480, 30);
  myGLCD.setColor(255, 255, 255);
  myGLCD.setBackColor(0, 191, 99); 
  myGLCD.print("SOUND LEVEL METER", CENTER, 10);
  myGLCD.setColor(0, 74, 173);
  myGLCD.fillRect(0, 30, 480, 320);

  // myGLCD.setFont(SmallFont);
  myGLCD.setColor(255, 255, 255);
  myGLCD.setBackColor(0, 74, 173);
  myGLCD.print("SPL      :", 60, 70);
  myGLCD.print("Min. SPL : ", 60, 110);
  myGLCD.print("Max. SPL : ", 60, 150);
  myGLCD.print("LEQ      : ", 60, 190);
  myGLCD.print("SEL      : ", 60, 230);
}  
   
void loop ()  
{ 
   unsigned long startMillis= millis();                   // Start of sample window
   float peakToPeak = 0;                                  // peak-to-peak level
 
   unsigned int signalMax = 0;                            //minimum value
   unsigned int signalMin = 1024;                         //maximum value
 
                                                          // collect data for 50 mS
   while (millis() - startMillis < sampleWindow)
   {
      sample = analogRead(SENSOR_PIN);                    //get reading from microphone
      if (sample < 1024)                                  // toss out spurious readings
      {
         if (sample > signalMax)
         {
            signalMax = sample;                           // save just the max levels
         }
         else if (sample < signalMin)
         {
            signalMin = sample;                           // save just the min levels
         }
      }
   }
 
   peakToPeak = signalMax - signalMin;      
//    map(value, fromLow, fromHigh, toLow, toHigh)

// Parameters
// value: the number to map.
// fromLow: the lower bound of the value’s current range.
// fromHigh: the upper bound of the value’s current range.
// toLow: the lower bound of the value’s target range.
// toHigh: the upper bound of the value’s target range.
                 // max - min = peak-peak amplitude
   //int db = map(peakToPeak,20,900,49.5,90);     
   int db = map(peakToPeak,20,800,49.5,90);          //calibrate for deciBels
    Serial.print(db);
    Serial.println(" dB");
  myGLCD.print(String(db), 250, 70);
  myGLCD.print(String(db), 250, 110);
  myGLCD.print(String(db), 250, 150);
  myGLCD.print(String(db), 250, 190);
  myGLCD.print(String(db), 250, 230);

   delay(2000); 
}

void createFile() {
  set_USB_Mode(0x06);
  diskConnectionStatus();
  USBdiskMount();
  setFileName("20-03-23.txt");
  if(fileCreate()){
    fileWrite("writing some data");
  } else {
    Serial.println("File could not be created. Maybe it already exists.");
  }
}

boolean fileCreate() {
  boolean createdFile = false;
  USB.write(0x57);
  USB.write(0xAB);
  USB.write(0x34);
  if(waitForResponse("Creating file.")){
    if(getResponseFromUSB() == 0x14) {
      createdFile = true;
    }
  }
  return(createdFile);
}
void set_USB_Mode(byte value) {
  USB.write(0x57);
  USB.write(0xAB);
  USB.write(0x15);

  USB.write(value);

  delay(20);

  if(USB.available()) {
    USB_Byte  = USB.read();
    if(USB_Byte == 0x51) {
      Serial.println("set_USB_Mode command acknowledged");
      USB_Byte = USB.read();

      //check to see if the USB stick is connected or not
      if(USB_Byte == 0x15){
        Serial.print("USB is present.");
      } else {
        Serial.print("USB not present. Error code:");
        Serial.print(USB_Byte, HEX);
        Serial.println("H");
      }
    } else {
      Serial.print("CH376 error! Error code: ");
      Serial.print(USB_Byte, HEX);
      Serial.println("H");
    }
  }
  delay(20);
}

void resetAll() {
  USB.write(0x57);
  USB.write(0xAB);
  USB.write(0x05);
  Serial.println("The CH376S module has been reset !");
  delay(200);
}

void appendFile(String fileName, String data) {
  resetAll();
  set_USB_Mode(0x06);
  diskConnectionStatus();
  USBdiskMount();
  setFileName(fileName);
  fileOpen();
  filePointer(false);
  fileWrite(data);
  fileClose(0x01);
}

void setFileName(String fileName) {
  Serial.print("Settng filename to : ");
  Serial.println(fileName);
  USB.write(0x57);
  USB.write(0xAB);
  USB.write(0x2F);
  USB.write(0x2F);
  USB.print(fileName);
  USB.write((byte)0x00);
  delay(20);
}

void diskConnectionStatus() {
  Serial.println("Checking USB disk connection status");
  USB.write(0x57);
  USB.write(0xAB);
  USB.write(0x30);

  if(waitForResponse("Connecting to USB disk")) {
    if(getResponseFromUSB() == 0x14){
        Serial.println("Connection to USB OK");
    } else {
      Serial.print("Connection to USB - FAILED.");
    }
  }
 }

 void USBdiskMount() {
   Serial.println("Mounting USB disk");
   USB.write(0x57);
   USB.write(0xAB);
   USB.write(0x31);

   if(waitForResponse("mounting USB disk")) {
     if(getResponseFromUSB() == 0x14) {
       Serial.println("USB Mounted - OK");
     } else {
       Serial.print("Filaed to Mount USB disk");
     }
   }
 }

 void fileOpen() {
   Serial.println("Opening file.");
   USB.write(0x57);
   USB.write(0xAB);
   USB.write(0x32);
   if(waitForResponse("file Open")) {
     if(getResponseFromUSB() == 0x14) {
       Serial.println("File opened successfully.");
     } else {
       Serial.print("Failed to open file.");
     }
   }
 }

void fileWrite(String data) {
  Serial.println("Writing to file:");
  byte dataLength = (byte) data.length();
  Serial.println(data);
  Serial.print("Data Length: ");
  Serial.println(dataLength);
  delay(100);

  USB.write(0x57);
  USB.write(0xAB);
  USB.write(0x3C);
  USB.write((byte) dataLength);
  USB.write((byte) 0x00);
  if(waitForResponse("Setting data Length")) {
    if(getResponseFromUSB() == 0x1E) {
      USB.write(0x57);
      USB.write(0xAB);
      USB.write(0x2D);
      USB.print(data);

      if(waitForResponse("Writing data to file")) {

      }
      Serial.print("Write code (normally FF and 14): ");
      Serial.print(USB.read(), HEX);
      Serial.print(",");
      USB.write(0x57);
      USB.write(0xAB);
      USB.write(0x3D);
      if (waitForResponse("Updating file size")) {

      }
      Serial.println(USB.read(), HEX);
    }
  }
}

boolean waitForResponse(String errorMsg){
  boolean bytesAvailable = true;
  int counter = 0;
  while(!USB.available()){
    delay(1);
    counter++;
    if(counter > timeOut){
      Serial.print("Timeout waiting for resposnse: Error while: ");
      Serial.println(errorMsg);
      bytesAvailable = false;
      break;
    }
  }
  delay(1);
  return(bytesAvailable);
}

byte getResponseFromUSB(){
  byte response = byte(0x00);
  if (USB.available()){
    response = USB.read();
  }
  return(response);
}

void filePointer(boolean fileBeginning){
  USB.write(0x57);
  USB.write(0xAB);
  USB.write(0x39);
  if(fileBeginning){
    USB.write((byte)0x00);
    USB.write((byte)0x00);
    USB.write((byte)0x00);
    USB.write((byte)0x00);
  } else {
    USB.write((byte)0xFF);
    USB.write((byte)0xFF);
    USB.write((byte)0xFF);
    USB.write((byte)0xFF);
  }
  if(waitForResponse("Setting file pointer")) {
    if(getResponseFromUSB() == 0x14){
      Serial.println("Pointer successfully applied.");
    }
  }
}

void fileClose(byte closeCmd){
  Serial.println("Closing file:");
  USB.write(0x57);
  USB.write(0xAB);
  USB.write(0x36);
  USB.write((byte)closeCmd);

  if(waitForResponse("closing file")) {
    byte resp = getResponseFromUSB();
    if(resp == 0x14) {
      Serial.println("File closed successfully.");
    } else {
      Serial.print("Failed to close the door. Error code: ");
      Serial.println(resp, HEX);
    }
  }
}



















