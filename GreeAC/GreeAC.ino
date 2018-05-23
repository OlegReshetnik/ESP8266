/*
SonOff
	Generic ESP
	Памть 1M, DOUT 64 k SPIFF
	GPIO
	0 Main button
	1 TX
	3 RX
	12 Relay
	13 LED (inversed logic)
	14 Available in header
*/
#define LED_PIN			13
#define LED_ON			0
#define LED_BLINK_TIME	1000 // 1 sec

#define LIGHT_PIN		12
#define LIGHT_ON		1

#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <EspFileServer.h>
#include <Ticker.h>

#define IR_LED 14
#define SEND_GREE
#include <IRremoteESP8266.h>
#include <IRsend.h>
#include <ir_Gree.h>

static const char text_plain[] PROGMEM = "text/plain; charset=UTF-8";

ESP8266WebServer server(80);
EspFileServer fileServer( &server );
Ticker led;

IRGreeAC gree(IR_LED);

void InitWiFi()
{
	const char * WiFi_Name = "Oleg_Home";
	const char * WiFi_Pass = "lbvf1234";
	IPAddress staticIP(192,168,1,200);
	IPAddress gateway(192,168,1,1);
	IPAddress subnet(255,255,255,0);
	WiFi.config(staticIP, gateway, subnet);
	WiFi.begin(WiFi_Name, WiFi_Pass);
	while(WiFi.waitForConnectResult() != WL_CONNECTED) WiFi.begin(WiFi_Name, WiFi_Pass);
}

void blinker() { digitalWrite( LED_PIN, !digitalRead( LED_PIN ) ); }

bool statusLight() { return (bool)digitalRead( LIGHT_PIN ); }

void doLight() { digitalWrite( LIGHT_PIN, !digitalRead( LIGHT_PIN ) ); }

void climate()
{
	if( server.hasArg("info") )
	{
		server.send( 200, FPSTR(text_plain),
		"P=" + String(gree.getPower()?'1':'0') +
		"&M=" + String((int)gree.getMode() ) +
		"&F=" + String((int)gree.getFan() ) +
		"&T=" + String((int)gree.getTemp() ) +
		"&S=" + String(gree.getSwingVerticalAuto()?'1':'0') +
		"&SL=" + String(gree.getSleep()?'1':'0') +
		"&L=" + String(statusLight() ? '1' : '0') );
	}
	else if( server.hasArg("L") )
	{
		doLight();
		server.send( 200, FPSTR(text_plain), "L=" + String(statusLight() ? '1' : '0') );
	}
	else {
		if( server.hasArg("P") ) gree.setPower( (bool)server.arg("P").toInt() );
		if( server.hasArg("M") ) gree.setMode( (uint8_t)server.arg("M").toInt() );
		if( server.hasArg("F") ) gree.setFan( (uint8_t)server.arg("F").toInt() );
		if( server.hasArg("T") ) gree.setTemp( (uint8_t)server.arg("T").toInt() );
		if( server.hasArg("S") ) gree.setSwingVertical( (bool)server.arg("S").toInt(), (uint8_t)server.arg("S").toInt() );
		if( server.hasArg("SL") ) gree.setSleep( (bool)server.arg("SL").toInt() );
		gree.send();
		server.send( 200, FPSTR(text_plain), "" );
	}
}

void setup(void)
{
	pinMode( LED_PIN, OUTPUT );
	digitalWrite( LED_PIN, LED_ON );
	led.attach_ms( LED_BLINK_TIME, blinker );

	pinMode( LIGHT_PIN, OUTPUT );
	digitalWrite( LIGHT_PIN, LIGHT_ON ); // включаем свет при включении
	
	InitWiFi();
	server.on( "/climate", HTTP_GET, climate );
	gree.begin();
	server.begin();

	gree.setFan(1);
	gree.setMode(GREE_COOL);
	gree.setTemp(22);
}

void loop(void)
{
	server.handleClient();
}

