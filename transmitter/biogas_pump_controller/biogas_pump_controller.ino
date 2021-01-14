
#include <SPI.h>            //stuff for radio
#include <RH_RF69.h>        //stuff for radio

RH_RF69 rf69(10, 3);        //stuff for radio

const int pump_on_pin = 9;

const int gas_alarm_pin = 8;
const int gas_header_empty_pin = 7;
const int gas_header_full_pin = 6;
const int gas_storage_empty_pin = 5;
const int gas_storage_full_pin = 4;

boolean pump_state = false;

boolean has_seen_error = false; // have we ever experienced an error
boolean has_seen_alarm = false;

struct sensor_reading {
  boolean alarm;
  boolean gas_storage_empty;
  boolean gas_storage_full;
  boolean gas_header_empty;
  boolean gas_header_full;
};

struct txdata {                               //defines the type of data that is going to be sent by the radio, named txdata
 boolean has_seen_error;
 boolean has_seen_alarm;
 boolean pump_on;
 sensor_reading reading;
};

void setup() {
 setup_radio(); 
pinMode(0, INPUT);                                    //FOR TESTING, REMOVE BEFORE USE                            
digitalWrite(0, HIGH);                                //FOR TESTING, REMOVE BOFORE USE
  pinMode(gas_header_empty_pin, INPUT);
  pinMode(gas_header_full_pin, INPUT);
  pinMode(gas_alarm_pin, INPUT);
  pinMode(gas_storage_empty_pin, INPUT);
  pinMode(gas_storage_full_pin, INPUT);
  
  pinMode(pump_on_pin, OUTPUT);
  
  set_pump_on_state(false);
  delay(500);                            //let stuff turn on  
}

void loop() {
  sensor_reading reading = get_sensor_reading();            // calls function which checks the input pins and stores them in the variable called reading which is of the type of sensor_reading

  boolean desired_pump_state = compute_desired_pump_state(reading, pump_state);
  
  boolean has_error = check_for_error(reading);
  if(has_error){
    has_seen_error = true;
  }
   if(reading.alarm){
     has_seen_alarm = true;
   }
  
  if(has_seen_error or has_seen_alarm){
    set_pump_on_state(false);
  }else{
    set_pump_on_state(desired_pump_state);
  }
  
  
 txdata data_to_send;
 data_to_send.reading = reading;
 data_to_send.pump_on = pump_state;
 data_to_send.has_seen_error = has_seen_error;
 data_to_send.has_seen_alarm = has_seen_alarm;

 radiotx(data_to_send);

  delay(100);                                              //slows loop down to allow for radio to send etc
}

sensor_reading get_sensor_reading() {     //made function called get_sensor_reading which will return data in the format of the sensor_reading thing laid out earlier
  sensor_reading reading;
   
  reading.alarm = !digitalRead(gas_alarm_pin);                    //gas alarm state is 0v so reading is inverted using the !
  reading.gas_header_empty = digitalRead(gas_header_empty_pin);
  reading.gas_header_full = digitalRead(gas_header_full_pin);
  reading.gas_storage_empty = digitalRead(gas_storage_empty_pin);
  reading.gas_storage_full = digitalRead(gas_storage_full_pin);
  return reading;
}

boolean compute_desired_pump_state(sensor_reading reading, boolean current_pump_state){ 
    if(reading.alarm == true) return false;
    if(reading.gas_header_full == true) return false;
    if(reading.gas_storage_empty == true) return false;
    if(reading.gas_header_empty == true) return true;
    return current_pump_state;
}
boolean check_for_error(sensor_reading reading){
  if(reading.alarm == true) return true;
  if(reading.gas_header_full and reading.gas_header_empty) return true;
  if(reading.gas_storage_full and reading.gas_storage_empty) return true;
  return false;
}

void set_pump_on_state(boolean enabled) {
  digitalWrite(pump_on_pin, enabled);          //turns pump on or off
  pump_state = enabled;                       //keeps track of whether the pump is on or off
}

void setup_radio() {
 //have to reset radio to get it working
 pinMode(2, OUTPUT);                 
 digitalWrite(2, HIGH);
 delayMicroseconds(100);
 digitalWrite(2, LOW);
  
 rf69.init();
 // Defaults after init are 434.0MHz, modulation GFSK_Rb250Fd250, +13dbM (for low power module)
 // No encryption
 rf69.setFrequency(433.0);

 rf69.setTxPower(50, true);

 // The encryption key has to be the same as the one in the server
 uint8_t key[] = { 0x09, 0x06, 0x01, 0x04, 0x05, 0x06, 0x07, 0x08,
                   0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08
                 };
 rf69.setEncryptionKey(key);
}

//void radiotx(txdata in) {
//
//  byte data[sizeof(txdata)];
//  memcpy(data, &in, sizeof(txdata));
//  rf69.send(data, sizeof(data));
//  rf69.waitPacketSent(100);
//}


//new radio code
void radiotx(txdata in) {
 if(rf69.waitpacketsent(100)) {
  byte data[sizeof(txdata)];
  memcpy(data, &in, sizeof(txdata));
  rf69.send(data, sizeof(data));
  return;
 }
 rf69.reset;
}