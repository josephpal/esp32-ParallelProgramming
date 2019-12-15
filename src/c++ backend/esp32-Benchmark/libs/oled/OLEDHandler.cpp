/*
 * OLEDHandler.cpp
 *
 *  Created on: Dec 14, 2019
 *      Author: joseph
 */

#include "OLEDHandler.h"

OLEDHandler::OLEDHandler() {
	Serial.print("[oled] initialize oled display ...");

	screen = new SSD1306(0x3c, SDA, SCL);

	screen->init();

	screen->flipScreenVertically();
	screen->setFont(ArialMT_Plain_10);
	screen->setTextAlignment(TEXT_ALIGN_CENTER);

	Serial.println(" -> done.\n");
}

OLEDHandler::~OLEDHandler() {
	delete screen;
}

void OLEDHandler::drawProgressBar(float value) {
	drawProgressBar("", value);
}

void OLEDHandler::drawProgressBar(String customMsg, float value) {
	screen->clear();

	/*
	* 	drawProgressBar(x, y, width, height, value);
		parameter (p):
			p1: x      --> x axis coordinat (cartesic)
			p2: y      --> y axis coordinat (cartesic)
			p3: width   --> progressbar width
			p4: height  --> progressbar height
			p5: value   --> value to display as percentage
	*/
	screen->drawProgressBar(10, 32, 100, 10, value);
	screen->setTextAlignment(TEXT_ALIGN_CENTER);

	/*
	* 	drawString(x,y,text);
		parameter (p):
			p1: x      --> x axis coordinat (cartesic)
			p2: y      --> y axis coordinat (cartesic)
			p3: string --> text message to display
	*/
	screen->drawString(64, 15, customMsg + String(value) + "%");

	if(value == 0){
		screen->drawString(64, 45, "Starting ...");
	} else if(value == 100){
		screen->drawString(64, 45, "Finished");
	} else if(value > 1) {
		screen->drawString(64, 45, "Processing ...");
	}

	screen->display();
}

void OLEDHandler::clearScreen() {
	screen->clear();
	screen->display();
}

void OLEDHandler::displayText(String msg) {
	clearScreen();
	screen->setFont(ArialMT_Plain_10);
	screen->setTextAlignment(TEXT_ALIGN_CENTER);

	screen->drawRect (12, 15, 103, 28);
	screen->drawString(64, 15, msg);

	screen->display();
}
