/*
 * BenchmarkHandler.cpp
 *
 *  Created on: Dec 7, 2019
 *      Author: joseph
 */

#include "BenchmarkHandler.h"

extern WebsocketHandler *wsHandler;

BenchmarkHandler::BenchmarkHandler(int numOfRunningThreads) {
	Serial.println("-------------------------------------------------------------- benchmark handler initialize -------------------------------------------------------------");

	this->numOfRunningThreads = numOfRunningThreads;

	results = new float[this->numOfRunningThreads];
	speedup = new float[this->numOfRunningThreads];

	bench = new Benchmark(1, 1500, this->numOfRunningThreads);
	bench->setMaxNumOfThreads(this->numOfRunningThreads);

	state = BENCHMARK_READY;
	ui = UI_BLOCKED;

	nHandler = NULL;
	nAssetHandler = NULL;

	/* delete old files and recreate them (results.txt, mandelbrot.ppm) */
	if( spiffs.fileExists(SPIFFS, "/results.txt") ) {
		Serial.print("[bench-handler] deleting old results.txt file ...");
		Serial.println( spiffs.deleteFile(SPIFFS, "/results.txt") ? " -> done." : " -> FAILED." );
	}

	createResultTXTFile("/results.txt");

	if( spiffs.fileExists(SPIFFS, "/mandelbrot.ppm") ) {
		Serial.print("[bench-handler] deleting old mandelbrot.ppm file ...");
		Serial.println( spiffs.deleteFile(SPIFFS, "/mandelbrot.ppm") ? " -> done." : " -> FAILED." );
	}

	Serial.println();
}

BenchmarkHandler::~BenchmarkHandler() {
	delete[] results;
	delete[] speedup;

	delete bench;
	delete nHandler;
	delete nAssetHandler;
}

void BenchmarkHandler::startComputationBenchmark() {
	if( ui != UI_RUNNING && ui != UI_READY ) {
		if( state != BENCHMARK_RUNNING && state != BENCHMARK_FINISHED) {
			state = BENCHMARK_RUNNING;

			Serial.println("----------------------------------------------------------------- benchmark Computation ----------------------------------------------------------------");

			for (int i = 0; i < this->numOfRunningThreads; ++i) {
				Serial.print("[bench-handler] Performing benchmark number ");
				Serial.print(i + 1);
				Serial.println(" started ...");

				Serial.print("[bench-handler] Number of threads: ");
				Serial.println(i + 1);

				/*
				 *	print progress on oled display
				 */
				oledHandler.drawProgressBar("Computation: ", ((float)i / (float)this->numOfRunningThreads) * 100);

				/*
				 *  performing the benchmark execution with the total number of threads, which will be created
				 *  return value is the execution time, which was needed to perform the computation
				 */
				results[i] = float(bench->performComputationBenchmark( i + 1 ));

				Serial.print("[bench-handler] Performing benchmark number ");
				Serial.print(i + 1);
				Serial.println(" finished.\n");
			}

			/* benchmark run finish */
			oledHandler.drawProgressBar("Computation: ", 100);

			/* store execution time results to results.txt */
			for (int i = 0; i < numOfRunningThreads; ++i) {
				appendToResultTXTFile("/results.txt", String((int)results[i]) + ";");
			}

			/* finalize the written results in the file with an \n */
			appendToResultTXTFile("/results.txt", String("\n"));

			/* benchmark is successfully finished, ready for another round */
			state = BENCHMARK_PERFORMED;

			/* ui = UI_READY and state = BENCHMARK_FINISHED should now be called from function finalize() to start UI ! */

		} else {
			Serial.println("[bench-handler] error: benchmark already running or not ready yet.");
		}
	} else {
		Serial.println("[bench-handler] error: can not start benchmark. UI is already running.");
	}
}

void BenchmarkHandler::startMandelbrotBenchmark() {
	if( ui != UI_RUNNING && ui != UI_READY ) {
		if( state != BENCHMARK_RUNNING && state != BENCHMARK_FINISHED) {
			state = BENCHMARK_RUNNING;

			Serial.println("----------------------------------------------------------------- benchmark Mandelbrot -----------------------------------------------------------------");

			for (int i = 0; i < this->numOfRunningThreads; ++i) {
				Serial.print("[bench-handler] Performing benchmark number ");
				Serial.print(i + 1);
				Serial.println(" started ...");

				Serial.print("[bench-handler] Number of threads: ");
				Serial.println(i + 1);

				/*
				 *	print progress on oled display
				 */
				oledHandler.drawProgressBar("Mandelbrot: ", ((float)i / (float)this->numOfRunningThreads) * 100);

				/*
				 *  performing the benchmark execution with the total number of threads, which will be created
				 *  return value is the execution time, which was needed to perform the computation
				 */
				results[i] = float(bench->performMandelbrotBenchmark("/mandelbrot.ppm", 600, 600, i+1));

				Serial.print("[bench-handler] Performing benchmark number ");
				Serial.print(i + 1);
				Serial.println(" finished.\n");
			}

			/* benchmark run finish */
			oledHandler.drawProgressBar("Mandelbrot: ", 100);

			/* store execution time results to results.txt */
			for (int i = 0; i < numOfRunningThreads; ++i) {
				appendToResultTXTFile("/results.txt", String((int)results[i]) + ";");
			}

			/* finalize the written results in the file with an \n */
			appendToResultTXTFile("/results.txt", String("\n"));

			/* benchmark is successfully finished, ready for another round */
			state = BENCHMARK_PERFORMED;

			/* ui = UI_READY and state = BENCHMARK_FINISHED should now be called from function finalize() to start UI ! */

		} else {
			Serial.println("[bench-handler] error: benchmark already running or not ready yet.");
		}
	} else {
		Serial.println("[bench-handler] error: can not start benchmark. UI is already running.");
	}
}

void BenchmarkHandler::displayResult() {
	if( state != BENCHMARK_RUNNING && state != BENCHMARK_FINISHED && state == BENCHMARK_PERFORMED ) {
		Serial.println("---------------------------------------------------------------------- evaluation ----------------------------------------------------------------------");

		/*
		 *  display the number of threads as an array:
		 *  -> number of threads:	[	1	2	3	....	]
		 */
		Serial.print("number of threads:\t[\t");

		for (int j = 0; j < numOfRunningThreads; ++j) {
			Serial.print( j + 1 );
			Serial.print("\t\t");
		}

		Serial.println("]");

		/*
		 *  display the different execution time as an array:
		 *  -> execution time:	[	423		121		....	]
		 */
		Serial.print("execution time:\t\t[\t");

		for (int k = 0; k < numOfRunningThreads; ++k) {
			Serial.print( (int)results[k] );

			if( String((int)results[k]).length() < 3 ) {
				Serial.print("\t\t");
			} else {
				Serial.print("\t");
			}
		}

		Serial.println("]");

		/*
		 *  calculate the total speedup in comparison to single execution
		 *  -> S = t_1 / t_p with
		 *     t_1: time for computing the result on a single core
		 *     t_p: time for computing the problem on p processors / threads
		 */
		Serial.print("total speedup:\t\t[\t");

		for (int l = 0; l < numOfRunningThreads; ++l) {
			speedup[l] = float(results[0] / results[l]);

			if( speedup[l] < 0 ) {
				Serial.print("0\t\t");
			} else {
				Serial.print( speedup[l] );
				Serial.print("\t");
			}
		}

		Serial.println("]");

		/*
		 *  in fact, we only have two real cores on the esp32, but each core can handle several own threads under the restrictions
		 *  of hardware resources such as heap size, clock frequency, scheduler performance ...
		 *  so each thread will be asigned to one dedicated core [0/1]
		 */
		Serial.print("core number:\t\t[\t");

		for (int m = 0; m < numOfRunningThreads; ++m) {
			Serial.print( (m+2)%2 );
			Serial.print("\t\t");
		}

		Serial.println("]");
	} else {
		Serial.println("[bench-handler] error: benchmark already running or not ready yet.");
	}

	Serial.println();
}

void BenchmarkHandler::createResultTXTFile(String filename) {
	Serial.print("[bench-handler] creating *.txt text file for storing the execution time results ...");

	/*
	 * 	filename.txt file:
	 *
	 * 	---------------------------------
	 * 	...
	 * 	---------------------------------
	 *
	 */

	if( !spiffs.writeFile(SPIFFS, filename.c_str(), String("")) ) {
		Serial.println(" -> FAILED.");
		return;
	}

	Serial.println(" -> done.");

	delay(100);
}

void BenchmarkHandler::appendToResultTXTFile(String filename, String data) {
	if( data == "\n" ) {
		Serial.print("[bench-handler] finalizing results.txt with EOL character ...");
	} else {
		Serial.print("[bench-handler] storing results to *.txt text file ...");
	}

	if( !spiffs.appendFile(SPIFFS, filename.c_str(), data) ) {
		Serial.println(" -> FAILED.");
		return;
	}

	Serial.println(" -> done.");

	if( data == "\n" ) {
		Serial.println();
	}
}

void BenchmarkHandler::finalize() {
	if( state != BENCHMARK_RUNNING && state != BENCHMARK_READY ) {
		/* update state: handler is finished, UI is ready to take off */
		state = BENCHMARK_FINISHED;
		ui = UI_READY;

		oledHandler.displayText("Benchmark finished.\nUI up and running!");

	} else {
		ui = UI_BLOCKED;
	}
}

void BenchmarkHandler::displayUI() {
	/* benchmark is finished! */
	if( state != BENCHMARK_RUNNING && state != BENCHMARK_READY ) {

		switch (ui) {
			case UI_READY:
				/* UI ready, initialize necessary objects */
				Serial.println("-------------------------------------------------------------------------- UI --------------------------------------------------------------------------");

				Serial.println("[bench-handler] benchmark successfully finished.");
				Serial.println("[bench-handler] UI is ready to be initialized!");

				initUI();

				ui = UI_RUNNING;

				break;
			case UI_RUNNING:
				/* UI running */
				nAssetHandler->handleAssetRequests();
				wsHandler->handleWebSocketRequests();

				delay(1);

				break;
			case UI_BLOCKED:
				/* benchmark is finished, but UI state is still blocked */
				/* -> this case actually can never happened, only if sth. went wrong during switching the state */
				Serial.println("[bench-handler] benchmark successfully finished.");
				Serial.println("[bench-handler] error: UI is still blocked! Please check the start**() or finalize() routines.");

				Serial.println("[bench-handler] ABORTED!");

				delay(1000);

				break;
			default:
				break;
		}

	}
}

void BenchmarkHandler::initUI() {
	/* -> Creating unique access point */
	nHandler = new NetworkHandler();
	nHandler->createUniqueAP("ESP32-", "12345678");

	/* serve web server files */
	nAssetHandler = new AssetHandler();

	/* allow frontend connection to backend via websocket connection: IP:90/ */
	wsHandler = new WebsocketHandler();
}
