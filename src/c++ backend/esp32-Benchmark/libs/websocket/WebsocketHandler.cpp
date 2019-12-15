#include "WebsocketHandler.h"

extern WebsocketHandler *wsHandler;

void webSocketTask(void* params) {
	WiFiClient* nClient = (WiFiClient*) params;
	WebSocketServer* nWebSocketServer = new WebSocketServer();
  
	String *data = new String();
	String *payload = new String();
	String *identifier = new String();

	if(wsHandler->addWebSocketClient(nClient, nWebSocketServer)) {
		if(nClient->connected() && nWebSocketServer->handshake(*nClient)) {
			int websocketClientID = wsHandler->getClientID(nClient);
      
			Serial.print("[ws] Handshake with client ");
			Serial.print(websocketClientID);
			Serial.println(" successfully done.");
      
			Serial.println("[ws] Establishing connection...");

			// Timer flag states
			wsHandler->setWebsocketFlag(websocketClientID, true);
			wsHandler->setWebsocketPingReceived(websocketClientID, true);

			//Testing 5 sec intervall JM
			int oldModulo = 0;

			while (nClient->connected()) {
				*data = nWebSocketServer->getData();

				delay(10); // Delay needed for receiving the data correctly

				if (data->length() > 0) {
					int delimiterIndex = data->indexOf('/');
					*payload = data->substring(delimiterIndex + 1, data->length());
					*identifier = data->substring(0, delimiterIndex);

					if(*identifier == "getppm") {
						Serial.println("[ws] get request for *.ppm file!");

						wsHandler->sendWebSocketMessage("ppm");
					} else if(*identifier == "getresults") {
						Serial.println("[ws] get request for results.txt file!");
						wsHandler->sendWebSocketMessage("results/Test123");

					} else if(*identifier == "reboot") {
						Serial.println("[ws] ESP reboot!");
						wsHandler->sendWebSocketMessage("rebooting");

					} else if(*identifier == "config") {
						Serial.println("[ws] config received");
						wsHandler->sendWebSocketMessage("received");

					} else if(*identifier == "ping") {
						// ping event -> flag to true
						wsHandler->setWebsocketFlag(websocketClientID, true);
						wsHandler->sendWebSocketMessageToClient(nClient, "pong");
					}
				}

				if( wsHandler->handleWebsocketTimeout(websocketClientID) ) {
					Serial.println("[ws] Error: websocket timed out!");
					Serial.println("[ws] Close websocket connection ...");
					break;
				}

				//Testing 5 sec intervall and start ws-timeout here JM:
				unsigned long currentTime = millis();
				if (currentTime % 5000 < oldModulo) {
					// Serial.println("[ws] ws-connection " + (String)websocketClientID + ": 5 Seconds passed, time: " + (String)currentTime);
					wsHandler->websocketTimedOut(websocketClientID);
				}

				oldModulo = currentTime % 5000;
			}
		}
	} else {
	Serial.println("[ws] Maximum client connections reached.");
	delete nClient;
	}

	wsHandler->removeWebSocketClient(nClient);

	delete data;
	delete payload;
	delete identifier;

	Serial.print("[ws] free heap: ");
	Serial.println(ESP.getFreeHeap());

	vTaskDelete(NULL);
}

WebsocketHandler::WebsocketHandler() {
	Serial.println();
	Serial.print("[ws] Starting websocket connection listener ...");

	addClient.unlock();
	removeClient.unlock();
	sendData.unlock();
	clientID.unlock();
	accessFlag.unlock();

	clientCount = 0;
	webSocketServer = new WiFiServer(90);

	webSocketConnections = new WebSocketConnection*[MAXCLIENTS];

	for(int i = 0; i < MAXCLIENTS; i++) {
		setWebsocketFlag(i, true);
		setWebsocketPingReceived(i, true);
	}

	for(int i = 0; i < MAXCLIENTS; i++) {
		webSocketConnections[i] = NULL;
		//connectedClients[i] = NULL;
	}

	webSocketServer->begin();
	Serial.println(" -> done.\n");
}

WebsocketHandler::~WebsocketHandler() {
	delete webSocketConnections;
	delete webSocketServer;
}

bool WebsocketHandler::addWebSocketClient(WiFiClient * pClient, WebSocketServer *nWebSocketServer) {
	addClient.lock();

	if(clientCount >= MAXCLIENTS) {
		addClient.unlock();
	} else {
		for(int i = 0; i < MAXCLIENTS; i++) {
			if(webSocketConnections[i] == NULL) {
				webSocketConnections[i] = new WebSocketConnection(pClient, nWebSocketServer);

				Serial.print("[ws] Adding client ");
				Serial.println(i);
				clientCount++;
				addClient.unlock();
				return true;
			}
		}
	}

	addClient.unlock();

	return false;
}

bool WebsocketHandler::removeWebSocketClient(WiFiClient * pClient) {
	removeClient.lock();

	for(int i = 0; i < MAXCLIENTS; i++) {
		if(webSocketConnections[i] != NULL) {
			if(webSocketConnections[i]->pClient == pClient) {
				delete webSocketConnections[i]->pClient;
				delete webSocketConnections[i]->pWebSocketServer;
				delete webSocketConnections[i];
				webSocketConnections[i] = NULL;

				Serial.print("[ws] Removing client ");
				Serial.println(i);
				clientCount--;
				removeClient.unlock();
				return true;
			}
		}
	}

	removeClient.unlock();
	return false;
}

void WebsocketHandler::sendWebSocketMessage(String message) {
	sendData.lock();

	for(int i = 0; i < MAXCLIENTS; i++) {
		if(webSocketConnections[i] != NULL) {
			Serial.print("[ws] Sending msg to client ");
			Serial.print(i);
			Serial.print(" : ");
			Serial.println(message);

			webSocketConnections[i]->pWebSocketServer->sendData(message);
		}
	}

	sendData.unlock();
}

void WebsocketHandler::sendWebSocketMessageToClient(WiFiClient * pClient, String msg) {
	sendData.lock();
	int clientID = getClientID(pClient);

	bool sendCondition = clientID != -1 && webSocketConnections[clientID] != NULL;

	if( msg != "pong" ) {
		Serial.print("[ws] Sending msg to client ");
		Serial.print(clientID);

		if(sendCondition) {
			Serial.print(" : ");
			Serial.println(msg);

			webSocketConnections[clientID]->pWebSocketServer->sendData(msg);
		} else {
			Serial.println(" failed. Client is removed or not available anymore.");
		}
	} else {
		if(sendCondition) {
			webSocketConnections[clientID]->pWebSocketServer->sendData(msg);
		}
	}

	sendData.unlock();
}

int WebsocketHandler::getClientID(WiFiClient * pClient) {
	clientID.lock();

	for(int i = 0; i < MAXCLIENTS; i++) {
		//Debugging for: C0 connected, C1 connected, C0 disconnected, crash
		//solution: check for NULLpointer before check for further pointers
		if (webSocketConnections[i] != NULL) {
			if (webSocketConnections[i]->pClient == pClient) {
				clientID.unlock();
				return i;
			}
		}
	}

	clientID.unlock();
	return -1;
}

void WebsocketHandler::handleWebSocketRequests() {
	WiFiClient* client = new WiFiClient(webSocketServer->available());

	if (*client) {
		//Serial.println("New client found");
		xTaskCreatePinnedToCore(
			webSocketTask,    /* Function to implement the task */
			"",               /* Name of the task */
			10000,            /* Stack size in words */
			(void*)client,    /* Task input parameter */
			0,                /* Priority of the task */
			NULL,             /* Task handle. */
			0);               /* Core where the task should run */
	} else {
		delete client;
	}

	delay(10);
}

bool WebsocketHandler::handleWebsocketTimeout(int clientID) {
	// Serial.println("[ws] handleWebsocketTimeout");

	if( wsHandler->getWebsocketFlag(clientID) == false
			&& wsHandler->getWebsocketPingReceived(clientID) == false ) {
		return true;
	}
	return false;
}

void WebsocketHandler::websocketTimedOut(int clientID) {
	// only check for timed out in current connection (not every connection)
	// necessary, as now every connection checks for timeout itself, not only one timer
  
	if(webSocketConnections[clientID] != NULL) {
		// Serial.print("[ws-timer] Client ");
		// Serial.print(clientID);
		// Serial.println(":");
      
		// Serial.print("[ws-timer] Flag: ");
		// Serial.println(wsHandler->getWebsocketFlag(clientID));
		// Serial.print("[ws-timer] Ping: ");
		// Serial.println(wsHandler->getWebsocketPingReceived(clientID));
    
		if(getWebsocketFlag(clientID) == true && getWebsocketPingReceived(clientID) == true) {
			// flag = true -> ping received
			setWebsocketPingReceived(clientID, true);
			// reset flag
			setWebsocketFlag(clientID, false);
		} else if (getWebsocketFlag(clientID) == false && getWebsocketPingReceived(clientID) == true) {
			// flag = false -> ping not received
			setWebsocketPingReceived(clientID, false);
			// reset flag
			setWebsocketFlag(clientID, false);
		} 
	} else {
		// initial state
		setWebsocketPingReceived(clientID, false);
		setWebsocketFlag(clientID, false);
	}
}

void WebsocketHandler::setWebsocketFlag(int clientID, bool mStatus) {
	accessFlag.lock();

	websocketConnectionStatus[clientID*2] = mStatus;

	accessFlag.unlock();
}

bool WebsocketHandler::getWebsocketFlag(int clientID) {
	accessFlag.lock();

	bool returnParam = websocketConnectionStatus[clientID*2];

	accessFlag.unlock();

	return returnParam;
}

void WebsocketHandler::setWebsocketPingReceived(int clientID, bool mStatus) {
	accessFlag.lock();

	websocketConnectionStatus[clientID*2 + 1] = mStatus;

	accessFlag.unlock();
}

bool WebsocketHandler::getWebsocketPingReceived(int clientID) {
	accessFlag.lock();

	bool returnParam = websocketConnectionStatus[clientID*2 + 1];

	accessFlag.unlock();

	return returnParam;
}

CSPIFFS WebsocketHandler::getStorage() {
	return storage.getSpiffs();
}
