/*
 * Mandelbrot.h
 *
 *  Created on: Dec 2, 2019
 *      Author: joseph
 *
 *  src:
 *
 *  [1]	http://warp.povusers.org/Mandelbrot/
 *  [2]	https://medium.com/farouk-ounanes-home-on-the-internet/mandelbrot-set-in-c-from-scratch-c7ad6a1bf2d9
 *  [3] https://stackoverflow.com/questions/53381279/mandelbrot-image-generator-in-c-using-multi-threading-overwrites-half-the
 *  [4] https://plus.maths.org/content/what-mandelbrot-set
 *  [5] https://de.wikipedia.org/wiki/Mandelbrot-Menge 
*/

#ifndef MANDELBROT_H_
#define MANDELBROT_H_

#include <Arduino.h>
#include "HelperFunctions.h"
#include "../memory/CSPIFFS.h"

class Mandelbrot {
public:
	/** default constructor; image width and height will be set @param and minX, maxX, minY, maxY will be set to 0.
	 *  The constructor will compute the maximum number of combined bits with the help of gd(width, 30). In this case,
	 *  we define the maximum available number of combined bits to 30; nevertheless this number can be smaller because
	 *  width / numOfCombinedBits has to be even! This is catched by the gd() function as well.
	 *
	 *  @param	specify the width of the image
	 *  @param	specify the height of the image
	 *  @return ---
	*/
	Mandelbrot(unsigned width, unsigned height);

	/** Overloaded constructor; image width and height will be set on @param; the image has to be a square (width = height!).
	 * 	In case if width != height, height will be set to width
	 *
	 *  @param	specify the width of the image
	 *  @param	specify the height of the image
	 *  @param	specify the borders of the part image (minX, maxX, minY and maxY)
	 *  @param	queue 	TODO
	 *  @param	buf 	TODO
	 *  @return ---
	*/
	Mandelbrot(unsigned width, unsigned height, unsigned minX, unsigned maxX, unsigned minY, unsigned maxY, QueueHandle_t &refParamQueue);
	virtual ~Mandelbrot();

	/** function to set the number of bits, which will be combined to an integer value for writing into the *.ppm file.
	 *  Depending on the width/height, the passed parameter won't fit, in this case the value will be automatic decrease until it reaches
	 *  a even solution of width / @param -> this is achieved by using the function gd(int a, int b) in "HelperFunctions.h".
	 *
	 *  If not defined, the constructor will compute the maximum number of combined bits with the help of gd(width, 30). In this case,
	 *  we define the maximum available number of combined bits to 30; nevertheless this number can be smaller because width / numOfCombinedBits
	 *  has to be even!
	 *
	 *  @param	specify the number of combined bits in the bitstream of the *.ppm file
	 *  @return ---
	*/
	void setCompressionLevel(int numOfCombinedBits);

	/** function to calculate a part image of the mandelbrot fractal between minX, maxX, minY and maxY; the result will be returned as
	 * 	an compressed string, which contains a stream of int values regarding to maximum available value (2^numOfCombinedBits).
	 *
	 *  @param	specify the number of combined bits in the bitstream of the *.ppm file
	 *  @return &result (property) will contain the int data stream of the part row picture
	*/
	void calculateCompressedImage();

	/** ...
	 *
	 *  @param
	 *  @return
	*/
	void createPPMFile(String filename);

	/** ...
	 *
	 *  @param
	 *  @return
	*/
	void writeToPPMFile(String filename);

	/** ...
	 *
	 *  @param
	 *  @return
	*/
	void readPPMFile(String filename);

private:
	unsigned width;
	unsigned height;

	unsigned minX, maxX, minY, maxY;

	int numOfCombinedBits;

	QueueHandle_t *queue;
	String result;

	CSPIFFS spiffs;
};

#endif /* MANDELBROT_H_ */
