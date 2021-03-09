
// #include <SPI.h>            //stuff for radio
// #include <RH_RF69.h>        //stuff for radio

// #define IS_RFM69HCW

// RH_RF69 rf69(10, 2);        //stuff for radio

//outputs
const int pump_on_pin = 9;
const int has_seen_alarm_pin = 13;
const int has_seen_error_pin = 12;

//inputs
const int gas_alarm_pin = 8;
const int gas_header_empty_pin = 7;
const int gas_header_full_pin = 6;
const int gas_storage_empty_pin = 5;
const int gas_storage_full_pin = 4;

unsigned long pump_on_at_millis = 0;
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

// struct txdata {                               //defines the type of data that is going to be sent by the radio, named txdata
//  boolean has_seen_error;
//  boolean has_seen_alarm;
//  boolean pump_on;
//  sensor_reading reading;
// };

void setup() {
  // setup_radio(); 
  pinMode(gas_header_empty_pin, INPUT);
  pinMode(gas_header_full_pin, INPUT);
  pinMode(gas_alarm_pin, INPUT);
  pinMode(gas_storage_empty_pin, INPUT);
  pinMode(gas_storage_full_pin, INPUT);

  pinMode(pump_on_pin, OUTPUT);
  pinMode(has_seen_alarm_pin, OUTPUT);
  pinMode(has_seen_error_pin, OUTPUT);
  
  set_pump_on_state(false);

 // Serial.begin(9600);                     //for testing
  delay(500);                            //let stuff turn on  
}

void loop() {
  sensor_reading reading = get_sensor_reading();            // calls function which checks the input pins and stores them in the variable called reading which is of the type of sensor_reading

  //Serial.println(pump_state);                                  //for testing
  boolean desired_pump_state = compute_desired_pump_state(reading, pump_state);
  
  boolean has_error = check_for_error(reading);
  if(has_error){
    has_seen_error = true;
  }
  if(reading.alarm){
     has_seen_alarm = true;
   }
  
  if(has_seen_error || has_seen_alarm){
    set_pump_on_state(false);
    digitalWrite(has_seen_alarm_pin, has_seen_alarm);             //turns lights on for error or alarm
    digitalWrite(has_seen_error_pin, has_seen_error);
  }else{
    set_pump_on_state(desired_pump_state); 
      
  }
  
  
//  txdata data_to_send;
//  data_to_send.reading = reading;
//  data_to_send.pump_on = pump_state;
//  data_to_send.has_seen_error = has_seen_error;
//  data_to_send.has_seen_alarm = has_seen_alarm;

//  radiotx(data_to_send);

 delay(200);                                              //slows loop down to allow for radio to send etc
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
  if(reading.gas_header_full && reading.gas_header_empty) return true;
  if(reading.gas_storage_full && reading.gas_storage_empty) return true;               //if 2 switches get stuck on
  if((pump_state) && ((millis() - pump_on_at_millis) > 120000)) return true;     //max time the pump may run for (2 mins)
  return false;
}

void set_pump_on_state(boolean enabled) {
  digitalWrite(pump_on_pin, enabled);                            //turns pump on or off
  if((pump_state == !enabled) && (enabled == true)){            //is true when the pump condition changes from off to on
      pump_on_at_millis = millis();
    }
  pump_state = enabled;                                          //keeps track of whether the pump is on or off
}

// void setup_radio() {
//  //have to reset radio to get it working
//  pinMode(2, OUTPUT);                 
//  digitalWrite(2, HIGH);
//  delayMicroseconds(100);
//  digitalWrite(2, LOW);
  
//  if (!rf69.init()){
//    Serial.println("init failed")
//  }
 // Defaults after init are 434.0MHz, modulation GFSK_Rb250Fd250, +13dbM (for low power module)
 // No encryption
//  rf69.setFrequency(433.0);

//  rf69.setTxPower(13, true);             //sets tx power in dBm

//  // The encryption key has to be the same as the one in the server
//  uint8_t key[] = { 0x09, 0x06, 0x01, 0x04, 0x05, 0x06, 0x07, 0x08,
//                    0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08
//                  };
//  rf69.setEncryptionKey(key);
// }

//void radiotx(txdata in) {
//
//  byte data[sizeof(txdata)];
//  memcpy(data, &in, sizeof(txdata));
//  rf69.send(data, sizeof(data));
//  rf69.waitPacketSent(100);
//}


//new radio code
// void radiotx(txdata in) {
//  if(rf69.waitPacketSent(200)) {
   
//   byte data[sizeof(txdata)];
//   memcpy(data, &in, sizeof(txdata));

//   rf69.setModeTx();
//   delay(50);
//   Serial.println("gonna send");
//   rf69.send(data, sizeof(txdata));
//   Serial.println("sent");
//   if(!rf69.waitPacketSent(200)){
//     setup_radio();
//     Serial.println("radio dead 1");
//   }
//   return;
//  }
//  Serial.println("radio dead 2");
//  setup_radio();                             //calls the function to reset the radio
// }