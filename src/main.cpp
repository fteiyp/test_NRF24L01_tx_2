
//Client
#include <SPI.h>
#include <RH_NRF24.h>

// data transfer array
uint16_t unmapppedValues[6]; // 2 bytes because 0 to 1055 analogRead
uint8_t flexValues[6]; // 1 byte for mapped number

// Singleton instance of the radio driver
RH_NRF24 nrf24;


void setup() 
{
  Serial.begin(9600);

  // Configuring nrf24 wireless transceivers
  while (!Serial) 
    ; // wait for serial port to connect. Needed for Leonardo only
  if (!nrf24.init())
    Serial.println("init failed");
  // Defaults after init are 2.402 GHz (channel 2), 2Mbps, 0dBm
  if (!nrf24.setChannel(1))
    Serial.println("setChannel failed");
  if (!nrf24.setRF(RH_NRF24::DataRate2Mbps, RH_NRF24::TransmitPower0dBm))
    Serial.println("setRF failed");    
}

void loop()
{
  // Reading flex sensors no loop because specific values 6 and 7
  unmapppedValues[0] = analogRead(0);
  unmapppedValues[1] = analogRead(1);
  unmapppedValues[2] = analogRead(2);
  unmapppedValues[3] = analogRead(3);
  unmapppedValues[4] = analogRead(6);
  unmapppedValues[5] = analogRead(7);

  Serial.println("unmapped sensor values: ");
  Serial.println(unmapppedValues[0]);
  Serial.println(unmapppedValues[1]);
  Serial.println(unmapppedValues[2]);
  Serial.println(unmapppedValues[3]);
  Serial.println(unmapppedValues[4]);
  Serial.println(unmapppedValues[5]);

  /* Map all values to 0 to 180 servo range
  Each flex sensor needs a specific mapping function because
  they have different resistances */
  flexValues[0] = map(unmapppedValues[0], 760, 1000, 0, 180);
  flexValues[1] = map(unmapppedValues[1], 700, 980, 0, 180);
  flexValues[2] = map(unmapppedValues[2], 770, 1000, 0, 180);
  flexValues[3] = map(unmapppedValues[3], 860, 1000, 0, 180);
  flexValues[4] = map(unmapppedValues[4], 700, 1000, 0, 180);
  flexValues[5] = map(unmapppedValues[5], 700, 1000, 0, 180);

  Serial.println("Sending to nrf24_server");
  // Send flexValues to nrf24_server
  nrf24.send(flexValues, sizeof(flexValues));
  nrf24.waitPacketSent();
  
  delay(100);
}

