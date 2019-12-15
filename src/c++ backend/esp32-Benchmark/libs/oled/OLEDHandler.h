/*
 * OLEDHandler.h
 *
 *  Created on: Dec 14, 2019
 *      Author: joseph
 */

#ifndef LIBS_OLED_OLEDHANDLER_H_
#define LIBS_OLED_OLEDHANDLER_H_

#include "SSD1306.h"
#include "Arduino.h"

class OLEDHandler {
public:
	OLEDHandler();
	virtual ~OLEDHandler();

	void drawProgressBar(float value);

	void drawProgressBar(String customMsg, float value);

	void clearScreen();

	void displayText(String msg);
private:
	SSD1306 * screen;
};

#endif /* LIBS_OLED_OLEDHANDLER_H_ */
