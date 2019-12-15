/*
 * Mandelbrot.cpp
 *
 *  Created on: Dec 2, 2019
 *      Author: joseph
 */

#include "Mandelbrot.h"

Mandelbrot::Mandelbrot(unsigned width, unsigned height) {
	this->width = width;
	this->height = height;

	this->minX = 0;
	this->maxX = 0;
	this->minY = 0;
	this->maxY = 0;

	numOfCombinedBits = HelperFunctions::getInstance()->gd(width, 30);

	queue = NULL;
	result = "";
}

Mandelbrot::Mandelbrot(unsigned width, unsigned height, unsigned minX, unsigned maxX, unsigned minY, unsigned maxY, QueueHandle_t &refParamQueue) {
	this->width = width;
	this->height = height;

	this->minX = minX;
	this->maxX = maxX;
	this->minY = minY;
	this->maxY = maxY;

	numOfCombinedBits = HelperFunctions::getInstance()->gd(width, 30);

	queue = new QueueHandle_t();
	*queue = refParamQueue;

	result = "";
}

Mandelbrot::~Mandelbrot() {
	delete queue;
}

void Mandelbrot::setCompressionLevel(int numOfCombinedBits) {
	this->numOfCombinedBits = HelperFunctions::getInstance()->gd(width, numOfCombinedBits);
}

void Mandelbrot::calculateCompressedImage() {
	// calculation status
	long status = 0;

	// fractal properties
	double MinRe = -1.2;
	double MaxRe = 1.2;
	double MinIm = -1.2;
	double MaxIm = 1.2;
	double Re_facor = (MaxRe - MinRe) / (width - 1);
	double Im_factor = (MaxIm - MinIm) / (height - 1);

	// implementation of the mathematical limes -> to infinite (in this case 34)
	unsigned MaxIterations = 34;

	// compression buffer
	int bufIndex = 0;
	int valCoded = 0;

	for (unsigned y = minY; y < maxY; ++y) {
		double c_im = MaxIm - y * Im_factor;

		for (unsigned x = minX; x < maxX; ++x) {
			double c_re = MinRe + x * Re_facor - 0.65; // image on Re-axis move with 0.65

			double Z_re = c_re, Z_im = c_im;
			bool isInside = true;

			for (unsigned n = 0; n < MaxIterations; ++n) {
				double Z_re2 = Z_re * Z_re, Z_im2 = Z_im * Z_im;

				if (Z_re2 + Z_im2 > 4) {
					isInside = false;
					break;
				}

				Z_im = 2 * Z_re * Z_im + c_im;
				Z_re = Z_re2 - Z_im2 + c_re;
			}

			if (isInside) {
				valCoded = (valCoded << 1) + 1;
			} else {
				valCoded = (valCoded << 1);
			}

			bufIndex++;

			if(bufIndex > numOfCombinedBits - 1) {
				bufIndex = 0;
				result += String(valCoded) + "\n";
				valCoded = 0;
			}
		}
	}

	/*
	 *  push back to the receiver of the queue that the result is calculated
	 *  "he" is actually waiting for the ping and will use the part result in form
	 *  of the referenced buffer (constructor).
	 */

	xQueueSend(*queue, &status, portMAX_DELAY);
}

void Mandelbrot::createPPMFile(String filename) {
	Serial.print("[mandelbrot] creating *.ppm image file " + String(width) + "x" + String(height));

	/*
	 * 	filename.ppm file:
	 *
	 * 	---------------------------------
	 * 	P3
	 * 	WIDTH HEIGHT
	 * 	1
	 *
	 * 	... -> Content
	 * 	---------------------------------
	 *
	 */

	String header = String("P3") + String("\n") + String(width) + String(" ") + String(height)
			+ String("\n") + String(1) + String("\n\n");

	if( !spiffs.writeFile(SPIFFS, filename.c_str(), header) ) {
		Serial.println(" -> FAILED.");
		return;
	}

	Serial.println(" -> done.");

	delay(100);
}

void Mandelbrot::writeToPPMFile(String filename) {
	Serial.print("[mandelbrot] append to *.ppm image file ...");

	if( !spiffs.appendFile(SPIFFS, filename.c_str(), result) ) {
		Serial.println(" -> FAILED.");
		return;
	}

	Serial.println(" -> done.");
}

void Mandelbrot::readPPMFile(String filename) {
	spiffs.readFile(SPIFFS, filename.c_str());
}
