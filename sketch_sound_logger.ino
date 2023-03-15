const int sampleWindow = 50;                              // Sample window width in mS (50 mS = 20Hz)
unsigned int sample;
 
#define SENSOR_PIN A0
 
void setup ()  
{   
  pinMode (SENSOR_PIN, INPUT); // Set the signal pin as input  
  Serial.begin(9600);
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
   delay(200); 

}