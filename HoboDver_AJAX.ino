#include <ESP8266WiFi.h>
#include "FS.h"
#include <WiFiClient.h>
#include <TimeLib.h>
#include <NtpClientLib.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <ESP8266mDNS.h>
#include <Ticker.h>
#include <ArduinoOTA.h>
#include <ArduinoJson.h>
#include "FSWebServerLib.h"                       //async web server based on ESP8266WebServer 
#include <Hash.h>
#include "HoboDver_AJAX.h"

String  strT = "n/a";        //string to json value "temp"

motor_states rot_direction = motor_stop;

//------------------------------------------------------------------------------------

void mosfet(enum mosfet_states state)
{
	switch (state)
	{
	case turn_off: digitalWrite(D5, HIGH); break;
	case turn_on:  digitalWrite(D5, LOW); break;
	}
}

void rotate_motor(enum motor_states dir)
{
	Serial.print("direction changed to ");
	Serial.println(dir, DEC);

	switch (dir)
	{
	case motor_forward: digitalWrite(D7, HIGH); //rotate forward
		digitalWrite(D8, LOW);
		break;
	case motor_backward:digitalWrite(D7, LOW); //rotate backward
		digitalWrite(D8, HIGH);
		break;
	case motor_stop:
	default:           digitalWrite(D7, HIGH); //STOP
		digitalWrite(D8, HIGH);
		break;
	}
}

//------------------------------------------------------------------------------------

void setup()// WiFi is started inside library
{
	Serial.begin(115200);
	Serial.println("\r\n\r\n-[INITIALIZING]-");
	Serial.println("HoboDver_AJAX_client v0.1a compiled " __DATE__ "," __TIME__);

	pinMode(D0, INPUT);  //koncevik _PULLUP
	pinMode(D1, INPUT);  //koncevik
	pinMode(D6, INPUT);  //knopka

	pinMode(D5, OUTPUT); //mosfet_12v
	digitalWrite(D5, HIGH);    //OFF on boot
	pinMode(D7, OUTPUT); //h-bridge
	digitalWrite(D7, HIGH);
	pinMode(D8, OUTPUT); //h-bridge
	digitalWrite(D8, HIGH);

	mosfet(turn_off);

	SPIFFS.begin(); // Not really needed, checked inside library and started if needed
	ESPHTTPServer.begin(&SPIFFS);

	Serial.println("\r\n-[INIT DONE]-");
}

//------------------------------------------------------------------------------------

void loop()
{
	static const unsigned long REFRESH_INTERVAL = 1000; // ms
	static unsigned long lastRefreshTime = 0;

	if (millis() - lastRefreshTime >= REFRESH_INTERVAL)
	{
		lastRefreshTime += REFRESH_INTERVAL;
		switch (rot_direction)
		{
		case motor_forward: rot_direction = motor_backward;
			break;
		case motor_backward:rot_direction = motor_forward;
			break;
		case motor_stop:    break;
		}
		rotate_motor(rot_direction);
	}

	if (digitalRead(D0) == LOW) { mosfet(turn_on); rot_direction = motor_forward; }
	if (digitalRead(D1) == LOW) { mosfet(turn_off); rot_direction = motor_stop; }

	ESPHTTPServer.handle(); // DO NOT REMOVE. Attend OTA update from Arduino IDE
}