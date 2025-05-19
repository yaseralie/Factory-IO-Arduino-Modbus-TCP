/*
   By: Yaser Ali Husen
  https://www.arduino.cc/reference/en/libraries/arduinomodbus/
  Arduino Mega with Ethernet Shield
  Factory Scene: Automated Warehouse
  Manual Read Sensors and Trigger Actuator
    - Drivers: Modbus TCP/IP Client
    - Inputs:
          + Coil 0: At Entry
          + Coil 1: At Load
          + Coil 2: At Left
          + Coil 3: At Middle
          + Coil 4: At Right
          + Coil 5: At Unload
          + Coil 6: At Exit
          + Coil 7: Moving X
    - Actuators:
          + Coil 8: Emitter
          + Coil 9: Entry Conveyor
          + Coil 10: Load Conveyor
          + COil 11: Forks Left
          + Coil 12: Forks Right
          + Coil 13: Lift
          + Coil 14: Unload Conveyor
          + Coil 15: Exit Conveyor
          + Holding Register0: Target Position
*/

#include <SPI.h>
#include <Ethernet.h>

#include <ArduinoRS485.h> // ArduinoModbus depends on the ArduinoRS485 library
#include <ArduinoModbus.h>
#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(0x27, 20, 4); // set the LCD address to 0x27 for a 16 chars and 2 line display

// Enter a MAC address for your controller below.
// The IP address will be dependent on your local network:
byte mac[] = {
  0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED
};
IPAddress ip(192, 168, 0, 88);
EthernetServer ethServer(502);
ModbusTCPServer modbusTCPServer;

//for button========================
int actuator1 = 22;
int actuator2 = 24;
int actuator3 = 26;
int actuator4 = 28;
int actuator5 = 30;
int actuator6 = 32;
int actuator7 = 34;
int actuator8 = 36;
int storage = 38;
int retrieving = 40;

bool status_button1 = false;
bool status_button2 = false;
bool status_button3 = false;
bool status_button4 = false;
bool status_button5 = false;
bool status_button6 = false;
bool status_button7 = false;
bool status_button8 = false;
bool status_button9 = false;
bool status_button10 = false;
//for button========================

// For variable sensor
int bit_val0 = 0;
int bit_val1 = 0;
int bit_val2 = 0;
int bit_val3 = 0;
int bit_val4 = 0;
int bit_val5 = 0;
int bit_val6 = 0;
int bit_val7 = 0;

//For stacker
bool status_storage = false;
bool status_retrieving = false;
int storage_pos = 1;
int retrieving_pos = 1;

void setup() {
  //Setup button to trigger actuator
  pinMode(actuator1, INPUT_PULLUP); //emitter
  pinMode(actuator2, INPUT_PULLUP); //entry conveyor
  pinMode(actuator3, INPUT_PULLUP); //load conveyor
  pinMode(actuator4, INPUT_PULLUP); //forks left
  pinMode(actuator5, INPUT_PULLUP); //forks right
  pinMode(actuator6, INPUT_PULLUP); //lift
  pinMode(actuator7, INPUT_PULLUP); //unload conveyor
  pinMode(actuator8, INPUT_PULLUP); //exit conveyor
  pinMode(storage, INPUT_PULLUP); //stacker target position
  pinMode(retrieving, INPUT_PULLUP); //stacker target position

  //LCD
  lcd.init();
  //Print a message to the LCD.
  lcd.backlight();

  // Open serial communications and wait for port to open:
  Serial.begin(9600);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }
  Serial.println("Ethernet Modbus TCP Example");

  // start the Ethernet connection and the server:
  Ethernet.begin(mac, ip);

  // Check for Ethernet hardware present
  if (Ethernet.hardwareStatus() == EthernetNoHardware) {
    Serial.println("Ethernet shield was not found.  Sorry, can't run without hardware. :(");
    while (true) {
      delay(1); // do nothing, no point running without Ethernet hardware
    }
  }
  if (Ethernet.linkStatus() == LinkOFF) {
    Serial.println("Ethernet cable is not connected.");
  }

  // start the server
  ethServer.begin();

  // start the Modbus TCP server
  if (!modbusTCPServer.begin()) {
    Serial.println("Failed to start Modbus TCP Server!");
    while (1);
  }

  // configure 16 coils at address 0x00
  // Coil 0-7 for Sensor
  // Coil 8-15 for actuator
  modbusTCPServer.configureCoils(0, 16);

  // configure Holding Register at address 0x00 for target position
  modbusTCPServer.configureHoldingRegisters(0, 1);
}

void loop() {
  // listen for incoming clients
  EthernetClient client = ethServer.available();

  if (client) {
    // a new client connected
    Serial.println("new client");

    // let the Modbus TCP accept the connection
    modbusTCPServer.accept(client);

    while (client.connected()) {
      // poll for Modbus TCP requests, while client connected
      modbusTCPServer.poll();

      // update sensor
      read_coils();

      //check button
      check_button();
    }

    Serial.println("client disconnected");
  }
}

void read_coils() {
  // read the current value of the coil
  bit_val0 = modbusTCPServer.coilRead(0);
  bit_val1 = modbusTCPServer.coilRead(1);
  bit_val2 = modbusTCPServer.coilRead(2);
  bit_val3 = modbusTCPServer.coilRead(3);
  bit_val4 = modbusTCPServer.coilRead(4);
  bit_val5 = modbusTCPServer.coilRead(5);
  bit_val6 = modbusTCPServer.coilRead(6);
  bit_val7 = modbusTCPServer.coilRead(7);

  //Display to LCD
  lcd.setCursor(0, 0);
  lcd.print("Entry=" + String(bit_val0) + ", Load=" + String(bit_val1));
  lcd.setCursor(0, 1);
  lcd.print("Left=" + String(bit_val2) + ", Middle=" + String(bit_val3));
  lcd.setCursor(0, 2);
  lcd.print("Right=" + String(bit_val4) + ", Unload=" + String(bit_val5));
  lcd.setCursor(0, 3);
  lcd.print("Exit=" + String(bit_val6) + ", MovX=" + String(bit_val7));
}

void print_status() {
  //Print serial
  Serial.print("Status Sensors: ");
  Serial.print(bit_val0);
  Serial.print(", ");
  Serial.print(bit_val1);
  Serial.print(", ");
  Serial.print(bit_val2);
  Serial.print(", ");
  Serial.print(bit_val3);
  Serial.print(", ");
  Serial.print(bit_val4);
  Serial.print(", ");
  Serial.print(bit_val5);
  Serial.print(", ");
  Serial.print(bit_val6);
  Serial.print(", ");
  Serial.println(bit_val7);
  Serial.print("Storage Position: ");
  Serial.print(storage_pos);
  Serial.print(", Retrieving Position: ");
  Serial.println(retrieving_pos);
}

void check_button() {
  //check button1=================================
  int buttonValue1 = digitalRead(actuator1);
  if (buttonValue1 == LOW )
  {
    if (status_button1 == false)
    {
      //turn on
      status_button1 = true;
      modbusTCPServer.coilWrite(8, 1);
      Serial.println("Emitter ON");
      print_status();
    }
  }
  else if (buttonValue1 == HIGH)
  {
    if (status_button1 == true)
    {
      //turn off
      status_button1 = false;
      modbusTCPServer.coilWrite(8, 0);
      Serial.println("Emitter OFF");
      print_status();
    }
  }
  //============================================
  //check button2=================================
  int buttonValue2 = digitalRead(actuator2);
  if (buttonValue2 == LOW )
  {
    if (status_button2 == false)
    {
      //turn on
      status_button2 = true;
      modbusTCPServer.coilWrite(9, 1);
      Serial.println("Entry Conveyor ON");
      print_status();
    }
  }
  else if (buttonValue2 == HIGH)
  {
    if (status_button2 == true)
    {
      //turn off
      status_button2 = false;
      modbusTCPServer.coilWrite(9, 0);
      Serial.println("Entry Conveyor OFF");
      print_status();
    }
  }
  //============================================
  //check button3=================================
  int buttonValue3 = digitalRead(actuator3);
  if (buttonValue3 == LOW )
  {
    if (status_button3 == false)
    {
      //turn on
      status_button3 = true;
      modbusTCPServer.coilWrite(10, 1);
      Serial.println("Load Conveyor ON");
      print_status();
    }
  }
  else if (buttonValue3 == HIGH)
  {
    if (status_button3 == true)
    {
      //turn off
      status_button3 = false;
      modbusTCPServer.coilWrite(10, 0);
      Serial.println("Load Conveyor OFF");
      print_status();
    }
  }
  //============================================
  //check button4=================================
  int buttonValue4 = digitalRead(actuator4);
  if (buttonValue4 == LOW )
  {
    if (status_button4 == false)
    {
      //turn on
      status_button4 = true;
      modbusTCPServer.coilWrite(11, 1);
      Serial.println("Forks Left ON");
      print_status();
    }
  }
  else if (buttonValue4 == HIGH)
  {
    if (status_button4 == true)
    {
      //turn off
      status_button4 = false;
      modbusTCPServer.coilWrite(11, 0);
      Serial.println("Forks Left OFF");
      print_status();
    }
  }
  //============================================
  //check button5=================================
  int buttonValue5 = digitalRead(actuator5);
  if (buttonValue5 == LOW )
  {
    if (status_button5 == false)
    {
      //turn on
      status_button5 = true;
      modbusTCPServer.coilWrite(12, 1);
      Serial.println("Forks Right ON");
      print_status();
      //If storage position, increament position
      if (status_storage == true) {
        storage_pos = storage_pos + 1;
      }
      //If retrieving position, increament position
      if (status_retrieving == true) {
        retrieving_pos = retrieving_pos + 1;
      }
    }
  }
  else if (buttonValue5 == HIGH)
  {
    if (status_button5 == true)
    {
      //turn off
      status_button5 = false;
      modbusTCPServer.coilWrite(12, 0);
      Serial.println("Forks Right OFF");
      print_status();
    }
  }
  //============================================
  //check button6=================================
  int buttonValue6 = digitalRead(actuator6);
  if (buttonValue6 == LOW )
  {
    if (status_button6 == false)
    {
      //turn on
      status_button6 = true;
      modbusTCPServer.coilWrite(13, 1);
      Serial.println("Lift ON");
      print_status();
    }
  }
  else if (buttonValue6 == HIGH)
  {
    if (status_button6 == true)
    {
      //turn off
      status_button6 = false;
      modbusTCPServer.coilWrite(13, 0);
      Serial.println("Lift OFF");
      print_status();
    }
  }
  //============================================
  //check button7=================================
  int buttonValue7 = digitalRead(actuator7);
  if (buttonValue7 == LOW )
  {
    if (status_button7 == false)
    {
      //turn on
      status_button7 = true;
      modbusTCPServer.coilWrite(14, 1);
      Serial.println("Unload Conveyor ON");
      print_status();
    }
  }
  else if (buttonValue7 == HIGH)
  {
    if (status_button7 == true)
    {
      //turn off
      status_button7 = false;
      modbusTCPServer.coilWrite(14, 0);
      Serial.println("Unload Conveyor OFF");
      print_status();
    }
  }
  //============================================
  //check button8=================================
  int buttonValue8 = digitalRead(actuator8);
  if (buttonValue8 == LOW )
  {
    if (status_button8 == false)
    {
      //turn on
      status_button8 = true;
      modbusTCPServer.coilWrite(15, 1);
      Serial.println("Exit Conveyor ON");
      print_status();
    }
  }
  else if (buttonValue8 == HIGH)
  {
    if (status_button8 == true)
    {
      //turn off
      status_button8 = false;
      modbusTCPServer.coilWrite(15, 0);
      Serial.println("Exit Conveyor OFF");
      print_status();
    }
  }
  //============================================
  //check button9=================================
  int buttonValue9 = digitalRead(storage);
  if (buttonValue9 == LOW )
  {
    if (status_button9 == false)
    {
      //turn on
      status_button9 = true;
      modbusTCPServer.holdingRegisterWrite(0, storage_pos);
      status_storage = true;
      Serial.println("GoTo Position");
      print_status();
    }
  }
  else if (buttonValue9 == HIGH)
  {
    if (status_button9 == true)
    {
      //turn off
      status_button9 = false;
      modbusTCPServer.holdingRegisterWrite(0, 100);
      status_storage = false;
      Serial.println("Home Position");
      print_status();
    }
  }
  //============================================
  //check button10=================================
  int buttonValue10 = digitalRead(retrieving);
  if (buttonValue10 == LOW )
  {
    if (status_button10 == false)
    {
      //turn on
      status_button10 = true;
      modbusTCPServer.holdingRegisterWrite(0, retrieving_pos);
      status_retrieving = true;
      Serial.println("GoTo Position");
      print_status();
    }
  }
  else if (buttonValue10 == HIGH)
  {
    if (status_button10 == true)
    {
      //turn off
      status_button10 = false;
      modbusTCPServer.holdingRegisterWrite(0, 100);
      status_retrieving = false;
      Serial.println("Home Position");
      print_status();
    }
  }
  //============================================
}
