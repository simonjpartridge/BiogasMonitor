#include <SPI.h>
#include <RH_RF69.h>


RH_RF69 rf69(10, 3);

// these must be kept in sync with the transmitter structs
struct sensor_reading {
  boolean alarm;
  boolean gas_storage_empty;
  boolean gas_storage_full;
  boolean gas_header_empty;
  boolean gas_header_full;
};

struct txdata {
  boolean has_seen_error;
  boolean pump_on;
  sensor_reading reading;
};

void setup() 
{
  Serial.begin(9600);
  Serial.println("start setup");
  while (!Serial) 
    ;
  
  setupRadio();
}

void loop()
{
  if (rf69.available())
  {
    // Should be a message for us now   
    uint8_t buf[sizeof(txdata)];
    uint8_t len = sizeof(txdata);
    txdata data_out;
    if (rf69.recv(buf, &len))
    {
//      RH_RF69::printBuffer("", buf, len); // prints out the buffer
      memcpy(&data_out, buf, len);
      Serial.print("alarm ");
      Serial.println(data_out.reading.alarm);
      
      Serial.print("has_seen_error ");
      Serial.println(data_out.has_seen_error);
      
      Serial.print("gas_header_full ");
      Serial.println(data_out.reading.gas_header_full);
      Serial.print("gas_header_empty ");
      Serial.println(data_out.reading.gas_header_empty);
//      Serial.print("got request: ");
//      Serial.println((char*)buf);
//      Serial.print("RSSI: ");
//      Serial.println(rf69.lastRssi(), DEC);
    }
    else
    {
      Serial.println("recv failed");
    }
  }
}

void setupRadio(){
  // Reset the RFM69 Radio
  pinMode(2, OUTPUT);
  digitalWrite(2, HIGH);
  delayMicroseconds(100);
  digitalWrite(2, LOW);
  // wait until chip ready
  delay(5);
  
  if (!rf69.init())
    Serial.println("init failed");
  // Defaults after init are 434.0MHz, modulation GFSK_Rb250Fd250, +13dbM (for low power module)
  // No encryption
  Serial.println("end init");
  if (!rf69.setFrequency(433.0))
    Serial.println("setFrequency failed");

  // If you are using a high power RF69 eg RFM69HW, you *must* set a Tx power with the
  // ishighpowermodule flag set like this:
  rf69.setTxPower(14, true);

  // The encryption key has to be the same as the one in the client
  uint8_t key[] = { 0x09, 0x06, 0x01, 0x04, 0x05, 0x06, 0x07, 0x08,
                    0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08};
  rf69.setEncryptionKey(key);
  Serial.println("finished setup");
}
