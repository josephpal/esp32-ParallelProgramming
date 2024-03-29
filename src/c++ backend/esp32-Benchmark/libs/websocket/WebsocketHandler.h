/*
 * WebsocketHandler.h
 *
 *  Created on: Okt 27, 2017
 *      Author: joseph
 *
 *  description:    
 */

#ifndef WEBSOCKETHANDLER_H_
#define WEBSOCKETHANDLER_H_

#define MAXCLIENTS 2

#include <WiFi.h>
#include "WebSocketServer.h"
#include "WebSocketConnection.h"
#include "../memory/CSpiffsStorage.h"
#include "Mutex.h"

class WebsocketHandler {
public:
    WebsocketHandler();
    ~WebsocketHandler();

    /*
     * 
     */
    void sendWebSocketMessage(String message);

    /*
     * 
     */
    void sendWebSocketMessageToClient(WiFiClient * pClient, String msg);

    /*
     * 
     */
    bool addWebSocketClient(WiFiClient * pClient, WebSocketServer *nWebSocketServer);

    /*
     * 
     */
    bool removeWebSocketClient(WiFiClient * pClient);

    /*
     * 
     */
    int getClientID(WiFiClient * pClient);

    /*
     * 
     */
    void handleWebSocketRequests();

    /*
     * 
     */
    bool handleWebsocketTimeout(int clientID);

    /*
     * 
     */
    void websocketTimedOut(int clientID);	//modified by JM, explanation see .cpp

    /*
     * 
     */
    void setWebsocketFlag(int clientID, bool mStatus);

    /*
     * 
     */
    bool getWebsocketFlag(int clientID);

    /*
     * 
     */
    void setWebsocketPingReceived(int clientID, bool mStatus);

    /*
     * 
     */
    bool getWebsocketPingReceived(int clientID);

    /*
     *
     */
    CSPIFFS getStorage();
        
private:
    int clientCount;
    WebSocketConnection **webSocketConnections;
    WiFiServer *webSocketServer;

    /* [timeoutFlagWS1, pingReceivedWS1, timeoutFlagWS2, pingReceivedWS2] */
    bool websocketConnectionStatus[4];

    Mutex addClient, removeClient, sendData, clientID, accessFlag;

    CSpiffsStorage storage;
};

#endif /* WEBSOCKETHANDLER_H_ */
