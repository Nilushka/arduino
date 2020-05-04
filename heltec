#include <PubSubClient.h>
#include <ESP8266WiFi.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <BME280I2C.h>
#include <Wire.h>
#include <Vector.h>
#include "heltec.h" // alias for #include "SSD1306Wire.h"
#include "images.h"

#define MQ2PIN A0
#define EXTPIN 0 //D3
#define TEMPIN 12 //D6

// Setup a oneWire instance to communicate with any OneWire devices (not just Maxim/Dallas temperature ICs)
OneWire oneWire_ext(EXTPIN);
OneWire oneWire_tem(TEMPIN);
// Pass our oneWire reference to Dallas Temperature.
DallasTemperature ds18b20_ext(&oneWire_ext);
DallasTemperature ds18b20_tem(&oneWire_tem);

BME280I2C bme;
WiFiClient espClient;
PubSubClient client(espClient);


/*************************** Calibration and Tweaks************************************/
String  dev_loc = "PEC"; //device location
String  dev_num = "1"; //device number

float t_add = -0.6; //temperature calibration value as addendum
float t_mult = 1.027; //temperature calibration value as factor

float et_add = -0.5; //external temperature calibration value as addendum
float et_mult = 1.031; //external temperature calibration value as factor (1.019 as default)

float h_add = -4; //humidity calibration value as addendum
float h_mult = 1; //humidity calibration value as factor

int heating = 60; //MQ2 heating time (sec)
int refresh = 4;  //display and sending refresh time
int wifi_reboot = 60;  //wifi timeout reboot time

/************************* WiFi Access Point *********************************/

#define wifi_ssid "netis"
#define wifi_password "123456Aa"

/************************* MQTT Setup *********************************/

#define mqtt_server "192.168.1.2"
#define mqtt_user "bananagas"      // if exist
#define mqtt_password "123456Aa"

String display_loc_num_str = dev_loc + " " + dev_num;
String loc_num_str = dev_loc + dev_num;
String temperature_topic_str = dev_loc + "/" + dev_num + "/" + "t"; //Topic temperature
String humidity_topic_str = dev_loc + "/" + dev_num + "/" + "h";
String pressure_topic_str = dev_loc + "/" + dev_num + "/" + "p";
String etemperature_topic_str = dev_loc + "/" + dev_num + "/" + "et";
String etcon_topic_str = dev_loc + "/" + dev_num + "/" + "ec";
String gas_topic_str = dev_loc + "/" + dev_num + "/" + "g";
const char *temperature_topic = temperature_topic_str.c_str();
const char *humidity_topic = humidity_topic_str.c_str();
const char *pressure_topic = pressure_topic_str.c_str();
const char *etemperature_topic = etemperature_topic_str.c_str();
const char *etcon_topic = etcon_topic_str.c_str();
const char *gas_topic = gas_topic_str.c_str();
const char *display_loc_num = display_loc_num_str.c_str();
const char *loc_num = loc_num_str.c_str();

String cal_t_sign;
String cal_t_add;
String cal_t_mult;
String cal_et_sign;
String cal_et_add;
String cal_et_mult;
String cal_arr;

//Definition main vars
float t; //temperature
float h; //humidity
float p; //pressure
float et; //external temperature
float g; // gas
float g_per; //gas in percents
float devtemp, hum; //sensor reading vars
bool et_on; // external temperature connection cache vars

//Output to display template vars
char buf_temp[4];
char buf_humi[4];
char buf_gas[4];
char buf_etemp[4];

//Output to MQTT  template vars
char buf_mqtt_temp[4];
char buf_mqtt_humi[4];
char buf_mqtt_pres[4];
char buf_mqtt_etemp[4];
char buf_mqtt_gas[4];

//Arrays
std::vector <float> t_arr;
std::vector <int> h_arr;
std::vector <float> p_arr;
std::vector <float> et_arr;
std::vector <int> g_arr;

unsigned long timing; //timer
unsigned long wifi_timing; //timer

void setup()
{
  //Serial.begin(9600);
  Wire.begin();
  bme.begin();
  ds18b20_tem.begin();
  ds18b20_ext.begin();
  Heltec.begin(true /*DisplayEnable Enable*/, false /*Serial Enable*/);
  pinMode(EXTPIN, INPUT);
  pinMode(TEMPIN, INPUT);
  client.setServer(mqtt_server, 1883);
  cal_t_sign = numberSign (t_add);
  cal_et_sign = numberSign (et_add);
  cal_t_add = abs((int(t_add * 10) % 10));
  cal_t_mult = (int(t_mult * 1000) % 1000);
  cal_et_add = abs((int(et_add * 10) % 10));
  cal_et_mult = (int(et_mult * 1000) % 1000);
  cal_arr = cal_t_sign + cal_t_add + " " + cal_t_mult + "  " + cal_et_sign + cal_et_add + " " + cal_et_mult;
  delay(1000);
}

void(* resetFunc) (void) = 0; // reset function
void loop() {
  Heltec.display->clear();
  if ((millis() - timing) > refresh * 1000)
  {
    timing = millis();
    client.loop();

    bme.read(p, devtemp, hum);
    h = (hum + h_add) * h_mult;
    g = analogRead(MQ2PIN);
    ds18b20_tem.requestTemperatures();
    ds18b20_ext.requestTemperatures();
    t = (ds18b20_tem.getTempCByIndex(0) + t_add) * t_mult;
    et = (ds18b20_ext.getTempCByIndex(0) + et_add) * et_mult;

    Heltec.display->clear();
    Heltec.display->setFont(ArialMT_Plain_10);

    Heltec.display->drawString(0, 22, cal_arr);
    Heltec.display->drawString(93, 22, display_loc_num);
    Heltec.display->setFont(ArialMT_Plain_16);

    //WI-FI
    if (WiFi.status() != WL_CONNECTED)
    {
      // Serial.println("Trying to connect");
      //WiFi.hostname (loc_num);
      WiFi.mode(WIFI_STA);
      WiFi.begin(wifi_ssid, wifi_password);
      delay(6000);
      if (WiFi.status() == WL_CONNECTED)
      {
        wifi_timing = millis();
      }
      else
      {
        if ((millis() - wifi_timing) > wifi_reboot * 1000)
        {
          resetFunc();
        }
      }
    }
    else
    {
      Heltec.display->drawXbm(60, 25, wifi_w, wifi_h, wifi); // Wi-Fi icon
    }

    //MQTT
    if (!client.connected())
    {
      if (WiFi.status() == WL_CONNECTED)
      {
        client.connect(loc_num, mqtt_user, mqtt_password);
        delay(5000);
      }
    }
    else
    {
      Heltec.display->drawXbm(77, 25, ser_w, ser_h, ser); //MQTT icon
    }

    // OLED BLOCK

    // header pics
    Heltec.display->drawXbm(8, 0, tem_w, tem_h, tem);
    Heltec.display->drawXbm(48, 0, shup_w, shup_h, shup);
    Heltec.display->drawXbm(82, 0, vla_w, vla_h, vla);
    Heltec.display->drawXbm(111, 0, gaz_w, gaz_h, gaz);

    // splitters
    Heltec.display->drawVerticalLine(36, 0, 21);
    Heltec.display->drawVerticalLine(76, 0, 21);
    Heltec.display->drawVerticalLine(105, 0, 21);
    Heltec.display->drawHorizontalLine(0, 22, 128);

    //DATA BLOCK
    /************************* TEMPERATURE MEDIAN AND SEND *********************************/
    if (t < -100 || t > 80)
    {
      Heltec.display->drawString(6, 5, "Err");
    }
    else
    {
      dtostrf(t, 4, 1, buf_temp);
      Heltec.display->drawString(0, 5, buf_temp);

      t_arr.push_back (t);
      if (t_arr.size () >= 5)
      {
        for (int i = 1; i < 5; ++i)
        {
          for (int r = 0; r < 5 - i; r++)
          {
            if (t_arr[r] < t_arr[r + 1])
            {
              float temp = t_arr[r];
              t_arr[r] = t_arr[r + 1];
              t_arr[r + 1] = temp;
            }
          }
        }
        dtostrf(t_arr[2], 0, 1, buf_mqtt_temp);  //
        client.publish(temperature_topic, String(buf_mqtt_temp).c_str(), true);
        t_arr.clear();   // array content remove
      }
    }

    /************************* ETEMPERATURE MEDIAN AND SEND *********************************/
    if (et < -100 || et > 80)
    {
      Heltec.display->drawXbm(45, 8, dis_w, dis_h, dis);
      if (et_on = true);
      {
        client.publish(etcon_topic, String("offline").c_str(), true);
        et_on = false;
      }
    }
    else
    {
      dtostrf(et, 4, 1, buf_etemp);
      Heltec.display->drawString(40, 5, buf_etemp);
      if (et_on == false);
      {
        client.publish(etcon_topic, String("online").c_str(), true);
        et_on = true;
      }

      et_arr.push_back (et);
      if (et_arr.size () >= 5)
      {
        for (int i = 1; i < 5; ++i)
        {
          for (int r = 0; r < 5 - i; r++)
          {
            if (et_arr[r] < et_arr[r + 1])
            {
              float temp = et_arr[r];
              et_arr[r] = et_arr[r + 1];
              et_arr[r + 1] = temp;
            }
          }
        }
        dtostrf(et_arr[2], 0, 1, buf_mqtt_etemp);
        client.publish(etemperature_topic, String(buf_mqtt_etemp).c_str(), true);
        et_arr.clear();
      }
    }

    /************************* HUMIDITY MEDIAN AND SEND *********************************/
    if (isnan(h))
    {
      Heltec.display->drawString(78, 5, "Err");
    }
    else
    {
      dtostrf(h, 3, 0, buf_humi);
      Heltec.display->drawString(78, 5, buf_humi);

      h_arr.push_back (h);
      if (h_arr.size () >= 5)
      {
        for (int i = 1; i < 5; ++i)
        {
          for (int r = 0; r < 5 - i; r++)
          {
            if (h_arr[r] < h_arr[r + 1])
            {
              float temp = h_arr[r];
              h_arr[r] = h_arr[r + 1];
              h_arr[r + 1] = temp;
            }
          }
        }
        dtostrf(h_arr[2], 0, 0, buf_mqtt_humi);
        client.publish(humidity_topic, String(buf_mqtt_humi).c_str(), true);
        h_arr.clear();
      }
    }

    /************************* GAS MEDIAN AND SEND *********************************/
    if (millis() < (heating * 1000))
    {
      int progress = (100 * millis()) / (heating * 1000);
      Heltec.display->drawProgressBar(108, 9, 19, 8, progress);
    }
    else
    {
      g_per = (g * 100) / 1023;
      dtostrf(g_per, 3, 0, buf_gas);
      Heltec.display->drawString(107, 5, buf_gas);

      g_arr.push_back (g_per);
      if (g_arr.size () >= 5)
      {
        for (int i = 1; i < 5; ++i)
        {
          for (int r = 0; r < 5 - i; r++)
          {
            if (g_arr[r] < g_arr[r + 1])
            {
              float temp = g_arr[r];
              g_arr[r] = g_arr[r + 1];
              g_arr[r + 1] = temp;
            }
          }
        }
        dtostrf(g_arr[2], 0, 0, buf_mqtt_gas);
        client.publish(gas_topic, String(buf_mqtt_gas).c_str(), true);
        g_arr.clear();
      }
    }

    /************************* PRESSURE MEDIAN AND SEND *********************************/
    p_arr.push_back (p);
    if (p_arr.size () >= 5)
    {
      for (int i = 1; i < 5; ++i)
      {
        for (int r = 0; r < 5 - i; r++)
        {
          if (p_arr[r] < p_arr[r + 1])
          {
            float temp = p_arr[r];
            p_arr[r] = p_arr[r + 1];
            p_arr[r + 1] = temp;
          }
        }
      }
      dtostrf(p_arr[2], 0, 0, buf_mqtt_pres);
      client.publish(pressure_topic, String(buf_mqtt_pres).c_str(), true);
      p_arr.clear();
    }

    //display all above
    Heltec.display->display();

    /*   //SERIAL PRINT BLOCK
       Serial.print(t);
       Serial.print("  ");

       Serial.print(h);
       Serial.print("  ");

       Serial.print(p);
       Serial.print("  ");

       Serial.print(et);
       Serial.print("  ");

       Serial.print(g);
       Serial.print("  ");

       if (WiFi.status() == WL_CONNECTED)
       {
         Serial.print(WiFi.localIP());
       }
       else
       {
         Serial.print("Wi-fi is DOWN!");
       }

       Serial.print("  ");
       Serial.print(cal_arr);
       Serial.println("");
    */

  }
}
/*------------- FUNCTIONS -------------*/
String numberSign (float value)
{
  String sign;
  if (value > 0)
  {
    sign = "+";
  }
  else if (value < 0)
  {
    sign = "-";
  }
  else
  {
    sign = "=";
  }
  return sign;
}
