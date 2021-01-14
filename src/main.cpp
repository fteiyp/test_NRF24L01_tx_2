
//Client
#include <SPI.h>
#include <RH_NRF24.h>

// data transfer array
uint8_t flexValues[6];

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
  // Reading flex sensors
  for (int i = 0; i < 4; i++) {
    flexValues[i] = analogRead(i);
  }
  // Last 2 flex sensor values are on pins 6 and 7
  flexValues[4] = analogRead(6);
  flexValues[5] = analogRead(7);

  // Map all values to 0 to 255 range
  for (int i = 0; i < 6; i++) {
    flexValues[i] = map(flexValues[i], 0, 1023, 0, 255);
  }

  Serial.println("mapped sensor values: ");
  Serial.println(flexValues[0]);
  Serial.println(flexValues[1]);
  Serial.println(flexValues[2]);
  Serial.println(flexValues[3]);
  Serial.println(flexValues[4]);
  Serial.println(flexValues[5]);

  Serial.println("Sending to nrf24_server");
  // Send a message to nrf24_server
  // uint8_t data[] = "FLAHFLAH";
  // nrf24.send(data, sizeof(data));
  nrf24.send(flexValues, sizeof(flexValues));

  
  nrf24.waitPacketSent();
  
  delay(400);
}

