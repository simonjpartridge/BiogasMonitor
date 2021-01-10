#include <SPI.h>
#include <RH_RF69.h>

RH_RF69 rf69(10, 3);

const int alarm_light_pin = 8;
const int error_light_pin = 7;
const int gas_storage_full_light_pin = 6;
const int gas_storage_empty_light_pin = 5;
const int pump_on_light_pin = 4;

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
  boolean has_seen_alarm;
  boolean pump_on;
  sensor_reading reading;
};

void setup() 
{
  pinMode(alarm_light_pin, OUTPUT);
  pinMode(error_light_pin, OUTPUT);
  pinMode(gas_storage_full_light_pin, OUTPUT);
  pinMode(gas_storage_empty_light_pin, OUTPUT);
  pinMode(pump_on_light_pin, OUTPUT);
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
    uint8_t buf[sizeof(txdata)];
    uint8_t len = sizeof(txdata);
    txdata data_out;
    if (rf69.recv(buf, &len))
    {
//      RH_RF69::printBuffer("", buf, len); // prints out the buffer
      memcpy(&data_out, buf, len);
      Serial.print("alarm: ");
      Serial.print(data_out.reading.alarm);
      Serial.print(". pump on: ");
      Serial.print(data_out.pump_on);
      Serial.print(". has_seen_error: ");
      Serial.print(data_out.has_seen_error);
      Serial.print(". gas_header_full: ");
      Serial.print(data_out.reading.gas_header_full);
      Serial.print(". gas_header_empty: ");
      Serial.print(data_out.reading.gas_header_empty);
      Serial.print(". gas_storage_full: ");
      Serial.print(data_out.reading.gas_header_full);
      Serial.print(". gas_storage_empty: ");
      Serial.print(data_out.reading.gas_storage_empty);
      Serial.print("\n");
digitalWrite(alarm_light_pin, data_out.has_seen_alarm);
digitalWrite(error_light_pin, data_out.has_seen_error);
digitalWrite(gas_storage_full_light_pin, data_out.reading.gas_storage_full);
digitalWrite(gas_storage_empty_light_pin, data_out.reading.gas_storage_empty);
digitalWrite(pump_on_light_pin, data_out.pump_on);
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

  rf69.setTxPower(14, true);

  uint8_t key[] = { 0x09, 0x06, 0x01, 0x04, 0x05, 0x06, 0x07, 0x08,
                    0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08};
  rf69.setEncryptionKey(key);
  Serial.println("finished setup");
}



