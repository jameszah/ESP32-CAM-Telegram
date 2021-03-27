/*******************************************************************

   A Telegram bot for taking a photo with an ESP32Cam

   Parts used:
   ESP32-CAM module* - http://s.click.aliexpress.com/e/bnXR1eYs

    = Affiliate Links

   Note:
   - Make sure that you have either selected ESP32 Wrover Module,
           or another board which has PSRAM enabled
   - Choose "Huge App" partion scheme

   Some of the camera code comes from Rui Santos:
   https://randomnerdtutorials.com/esp32-cam-take-photo-save-microsd-card/

   Written by Brian Lough
    YouTube: https://www.youtube.com/brianlough
    Tindie: https://www.tindie.com/stores/brianlough/
    Twitter: https://twitter.com/witnessmenow

    Aug 7, 2020 - jz
    Mods to library and example to demonstrate
     - bugfix with missing println statement
     - method to send big and small jpegs
     - sending a caption with a pictire

    Mar 26, 2021
    Mods for esp32 version 1.05
    See line 250 of UniversalTelegramBot.cpp in the current github software
    https://github.com/witnessmenow/Universal-Arduino-Telegram-Bot/issues/235#issue-842397567
    See https://github.com/witnessmenow/Universal-Arduino-Telegram-Bot/blob/65e6f826cbab242366d69f00cebc25cdd1e81305/src/UniversalTelegramBot.cpp#L250

Using library WiFi at version 1.0 in folder: C:\ArduinoPortable\arduino-1.8.13\portable\packages\esp32\hardware\esp32\1.0.6\libraries\WiFi 
Using library WiFiClientSecure at version 1.0 in folder: C:\ArduinoPortable\arduino-1.8.13\portable\packages\esp32\hardware\esp32\1.0.6\libraries\WiFiClientSecure 
Using library ArduinoJson at version 6.16.1 in folder: C:\ArduinoPortable\arduino-1.8.13\portable\sketchbook\libraries\ArduinoJson 

    
*******************************************************************/

// ----------------------------
// Standard Libraries - Already Installed if you have ESP32 set up
// ----------------------------

#include <WiFi.h>
#include <WiFiClientSecure.h>
#include "esp_camera.h"

// ----------------------------
// Additional Libraries - each one of these will need to be installed.
// ----------------------------

//#include <UniversalTelegramBot.h>
#include "UniversalTelegramBot.h"
// Library for interacting with the Telegram API
// Search for "Telegegram" in the Library manager and install
// The universal Telegram library
// https://github.com/witnessmenow/Universal-Arduino-Telegram-Bot

#include <ArduinoJson.h>
// Library used for parsing Json from the API responses
// Search for "Arduino Json" in the Arduino Library manager
// https://github.com/bblanchon/ArduinoJson


//------- Replace the following! ------

//#define CAMERA_MODEL_WROVER_KIT
//#define CAMERA_MODEL_ESP_EYE
//#define CAMERA_MODEL_M5STACK_PSRAM
//#define CAMERA_MODEL_M5STACK_WIDE
#define CAMERA_MODEL_AI_THINKER

// Initialize Wifi connection to the router
char ssid[] = "jzjz";     // your network SSID (name)
char password[] = "jzjz"; // your network key



// Initialize Telegram BOT
#define BOTtoken "XXXXXXXXX:XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX"  // your Bot Token (Get from Botfather)

//------- ------------------------ ------

#include "camera_pins.h"
#include "camera_code.h"

#define FLASH_LED_PIN 4

WiFiClientSecure client;
UniversalTelegramBot bot(BOTtoken, client);

int Bot_mtbs = 1000; //mean time between scan messages
long Bot_lasttime;   //last time messages' scan has been done

bool flashState = LOW;

camera_fb_t * fb = NULL;

bool isMoreDataAvailable();
//byte* getNextBuffer();
//int getNextBufferLen();

////////////////////////////////  send photo as 512 byte blocks -
int currentByte;
uint8_t* fb_buffer;
size_t fb_length;

bool isMoreDataAvailable() {
  return (fb_length - currentByte);
}

uint8_t getNextByte() {
  currentByte++;
  return (fb_buffer[currentByte - 1]);
}

bool dataAvailable = false;

//////////////////////////////// send photo as 1 giant block

bool isMoreDataAvailableXXX() {
  if (dataAvailable)
  {
    dataAvailable = false;
    return true;
  } else {
    return false;
  }
}

byte* getNextBuffer() {
  if (fb) {
    return fb->buf;
  } else {
    return nullptr;
  }
}

int getNextBufferLen() {
  if (fb) {
    return fb->len;
  } else {
    return 0;
  }
}

///////////////////////////////


void handleNewMessages(int numNewMessages) {
  Serial.println("handleNewMessages");
  Serial.println(String(numNewMessages));

  for (int i = 0; i < numNewMessages; i++) {
    String chat_id = String(bot.messages[i].chat_id);
    String text = bot.messages[i].text;

    Serial.printf("\nGot a message %s\n", text);

    String from_name = bot.messages[i].from_name;
    if (from_name == "") from_name = "Guest";

    String hi = "hi ... \nGot: ";
    hi += text;
    bot.sendMessage(chat_id, hi, "Markdown");

    if (text == "/flash") {
      flashState = !flashState;
      digitalWrite(FLASH_LED_PIN, flashState);
    }

    if (text == "/uxga" || text == "/photo" || text == "/caption" ) {

      fb = NULL;

      sensor_t * s = esp_camera_sensor_get();
      //s->set_framesize(s, FRAMESIZE_QVGA);  // jz  qvga 320x250   4 kb
      s->set_framesize(s, FRAMESIZE_UXGA);  // jz  uxga 1600x1200 100 kb

      fb = esp_camera_fb_get();
      esp_camera_fb_return(fb);
      fb = esp_camera_fb_get();
      esp_camera_fb_return(fb);

      // Take Picture with Camera
      fb = esp_camera_fb_get();
      if (!fb) {
        Serial.println("Camera capture failed");
        bot.sendMessage(chat_id, "Camera capture failed", "");
        return;
      }

      currentByte = 0;
      fb_length = fb->len;
      fb_buffer = fb->buf;

      if (text == "/caption") {
        Serial.println("\n\n\nSending UXGA with caption");
        Serial.println("\n>>>>> Sending as 512 byte blocks, with a caption, and with jzdelay of 0, bytes=  " + String(fb_length));

        String sent = bot.sendMultipartFormDataToTelegramWithCaption("sendPhoto", "photo", "img.jpg",
                      "image/jpeg", "I am the caption!", chat_id, fb_length,
                      isMoreDataAvailable, getNextByte, nullptr, nullptr);

        Serial.println("done!");
      } else {

        Serial.println("\n\n\nSending UXGA");
        Serial.println("\n>>>>> Sending as 512 byte blocks, with jzdelay of 0, bytes=  " + String(fb_length));

        bot.sendPhotoByBinary(chat_id, "image/jpeg", fb_length,
                              isMoreDataAvailable, getNextByte,
                              nullptr, nullptr);

        dataAvailable = true;
        /*
          Serial.println("\n>>>>>Sending as one block, bytes=  " + String(fb_length));

          Serial.println(">>>> This should delay 1 minute and then fail");

          bot.sendPhotoByBinary(chat_id, "image/jpeg", fb->len,
                              isMoreDataAvailableXXX, nullptr,
                              getNextBuffer, getNextBufferLen);

          Serial.println(">>>> That should have failed");
        */
        Serial.println("done!");
      }

      esp_camera_fb_return(fb);
    }



    if (text == "/qvga" ) {

      fb = NULL;

      sensor_t * s = esp_camera_sensor_get();
      s->set_framesize(s, FRAMESIZE_QVGA);  // jz  qvga 320x250   4 kb

      fb = esp_camera_fb_get();   // takes a pic or two for camera to adjust
      esp_camera_fb_return(fb);
      fb = esp_camera_fb_get();
      esp_camera_fb_return(fb);

      Serial.println("\n\n\nSending QVGA");

      // Take Picture with Camera
      fb = esp_camera_fb_get();
      if (!fb) {
        Serial.println("Camera capture failed");
        bot.sendMessage(chat_id, "Camera capture failed", "");
        return;
      }

      currentByte = 0;
      fb_length = fb->len;
      fb_buffer = fb->buf;

      Serial.println("\n>>>>> Sending as 512 byte blocks, with jzdelay of 0, bytes=  " + String(fb_length));

      bot.sendPhotoByBinary(chat_id, "image/jpeg", fb_length,
                            isMoreDataAvailable, getNextByte,
                            nullptr, nullptr);
      /*
      dataAvailable = true;
      Serial.println("\n>>>>>Sending as one block, bytes=  " + String(fb_length));


      bot.sendPhotoByBinary(chat_id, "image/jpeg", fb->len,
                            isMoreDataAvailableXXX, nullptr,
                            getNextBuffer, getNextBufferLen);

      //Serial.println("\n\n\nThat should have failed");

      Serial.println("done!");
      */
      
      esp_camera_fb_return(fb);
    }

    if (text == "/start") {
      String welcome = "Welcome to the ESP32Cam Telegram bot.\n\n";
      welcome += "/photo : will take a photo\n";
      welcome += "/flash : toggle flash LED (VERY BRIGHT!)\n";
      welcome += "/qvga : small photo \n";
      welcome += "/uxga : big photo\n";
      welcome += "/caption : big phot with caption\n";
      bot.sendMessage(chat_id, welcome, "Markdown");
    }
  }
}

 

void setup() {
  Serial.begin(115200);

  pinMode(FLASH_LED_PIN, OUTPUT);
  digitalWrite(FLASH_LED_PIN, flashState); //defaults to low

  if (!setupCamera()) {
    Serial.println("Camera Setup Failed!");
    while (true) {
      delay(100);
    }
  }

  // Attempt to connect to Wifi network:
  Serial.print("Connecting Wifi: ");
  Serial.println(ssid);

  // Set WiFi to station mode and disconnect from an AP if it was Previously
  // connected
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  // Make the bot wait for a new message for up to 60seconds
  bot.longPoll = 60;

  client.setInsecure();
}

void loop() {
  if (millis() > Bot_lasttime + Bot_mtbs)  {
    int numNewMessages = bot.getUpdates(bot.last_message_received + 1);

    while (numNewMessages) {
      Serial.println("got response");
      handleNewMessages(numNewMessages);
      numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    }

    Bot_lasttime = millis();
  }
}
