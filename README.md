# ESP32-Cam-Telegram
Demo of sending a photo from a ESP32-CAM acting as a Telegram BOT

https://github.com/witnessmenow/Universal-Arduino-Telegram-Bot/issues/186#issue-654370487

Bugfix on line 239 of .cpp 

Also demo sending a big .jpg two ways - one of which fails

Some other debugging changes - search for jz

Just add wifi name/pass and BOTtoken, then send /photo from your Telegram BOT and look at serial monitor for debug messages, and telegram for one picture

-------------------

ESP32-Cam3.ino takes the commands /qvga (320x240, about 4 kb) and /uxga (1600x1200, about 100kb) to send different sized photos 
- the qvga works on both send techqinues
- the uxga fails on second send technqiue
