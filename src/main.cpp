#include <Arduino.h>
#include <Settings_Config.h>
#define LED_BUILTIN 2
#define PowerReadPin 32
#define THRESHHOLD_ADC_VALUE 100
#include <WiFi.h>
#include <HTTPClient.h>
#include <EasyNTPClient.h>
#include <WiFiUdp.h>
#include <assert.h>
#include <Data.h>
#include <vector>

TaskHandle_t detect_pulse_t, post_data_t;
DataClass data(PROGRAM_VERSION,COMPILED,DEVICE,CONDENSE_FREQUENCY);
HTTPClient http;
WiFiUDP udp;
EasyNTPClient ntpClient(udp, "pool.ntp.org");

void post_data(void * pvParameters)
{
  while(true)
  {
  delay(2000);
  if (data.items > 5)
  {
    http.begin(POST_URL);
    http.addHeader("Content-Type", "text/plain");
    Serial.println(data.return_post_payload());
    int httpResponseCode=http.POST(data.return_post_payload());
    if (httpResponseCode == 200)
    {
      Serial.print("success: sent ");
      Serial.println(data.items+1);
      data.remove_sent_pulses();
    }
    else
    {
      Serial.print("failed to send items: ");
      Serial.println(data.items+1);
    }
  }
    //delay for 60 seconds
    delay(POST_PERIOD_MILIS);
    }
}

bool wait_for_state(int pin, int cuttoff_value, bool state)
{
  // basically a way to do pulseIn but without having a digital input
  // instead just picking an analog value and doing a binary cutoff
  while (analogRead(pin) > cuttoff_value == state)
  {
    delay(10);
    //Do nothing at all. Lock until we detect the required state
  }
}

void detect_pulse(int pin, int cutoff_value)
{
  wait_for_state(PowerReadPin,ADC_CUTOFF,false);
  digitalWrite(LED_BUILTIN,LOW);
  wait_for_state(PowerReadPin,ADC_CUTOFF,true);
  digitalWrite(LED_BUILTIN,HIGH);
  wait_for_state(PowerReadPin,ADC_CUTOFF,false);
  digitalWrite(LED_BUILTIN,LOW);
  // at this point a pulse has been detected.
  data.add_pulse(ntpClient.getUnixTime());
}


void pulse_detect_run_forever(void * pvParameters)
{
  while(true)
  {
    Serial.println("Detecting pulses");
    detect_pulse(PowerReadPin,ADC_CUTOFF);
  }
}
void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  Serial.println(CONDENSE_FREQUENCY);
  delay(1000);
  Serial.println("Starting");
  WiFi.begin(WPA_SSID, WPA_PASSWORD);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(5000);
    Serial.println("Connecting to wifi....");
  }
  
  Serial.println("Connected to Wifi");
  WiFiUDP udp;
  EasyNTPClient ntpClient(udp, "pool.ntp.org");
  Serial.println(ntpClient.getUnixTime());
  // If unix time is less than 2019 then we've got a problem.
  assert(ntpClient.getUnixTime() > 1546300800);
  // check that I can post to a server that should be up.

  //while (!test_post())
  //{
    //Serial.println("Failed to do a test post to postman-echo.. this isn't good...");
    //delay(10000);
  //}

  xTaskCreatePinnedToCore(
                    pulse_detect_run_forever,   /* Task function. */
                    "detect_pulse",     /* name of task. */
                    20000,       /* Stack size of task */
                    NULL,        /* parameter of the task */
                    1,           /* priority of the task */
                    &detect_pulse_t,      /* Task handle to keep track of created task */
                    0);          /* pin task to core 0 */                  
  delay(500); 


   xTaskCreatePinnedToCore(
                     post_data,   /* Task function. */
                     "post_data",     /* name of task. */
                     20000,       /* Stack size of task */
                     NULL,        /* parameter of the task */
                     1,           /* priority of the task */
                     &post_data_t,      /* Task handle to keep track of created task */
                     1);          /* pin task to core 1 */                  
   delay(500); 


}

void loop() {
  //nothing required in here, I have two infinitly running tasks pinned to two of the esp32 cores.
}