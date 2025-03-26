//MASTER CODE: Slave code starts from Line 34
#include <Wire.h> // Library for I2C Communication

void setup() {
  Serial.begin(9600); // Begins Serial Communication at 9600 baud rate
  Wire.begin(); // Begins I2C communication
}

void loop() {
  Wire.requestFrom(8, 1); // Request 1 byte from slave Arduino (8)
  byte MasterReceive = Wire.read(); // Receive a byte from the slave Arduino

  int potvalue = analogRead(A0); // Reads analog value from POT (0-5V)
  byte MasterSend = map(potvalue, 0, 1023, 0, 127); // Convert digital value (0 to 1023) to (0 to 127)

  Wire.beginTransmission(8); // Start transmission to slave Arduino (8)
  Wire.write(MasterSend); // Send one byte converted POT value to slave
  Wire.endTransmission(); // Stop transmitting

  // Print values to Serial Monitor
  Serial.print("Master Sent to Slave: ");
  Serial.println(MasterSend);
  Serial.print("Master Received from Slave: ");
  Serial.println(MasterReceive);
  Serial.println("------------------------------");

  delay(500);
}





// SLAVE CODE: 
#include <Wire.h> // Library for I2C Communication

byte SlaveReceived = 0;

void setup() {
  Serial.begin(9600); // Begins Serial Communication at 9600 baud rate
  Wire.begin(8); // Begins I2C communication with Slave Address as 8
  Wire.onReceive(receiveEvent); // Function call when Slave receives value from master
  Wire.onRequest(requestEvent); // Function call when Master requests value from Slave
}

void loop() {
  // Print received value from Master
  Serial.print("Slave Received from Master: ");
  Serial.println(SlaveReceived);
  Serial.println("------------------------------");

  delay(500);
}

void receiveEvent(int howMany) { // Called when Slave receives value from Master
  SlaveReceived = Wire.read(); // Read received value from Master
}

void requestEvent() { // Called when Master requests value from Slave
  int potvalue = analogRead(A0); // Reads analog value from POT (0-5V)
  byte SlaveSend = map(potvalue, 0, 1023, 0, 127); // Convert digital value (0 to 127)
  Wire.write(SlaveSend); // Send one byte converted POT value to Master
}
