#include<string>
#ifndef Data_Config_h
#define Data_Config_h
// WiFi Authentication
char WPA_SSID[] = "****";
char WPA_PASSWORD[] = "***";

// Post URL TODO: option of port
char POST_URL[] = "http://192.168.0.231";

// Variables that get sent as tags along with data
std::string PROGRAM_VERSION = "0.0.1";
std::string COMPILED = __DATE__;
std::string DEVICE = "esp32_MeterReader";

// condense datapoints to send a sum of counts every CONDENSE_FREQUENCY seconds to reduce size of posts.
int CONDENSE_FREQUENCY = 20;

// try to post this often (Miliseconds)
int POST_PERIOD_MILIS = 10 * 1000;

//anything below this value is considered a 0, anything above is considerd a 1 for the purposes of detecting pulses.
int ADC_CUTOFF = 600;
#endif /* _ESP32_CORE_ARDUINO_H_ */

