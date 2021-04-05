/*
    Bluetooh Basic: LED ON OFF - Avishkar
    Coder - Mayoogh Girish
    Website - http://bit.do/Avishkar
    Download the App :
    This program lets you to control a LED on pin 13 of arduino using a bluetooth module
*/

#include <OneWire.h>
#include <DallasTemperature.h>
#include <stdlib.h>

// Data pin ori ONWIRE
#define ONE_WIRE_BUS 2
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);
int g_numberOfDevices;
DeviceAddress g_tempDeviceAddress;



int ledState = HIGH;             // ledState used to set the LED
const int ledPin_12 = 12;      // select the pin for the LED
unsigned long prevLEDMillis = 0;        // will store last time LED was updated
unsigned long prevLogMillis = 0;        // will store last time LED was updated
unsigned long prevReadSensorsMilis = 0;
unsigned long prevLogMilis = 0;
float g_temp[3];
// xx.x;
#define U_SIZE 5
#define NUM_SAMPLE 60
#define L_SIZE (NUM_SAMPLE * U_SIZE)
#define LOG_TIMER 2000
#define TEMP_TIMER 2000

char* c_log0 = NULL;
char* c_log1 = NULL;
char* c_log2 = NULL;
int g_log = 0;
// ---------------
const char numChars = 32;
char recivedChars[numChars];
boolean serialNewData  = false;
void recvWithEndMarker(void);

int change(int  state);
void shift_buffer( float buf[], const int b_size, const int margin);
void getTempreatures(float measureC[], int len = 3);
void initSensors();
void setup()
{
  Serial.begin(9600);         //Sets the data rate in bits per second (baud) for serial data transmission
  pinMode(ledPin_12, OUTPUT);
  for (int i = 0; i < 3; i++)
    g_temp[i] = 0.0;
  char sample[] = {'0','0','.','0',';','\0'};
  c_log0 = (char*)malloc(L_SIZE + 1);
  c_log1 = (char*)malloc(L_SIZE + 1);
  c_log2 = (char*)malloc(L_SIZE + 1);
  c_log0[0] = '\0';
  c_log1[0] = '\0';
  c_log2[0] = '\0';
  for (int i = 0; i< NUM_SAMPLE; i++)
  {
    strcat(c_log0, sample);
    strcat(c_log1, sample);
    strcat(c_log2, sample);
  }
  initSensors();
}
void loop()
{
  recvWithEndMarker();
  //Serial.println(g_inputByte);
  if (((strcmp(recivedChars,"one") == 0) && serialNewData)){
    // the initali screen send a current temperatures
    Serial.print( g_temp[0]);
    Serial.print(';');
    Serial.print( g_temp[1]);
    Serial.print(';');
    Serial.print( g_temp[2]);
    Serial.println(';');
    serialNewData  = false;
    
  }
  if (((strcmp(recivedChars,"c_log0") == 0) && serialNewData))
  {
    Serial.println(c_log0);
    serialNewData  = false;
  }
  if (((strcmp(recivedChars,"c_log1") == 0) && serialNewData))
  {
    Serial.println(c_log1);
    serialNewData  = false;
  }
  if (((strcmp(recivedChars,"c_log2") == 0) && serialNewData))
  {
    Serial.println(c_log2);
    serialNewData  = false;
  }   
  showNewData();
  
  unsigned long currentMillis = millis();
  if (currentMillis - prevLEDMillis >= 2000)
  {
    // save the last time you blinked the LED
    prevLEDMillis = currentMillis;
    // if the LED is off turn it on and vice-versa:
    ledState = change(ledState);
  }
  if (currentMillis - prevLogMillis >= LOG_TIMER)
  {
    prevLogMillis = currentMillis;
    
    trim_strings(c_log0,  1);
    trim_strings(c_log1,  1);
    trim_strings(c_log2,  1);
    char buf[6];
    // char *  dtostrf (double __val, signed char __width, unsigned char __prec, char *__s)
    dtostrf(g_temp[0], 4, 1, buf);
    c_log0 = strcat(c_log0, buf);
    c_log0 = strcat(c_log0, ";");
    dtostrf(g_temp[1], 4, 1, buf);
    c_log1 = strcat(c_log1, buf);
    c_log1 = strcat(c_log1, ";");
    dtostrf(g_temp[1], 4, 1, buf);
    c_log2 = strcat(c_log2, buf);
    c_log2 = strcat(c_log2, ";");
    //Serial.println(c_log0);
  
  }
  if (currentMillis - prevReadSensorsMilis >= TEMP_TIMER)
  {
    prevReadSensorsMilis = currentMillis;
    //getTempreatures(g_temp, 3);
    g_temp[0] = g_temp[0] + 0.2;
    if (g_temp[0] >= 99.0) g_temp[0] = 1.0;
    g_temp[1] = g_temp[1] + 0.9;
    if (g_temp[1] >= 99.0) g_temp[1] = 1.0;
    g_temp[2] = g_temp[2] + 2.2;
    if (g_temp[2] >= 99.0) g_temp[2] = 1.0;
  }
  digitalWrite(ledPin_12, ledState);

}
////////////////////////////////////////
// util functions

int change(int  state)
{ if (state == LOW)
  {
    return HIGH;
  } else
  {
    return LOW;
  }
}
void shift_buffer( float buf[], const int b_size, const int margin)
{
  float * tmp1 = (float*) new float[b_size];
  memset(tmp1, 0, sizeof(tmp1));
  memcpy(tmp1, &buf[margin], sizeof(float) * (b_size - margin) );
  memcpy(buf, &tmp1, sizeof(tmp1));
  delete[] tmp1;
}

char * append_strings( char * old_str, const char * new_str)
{
  // find the size of the string to allocate
  const size_t old_len = strlen(old_str), new_len = strlen(new_str);
  const size_t out_len = old_len + new_len + 1;

  // allocate a pointer to the new string
  char *out = (char*)malloc(out_len);
  // concat both strings and return
  memcpy(out, old_str, old_len);
  memcpy(out + old_len, new_str, new_len + 1);
  free(old_str);
  old_str = out;
  return out;
}
// remove margin  old measurements
void trim_strings( char * old_str, const int margin)
{
  // find the size of the string to allocate
  const size_t old_len = strlen(old_str);
  const size_t out_len = old_len + 1;

  // allocate a pointer to the new string
  char *out = (char*)malloc(out_len);

  char * pch;
  int  found = 1;
  pch = strchr(old_str, ';');
  while ((pch != NULL) & (found < margin))
  {
    found++;
    pch = strchr(pch + 1, ';');
  }

  // copy to temp buff since pch
  strcpy(out, pch + 1);
  strcpy(old_str, out);
  free(out);
  return ;
}

// function to print a device address
void printAddress(DeviceAddress deviceAddress) {
  for (uint8_t i = 0; i < 8; i++) {
    if (deviceAddress[i] < 16) Serial.print("0");
    Serial.print(deviceAddress[i], HEX);
  }
}
void initSensors( )
{
  // Start up the library
  sensors.begin();
  // Grab a count of devices on the wire
  g_numberOfDevices = sensors.getDeviceCount();

  // locate devices on the bus
  Serial.print("Locating devices...");
  Serial.print("Found ");
  Serial.print(g_numberOfDevices, DEC);
  Serial.println(" devices.");

  // Loop through each device, print out address
  for (int i = 0; i < g_numberOfDevices; i++) {
    // Search the wire for address
    if (sensors.getAddress(g_tempDeviceAddress, i)) {
      Serial.print("Found device ");
      Serial.print(i, DEC);
      Serial.print(" with address: ");
      printAddress(g_tempDeviceAddress);
      Serial.println();
    } else {
      Serial.print("Found ghost device at ");
      Serial.print(i, DEC);
      Serial.print(" but could not detect address. Check power and cabling");
    }
  }
}
void getTempreatures(float measureC[], int len  )
{
  sensors.requestTemperatures(); // Send the command to get temperatures
  // Loop through each device, print out temperature data
  for (int i = 0; i < g_numberOfDevices; i++) {
    if (sensors.getAddress(g_tempDeviceAddress, i)) {
      // Output the device ID
      Serial.print("Temperature for device: ");
      Serial.println(i, DEC);
      // Print the data
      float tempC = sensors.getTempC(g_tempDeviceAddress);
      Serial.print("Temp C: ");
      Serial.println(tempC);

      if ( i < len )
        measureC[i] = tempC;
    }
  }
}
void recvWithEndMarker(){
  static byte ndx = 0;
  char endMaker = '\n';
  char rc;
  while (Serial.available() > 0 && serialNewData == false )
  {
    rc = Serial.read();
    if (rc != endMaker) 
    {
      recivedChars[ndx] = rc;
      ndx++;
      if (ndx >= numChars) 
      {
        ndx = numChars - 1;
      }
    }
    else 
    {
      recivedChars[ndx] = '\0';
      ndx = 0;
      serialNewData = true;
    }
  }
}
void showNewData() {
 if (serialNewData == true) {
 Serial.print("Unknow command recived ... ");
 Serial.println(recivedChars);
 serialNewData = false;
 }
}
