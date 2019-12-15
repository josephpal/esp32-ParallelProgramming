/*
 * esp32-BasicParallelProcessing.ino
 *
 *  Created on: 	Dec 05, 2019
 *  				M. Belean, F.J. Pal
 *
 *  modified on:	Dec 09, 2019
 *  
 *  description:
 *  
 *  version:		0.5
 *  
 *  notifications:
 *  
 *  src:	[1]
 *  		[2]
 *
 */

#include "libs/benchmark/BenchmarkHandler.h"
#include "libs/websocket/WebsocketHandler.h"

/* global pointers, this is actually necessary to get access to the benchmark- and websocket resources */
/* from all threads running on different cores */

BenchmarkHandler *handler;
WebsocketHandler *wsHandler;

/* --------------------------------------------------------------------------------------------------- */

void setup() {
	/* serial communication */
    Serial.begin(115200);

    /* initialize internal storage */
    if(!SPIFFS.begin(true)){
		Serial.println("[main] SPIFFS mount failed.");
		return;
	}

    delay(1000);

    handler = new BenchmarkHandler(16);

    /* first we run the Computation example, afterwards we perform the Mandelbrot in parallel */
    /* execution time results will be stored in results.txt (first line, first example; second line, second example) */
    /* The Mandelbrot fractal will be stored in mandelbrot.ppm */
    handler->startComputationBenchmark();
    handler->displayResult();

    handler->startMandelbrotBenchmark();
    handler->displayResult();

    /* our benchmark runs are over, if they terminated successfully, we can finalize and initialize the UI */
    handler->finalize();
}

void loop() {
	/*
	 * delete main loop task to disable watchdog core panic errors
	 * other possibilities:
	 * 		// main loop in idle task
	 * 		vTaskDelay(portMAX_DELAY);
	 * 		delay(1000);
	 *
	 *		vTaskDelete(NULL);
	 *
	 */
	handler->displayUI();
}
