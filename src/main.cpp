
// Right hand device - TX2
#include <RF24.h>
#include <Adafruit_SSD1306.h>
#include <MPU6050_light.h>


// role of transmitter (node number)
#define role 1
#define DELAY 20 // 20 ms delay

// data transfer array
uint16_t values[5];

// instantiate an object for the nRF24L01 transceiver
RF24 radio(7, 8); // using pin 7 for the CE pin, and pin 8 for the CSN pin

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
#define SCREEN_ADDRESS 0x3C ///< See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// instatiate the accelerometer
// Adafruit_MPU6050 mpu;
MPU6050 mpu(Wire);

// For this example, we'll be using 6 addresses; 1 for each TX node
// It is very helpful to think of an address as a path instead of as
// an identifying device destination
// Notice that the last byte is the only byte that changes in the last 5
// addresses. This is a limitation of the nRF24L01 transceiver for pipes 2-5
// because they use the same first 4 bytes from pipe 1.
uint64_t address[6] =
{
  0x7878787878LL,
  0xB3B4B5B6F1LL,
};

struct PayloadStruct
{
  uint8_t nodeID;
  uint8_t val1;
  uint8_t val2;
  uint8_t val3;
  uint8_t val4;
  uint8_t val5;
};
PayloadStruct payload;

void serialPrintUnmapped();
void displaySensors();

/*************************************** SETUP *****************************************/

void setup()
{
  Serial.begin(115200);

  // initialize the transceiver on the SPI bus
  if (!radio.begin()) { 
    Serial.println(F("radio flop"));
    while (1) {} // hold in infinite loop
  }
  // Set the PA Level low to try preventing power supply related problems
  // because these examples are likely run with nodes in close proximity of
  // each other.
  radio.setPALevel(RF24_PA_LOW); // RF24_PA_MAX is default.
  // save on transmission time by setting the radio to only transmit the
  // number of bytes we need to transmit a float
  radio.setPayloadSize(sizeof(payload)); // 2x int datatype occupy 8 bytes
  // Set the address on pipe 0 to the RX node.
  radio.stopListening(); // put radio in TX mode
  radio.openWritingPipe(address[role]);
  // ****** Change this to not retry?????????
  // According to the datasheet, the auto-retry features's delay value should
  // be "skewed" to allow the RX node to receive 1 transmission at a time.
  // So, use varying delay between retry attempts and 15 (at most) retry attempts
  radio.setRetries(((role * 3) % 12) + 3, 0); // maximum value is 15 for both args (2 retries)

  // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("SSD1306 flop"));
    while (1) {} // hold in infinite loop
  }

  // initialize the MP6050
  Wire.begin();
  byte status = mpu.begin();
  Serial.print(F("MPU6050 status: "));
  Serial.println(status);
  while(status!=0){ } // stop everything if could not connect to MPU6050

  // setup OLED display
  display.clearDisplay();
  display.setRotation(2);
  display.setTextSize(1);               
  display.setTextColor(SSD1306_WHITE); 

  // set the payload's nodeID & reset the payload's identifying number
  payload.nodeID = role;
  payload.val1 = 0;

}

/*************************************** LOOP *****************************************/

void loop()
{
  // Reading flex sensors no loop because specific values 6 and 7
  values[0] = analogRead(0);
  values[1] = analogRead(1);
  values[2] = analogRead(2);
  values[3] = analogRead(3);
  values[4] = analogRead(6);

  // display values on OLED
  display.clearDisplay();
  display.setCursor(0,0);
  display.println(F("Unmapped:"));
  display.print(values[0]);
  display.print(F(", "));
  display.print(values[1]);
  display.print(F(", "));
  display.print(values[2]);
  display.print(F(", "));
  display.print(values[3]);
  display.print(F(", "));
  display.print(values[4]);

  /* Map all values to 0 to 180 servo range
  Each flex sensor needs a specific mapping function because
  they have different resistances */
  values[0] = map(values[0], 700, 1000, 80, 180) - 21;
  values[1] = map(values[1], 700, 1000, 80, 180) - 8;
  values[2] = map(values[2], 700, 1000, 80, 180) - 27;
  values[3] = map(values[3], 700, 1000, 80, 180) - 3;
  values[4] = map(values[4], 700, 1000, 80, 180) - 3;

  display.println();
  display.println(F("Mapped:"));
  display.print(values[0]);
  display.print(F(", "));
  display.print(values[1]);
  display.print(F(", "));
  display.print(values[2]);
  display.print(F(", "));
  display.print(values[3]);
  display.print(F(", "));
  display.println(values[4]);

  // update the MPU and print to display
  mpu.update();
  uint8_t accX = (uint8_t) (mpu.getAccX() * 90) + 90;
  uint8_t accY = (uint8_t) (mpu.getAccY() * 90) + 90;
  display.println();
  display.print(F("X: ")); display.print(accX);
  display.print(F(", Y: ")); display.print(accY);

  display.display();

  payload.val1 = lowByte(values[0]);
  payload.val2 = lowByte(values[1]);
  payload.val3 = lowByte(values[2]);
  payload.val4 = lowByte(values[3]);
  payload.val5 = lowByte(values[4]);

  // This device is a TX node
  unsigned long start_timer = micros();                 // start the timer
  bool report = radio.write(&payload, sizeof(payload)); // transmit & save the report
  unsigned long end_timer = micros();                   // end the timer

  if (report)
  {
    // payload was delivered
    Serial.print(F("Transmission of payloadID "));
    Serial.print(payload.val1); // print payloadID
    Serial.print(F(" as node "));
    Serial.print(payload.nodeID); // print nodeID
    Serial.print(F(" successful!"));
    Serial.print(F(" Time to transmit: "));
    Serial.print(end_timer - start_timer); // print the timer result
    Serial.println(F(" us"));
  }
  else
  {
    Serial.println(F("Transmission failed or timed out")); // payload was not delivered
  }

  // to make this example readable in the serial monitor
  delay(DELAY); // slow transmissions down by 1 second

} // loop

/*************************************** FUNCTIONS *****************************************/