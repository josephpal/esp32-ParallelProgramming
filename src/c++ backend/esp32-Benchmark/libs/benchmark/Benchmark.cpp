/*
 * Benchmark.cpp
 *
 *  Created on: Nov 11, 2019
 *      Author: joseph
 */

#include "Benchmark.h"

void computationTask(void * params) {

	Computation *comp = (Computation*) params;

	comp->compute();
	yield();

	delete comp;

	vTaskDelete( NULL );
}

void mandelbrotTask(void * params) {

	Mandelbrot *mandelbrot = (Mandelbrot*) params;

	mandelbrot->calculateCompressedImage();

	vTaskDelete( NULL );
}

/* -------------------------------------------------------------------------------------------------------------------------------------------- */

Benchmark::Benchmark(int durationCycles, int coolDownTimer, int queueSize) {
	this->durationCycles = durationCycles > 0 ? durationCycles : 4;
	this->coolDownTimer = coolDownTimer > 0 ? coolDownTimer : 1500;

	this->upperLimitSum = 50000;
	this->upperLimitMul = 10000;

	queue = new QueueHandle_t();
	this->queueSize = queueSize;

	//TODO long or String?
	*queue = xQueueCreate( queueSize, sizeof( long ) );

	maxNumOfThreads = queueSize;
}

Benchmark::~Benchmark() {
	Serial.println();

	delete queue;
}

/* ---------------------------------------------- perform benchmark for computation example (sum) --------------------------------------------- */

float Benchmark::performComputationBenchmark(int numOfThreads) {
	delay(coolDownTimer);

	if (numOfThreads > maxNumOfThreads) {
		Serial.println("[bench] error: Number of maximal threads reached!");
		Serial.print("[bench] error: running with maxNumOfThreads => ");
		Serial.println(maxNumOfThreads);
		return runComputationWith(maxNumOfThreads, durationCycles);
	} else {
		return runComputationWith(numOfThreads, durationCycles);
	}
}

float Benchmark::runComputationWith(int numOfThreads, int numOfCycles) {
	long average = 0;
	long result = 0;

	for (int i = 0; i < numOfCycles; i++) {

		// declarations
		Computation *c[numOfThreads];

		/*
		 * =============================================================================
		 * =================================
		 */

		long start = millis();

		for (int j = 0; j < numOfThreads; j++) {

			int lowLimSum = (getUpperLimitSum() / numOfThreads) * j;
			int uppLimSum = (getUpperLimitSum() / numOfThreads) * (j + 1);
			int uppLimMul = getUpperLimitMul();

			if(getUpperLimitSum() % numOfThreads != 0 && j == numOfThreads - 1) {
				uppLimSum += (getUpperLimitSum() % numOfThreads);
			}

			c[j] = new Computation(lowLimSum, uppLimSum, 0, uppLimMul, *queue);

			Serial.print("[task] Thread ");
			Serial.print(j);
			Serial.print(" on core [");
			Serial.print((j+2)%2);
			Serial.print("] -> ");
			Serial.print(c[j]->getLoopOneStart());
			Serial.print(";");
			Serial.println(c[j]->getLoopOneEnd());

			// Serial.print(" -> stack size allocation : ");
			// Serial.println(sizeof(c[j]) * 32 * 8);				/* https://github.com/espressif/arduino-esp32/issues/1745 */
																	/* https://www.arduino.cc/en/pmwiki.php?n=Reference/Sizeof */
																	/* https://www.freertos.org/vTaskGetInfo.html */
																	/* https://www.esp32.com/viewtopic.php?t=4295 */
																	/* Serial.println(uxTaskGetStackHighWaterMark(NULL)); in task for left stack size */

			xTaskCreatePinnedToCore(
						computationTask,     						/* Function to implement the task */
						(j+2)%2 ==  0 ? "calcTask0" : "calcTask1",  /* Name of the task */
						sizeof(c[j]) * 32 * 8,            			/* Stack size in words: */
						 	 	 	 	 	 	 	 	 	 	 	/* -> size in words of the computation object: sizeof(c[j]) * 32 */
																	/* -> multiple by 8 for secure issues (used stack size will increase during runtime) */
																	/* => total allocated stack size: 1024 */
						c[j],             							/* Task input parameter */
						0,                							/* Priority of the task */
						NULL,             							/* Task handle. */
						(j+2)%2);         							/* Core where the task should run -> toggle between 0,1 if user try */
																	/* to run multiple threads on a dual core processor */
		}

		for (int k = 0; k < numOfThreads; k++) {
			long partResult;

			/*
			 * 	regarding to src [3]: "For the last argument, the value is specified in ticks, and we will
			 * 						   pass the value portMAX_DELAY, meaning that we will wait indefinitely
			 * 						   in case the queue is fullor we receive an message object."
			 *
			 * 	So this function call is a blocking call and the current (main) thread will wait until he received a new message
			 * 	from the queue. In this case, we can add the current part result to the main result:
			 * 	-> result += partResult
			 */
			xQueueReceive(*queue, &partResult, portMAX_DELAY);

			Serial.print("[bench] Received (part) result from thread ");
			Serial.print(k);
			Serial.print(" on core ");
			Serial.println((k+2)%2);

			result += partResult;
		}


		long end = millis();

		average += end - start;

		/*
		 * =============================================================================
		 * =================================
		 */
	}

	Serial.print("[bench] Number of threads: ");
	Serial.print(numOfThreads);
	Serial.print("; total time: ");
	Serial.print(average / numOfCycles);
	Serial.println(" ms.");

	Serial.print("[bench] => Result: ");
	Serial.println(result / numOfCycles);

	return average / numOfCycles;
}

int Benchmark::getUpperLimitMul() {
	return upperLimitMul;
}

void Benchmark::setUpperLimitMul(int upperLimitMul) {
	this->upperLimitMul = upperLimitMul;
}

int Benchmark::getUpperLimitSum() {
	return upperLimitSum;
}

void Benchmark::setUpperLimitSum(int upperLimitSum) {
	this->upperLimitSum = upperLimitSum;
}

/* -------------------------------------------------------------------------------------------------------------------------------------------- */

/* --------------------------------------------- perform benchmark for Mandelbrot example (fractal) ------------------------------------------- */

float Benchmark::performMandelbrotBenchmark(String filename, unsigned int width, unsigned int height, int numOfThreads) {
	delay(coolDownTimer);

	if (numOfThreads > maxNumOfThreads) {
		Serial.println("[bench] error: Number of maximal threads reached!");
		Serial.print("[bench] error: running with maxNumOfThreads => ");
		Serial.println(maxNumOfThreads);
		return runMandelbrotWith(filename, width, height, maxNumOfThreads, durationCycles);
	} else {
		return runMandelbrotWith(filename, width, height, numOfThreads, durationCycles);
	}
}

float Benchmark::runMandelbrotWith(String ppmFilename, unsigned int width, unsigned int height, int numOfThreads, int numOfCycles) {

	/*
	 * 	sub image coordinate computation (e.g. 600x600, 5 threads) -> row method
	 *
	 *	-- x
	 *	|
	 *	y
	 *
	 *	P0a(0|0)
	 *		x________________________________
	 * 		|								|
	 * 		|				0				|
	 * 		|_______________________________|
	 *										x
	 *	P1a(0|150)						P0a(600|150)
	 *		x________________________________
	 * 		|								|
	 * 		|				1				|
	 * 		|_______________________________|
	 *										x
	 *	P2a(0|300)						P1e(600|300)
	 *		x________________________________
	 * 		|								|
	 * 		|				2				|
	 * 		|_______________________________|
	 * 										x
	 * 						.			P2e(600|450)
	 * 						.
	 * 						.
	 *		_________________________________
	 * 		|								|
	 * 		|				3				|
	 * 		|_______________________________|
	 *		_________________________________
	 * 		|								|
	 * 		|				4				|
	 * 		|_______________________________|
	 *
	 */

	long average = 0;
	float execTime = 0;
	int devider = numOfThreads;

	/* buffer for the results of the threads */
	String resultBuffer[numOfThreads];

	Serial.println("[bench] Creating compressed image ...");
	Serial.println("[bench] sub images are arranged as " + String(devider) + "x rows.");

	/* combine maximal compressionLevel bits of the bitstream (rgb content of the *.ppm file) to a new number */
	/* e.g. 0110 0010 ... -> 10 2 ... */
	int numOfCombinedBits = HelperFunctions::getInstance()->gd(width, 30);

	Serial.println("[bench] compression level (bits combined): " +  String(numOfCombinedBits));

	/* declaration of task objects */
	Mandelbrot *m[numOfThreads];

	for (int i = 0; i < numOfCycles; i++) {
		/*
		 * =============================================================================
		 * =================================
		 */

		long start = millis();

		for (int j = 0; j < numOfThreads; j++) {

			unsigned minX = 0;
			unsigned maxX = width;
			unsigned minY = j * (height / numOfThreads);
			unsigned maxY = (j+1) * (height / numOfThreads);

			if(height % numOfThreads != 0 && j == numOfThreads - 1) {
				maxY += (height % numOfThreads);
			}

			m[j] = new Mandelbrot(width, height, minX, maxX, minY, maxY, *queue);

			Serial.print("[task] Thread ");
			Serial.print(j);
			Serial.print(" on core [");
			Serial.print((j+2)%2);
			Serial.print("] -> ");
			Serial.println("Calculating (" + String(minX) + "," + String(minY) + ") to ("
					+ String(maxX) + "," + String(maxY) + ").");

			xTaskCreatePinnedToCore(
						mandelbrotTask,     						/* Function to implement the task */
						(j+2)%2 ==  0 ? "calcTask0" : "calcTask1",  /* Name of the task */
						2048,            							/* Stack size in words: */
																	/* -> size in words of the computation object: sizeof(c[j]) * 32 */
																	/* -> multiple by 8 for secure issues (used stack size will increase during runtime) */
																	/* => total allocated stack size: 1024 */
						m[j],             							/* Task input parameter */
						0,                							/* Priority of the task */
						NULL,             							/* Task handle. */
						(j+2)%2);         							/* Core where the task should run -> toggle between 0,1 if user try */
																	/* to run multiple threads on a dual core processor */
		}

		for (int k = 0; k < numOfThreads; k++) {
			long threadFinished;

			/*
			 * 	regarding to src [3]: "For the last argument, the value is specified in ticks, and we will
			 * 						   pass the value portMAX_DELAY, meaning that we will wait indefinitely
			 * 						   in case the queue is fullor we receive an message object."
			 *
			 * 	So this function call is a blocking call and the current (main) thread will wait until he received a new message
			 * 	from the queue.
			 */
			xQueueReceive(*queue, &threadFinished, portMAX_DELAY);

			Serial.print("[bench] Received (part) result from thread ");
			Serial.print(k);
			Serial.print(" on core ");
			Serial.print((k+2)%2);
			Serial.print(" with status ");
			Serial.println(threadFinished);
		}

		long end = millis();

		average += end - start;

		/*
		 * =============================================================================
		 * =================================
		 */
	}

	execTime = average / numOfCycles;

	Serial.print("[bench] Number of threads: ");
	Serial.print(numOfThreads);
	Serial.print("; total time: ");
	Serial.print((int)execTime);
	Serial.println(" ms.");

	Serial.println("[bench] Compressed from " + String(width*height) + " to " + String(((width * height) / numOfCombinedBits))
			+ " characters -> done.");

	/* collect results and create PPM file to store the compressed image */
	m[0]->createPPMFile(ppmFilename);

	for(int i=0; i < numOfThreads; i++) {
		/* write task(s) string result property into the *.ppm file */
		m[i]->writeToPPMFile(ppmFilename);
	}

	/* delete the task Mandelbrot objects and free the memory */
	Serial.print("[bench] cleaning up temporary files ...");

	/* contains the elements which weren't be able to delete */
	String elements = "";

	for (int i = 0; i < numOfThreads; ++i) {
		if( m[i] != NULL ) {
			delete m[i];
		} else {
			elements += String(i) + String(" ");
		}
	}

	if( elements.length() != 0 ) {
		Serial.println(" -> FAILED.");
		Serial.println("[bench] error: could not delete object(s) Mandelbrot m[ " + elements + "].");
	} else {
		Serial.println(" -> done.");
	}

	return execTime;
}

/* -------------------------------------------------------------------------------------------------------------------------------------------- */

void Benchmark::setMaxNumOfThreads(int maxNumOfThreads) {
	this->maxNumOfThreads = maxNumOfThreads;
}
