/*
 * BenchmarkHandler.h
 *
 *  Created on: Dec 7, 2019
 *      Author: joseph
 */

#ifndef BENCHMARKHANDLER_H_
#define BENCHMARKHANDLER_H_

#include <Arduino.h>

#include "Benchmark.h"
#include "../network/NetworkHandler.h"
#include "../asset/AssetHandler.h"
#include "../websocket/WebsocketHandler.h"
#include "../memory/CSPIFFS.h"
#include "../oled/OLEDHandler.h"

enum HandlerState {
	BENCHMARK_READY,
	BENCHMARK_RUNNING,
	BENCHMARK_PERFORMED,
	BENCHMARK_FINISHED,
	BENCHMARK_ABORTED,
	BENCHMARK_UNKOWN
};

enum UIState {
	UI_READY,
	UI_RUNNING,
	UI_BLOCKED
};

class BenchmarkHandler {
public:
	/** overloaded constructor
	 *
     *  @param  specify the amount of maximum created threads, on which the task (Computation or Mandelbrot) should be split
     *  @return ---
    */
	BenchmarkHandler(int numOfRunningThreads);

	/** default destructor
     *
     *  @param	---
     *  @return ---
    */
	~BenchmarkHandler();

	/** ...
     *
     *  @param	---
     *  @return ---
    */
	void startComputationBenchmark();

	/** ...
     *
     *  @param	---
     *  @return ---
    */
	void startMandelbrotBenchmark();

	/**	...
	 *
	 * 	@param	...
	 * 	@return	---
	*/
	void displayResult();

	/** ...
	 *
	 *  @param
	 *  @return
	*/
	void createResultTXTFile(String filename);

	/** ...
	 *
	 *  @param
	 *  @return
	*/
	void appendToResultTXTFile(String filename, String data);

	/**	when user Benchmark performs are finished, this function should be called afterwards to enable the UI
	 *
	 * 	@param	...
	 * 	@return	---
	*/
	void finalize();

	/**	...
	 *
	 * 	@param	...
	 * 	@return	---
	*/
	void displayUI();

	/**	...
	 *
	 * 	@param	...
	 * 	@return	---
	*/
	void initUI();

private:
	int numOfRunningThreads;

	float *results;
	float *speedup;

	Benchmark *bench;

	HandlerState state;
	UIState ui;

	/* UI specific elements */
	NetworkHandler  *nHandler;
	AssetHandler *nAssetHandler;

	/* memory access */
	CSPIFFS spiffs;

	/* display driver */
	OLEDHandler oledHandler;
};

#endif /* BENCHMARKHANDLER_H_ */
