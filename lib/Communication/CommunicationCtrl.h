/**
 * @file CommunicationCtrl.h
 * @author Philip Zellweger (philip.zellweger@hsr.ch)
 * @brief The Communication Controll class contains the FSM for the Sortic Communication Hub
 * @version 0.1
 * @date 2019-11-25
 * 
 * @copyright Copyright (c) 2019
 * 
 */

#ifndef COMMUNICATIONCTRL_H__
#define COMMUNICATIONCTRL_H__

#include <Arduino.h>
#include <string.h>
#include <stdio.h>
#include <memory>

// own files:
#include "LogConfiguration.h"
#include "MainConfiguration.h"
#include "I2cCommunication.h"
#include "MQTTCommunication.h"
#include "MessageTranslation.h"

static struct ReceivedI2cMessage gReceivedI2cMessage;
static struct WriteI2cMessage gWriteI2cMessage;


static std::deque<std::shared_ptr<ErrorMessage>> errorMessageBuffer;
static std::deque<std::shared_ptr<SBAvailableMessage>> sbAvailableMessageBuffer;
static std::deque<std::shared_ptr<SBPositionMessage>> sbPositionMessageBuffer;
static std::deque<std::shared_ptr<SBStateMessage>> sbStateMessageBuffer;
static std::deque<std::shared_ptr<SBToSOHandshakeMessage>> handshakeMessageSBToSOBuffer;
static std::deque<std::shared_ptr<BufferMessage>> soBufferMessageBuffer;

#define MASTER

//void callback(char* topic, byte* payload, unsigned int length);

/**
 * @brief The Communication Controll class contains the FSM for the Sortic Communication Hub
 * 
 */
class CommunicationCtrl
{
    //======================PUBLIC===========================================================
    public:


    /**
     * @brief Line class holds the different lines on the gametable
     * 
     */
    enum class Line
    {
        UploadLine,
        Line1,
        Line2,
        Line3,
        ErrorLine
    };

    /**
     * @brief Sortic struct holds all params of the Sortic
     * 
     */
    struct Sortic 
    {
        String id = DEFAULT_HOSTNAME;                    ///< Sorticname / Hostname of the Sortic
        Line actualLine = Line::UploadLine;              ///< actual line
        Line targetLine = Line::UploadLine;              ///< target line
        String status = "null";                          ///< status of the Box FSM
        String cargo = "null";
        String ack = "-1";                             ///< ack for handshake vehicle
        String req = "-1";                             ///< req for handshake vehicle
        String targetReg = "null";
        String targetDet = "null";
        unsigned int packageId = 0;
        
    } sortic;

    /**
     * @brief Enum class holds all possible events
     * 
     */
    enum class Event
    {
        NoEvent,
        Publish,
        SearchBox,
        BoxAvailable,
        ReqBox,
        AnswerReceived,
        NoAnswerReceived,
        SimulateBuffer,
        ArrivConfirmation,
        Error,
        Resume,
        Reset
    };
    
    /**
     * @brief Construct a new Communication Ctrl object
     * 
     */
    CommunicationCtrl();

    /**
     * @brief Destroy the Communication Ctrl object
     * 
     */
    ~CommunicationCtrl();

    /**
     * @brief Calls the do-function of the active state and hence generates Events
     * 
     */
    void loop();

    /**
     * @brief Calls the do-function of the active state and hence generates Events
     * 
     * @param newEvent - Event 
     */
    void loop(Event newEvent);


    /**
     * @brief Set the State object
     * 
     * @param state - *char
     */
    void setState(char *state);

    static void callback(char* topic, byte* payload, unsigned int length);

    //======================PRIVATE==========================================================
    private:

    I2cCommunication pBus = I2cCommunication( 7, &gReceivedI2cMessage, &gWriteI2cMessage);          ///< Instance of I2cCommunication
    Communication pComm = Communication(DEFAULT_HOSTNAME, &callback);
    PackageMessage pPackage;        ///< Instance of PackageMessage  

    unsigned long long idCounter = 0;

    /**
     * @brief Enum class holds all possible states
     * 
     */
    enum class State
    {
        idle,                           ///< idle state
        publish,                        ///< publish messages
        boxCommunication,               ///< box communication state
        arrivConfirmation,              ///< arriv confirmation state
        bufferSimulation,
        errorState,
        resetState
    };

    enum class SorticState
    {
        readRfidVal,
        waitForSort,
        sortPackageInBox,
        waitForArriv,
        errorState,
        resetState
    };


    State lastStateBeforeError;         ///< holds last state to return after error                   
    State currentState;                 ///< holds current state of the FSM
    Event currentEvent;                 ///< holds current event of the FSM

    unsigned long currentMillis = 0;          ///< store current time
    unsigned long previousMillis = 0;         ///< store last time
    unsigned long previousMillisPublish = 0;  ///< store last publish time
    unsigned long previousMillisCheckMQTT = 0;
    unsigned long previousMillisCheckI2C = 0;


    /**
     * @brief Functionpointer to call the current states do-functions
     * 
     */
    Event (CommunicationCtrl::*doActionFPtr)(void) = nullptr;

    void (CommunicationCtrl::*callbackFPtr)(char* topic, byte* payload, unsigned int length) = nullptr;

    /**
     * @brief changes the state of the FSM based on the event
     * 
     */
    void process(Event);

    /**
     * @brief entry action of the state idle, communication hub waits for action
     * 
     */
    void entryAction_idle();

    /**
     * @brief main action of the idle state
     * 
     * - read I2CReadFlag
     * - check for mqtt messages
     * 
     * @return CommunicationCtrl::Event - generated Event
     */
    Event doAction_idle();

    /**
     * @brief exit action of the idle state
     * 
     */
    void exitAction_idle();

    /**
     * @brief entry action of the state publish states
     * 
     */
    void entryAction_publish();

    /**
     * @brief main action of the state publish states
     * 
     * - publish state message
     *
     * @return Event - gerated Event
     */
    Event doAction_publish();

    /**
     * @brief exit action of the state publish states
     * 
     */
    void exitAction_publish();

    /**
     * @brief entry action of the state box communication
     * 
     * - subscribe to available boxes
     * - reset sortic params ack and req
     */
    void entryAction_boxCommunication(Event event);

    /**
     * @brief main action of the state box communication
     * 
     * - receive message with available boxes
     * - choiche best box
     * - publish request
     * - receive request
     * - publish acknoledge
     * - receive acknoledge
     * - set writeflag and message for sortic base
     * 
     * @return Event - generated Event
     */
    Event doAction_boxCommunication();

    /**
     * @brief exit action of the state box communication
     * 
     * - reset messages
     */
    void exitAction_boxCommunication();

    /**
     * @brief entry action of the state arriv communication
     * 
     * - subscribe to box fill level
     */
    void entryAction_arrivCommunication();

    /**
     * @brief main action of the state arriv communication
     * 
     * - receive updated fill level
     * - set writeflag and message for sortic base
     * 
     * @return Event - generated Event
     */
    Event doAction_arrivCommunication();

    /**
     * @brief exit action of the state arriv communication
     * 
     * - reset messages 
     */
    void exitAction_arrivCommunication();

    void entryAction_bufferSimulation();

    Event doAction_bufferSimulation();

    void exitAction_bufferSimulation();

    void entryAction_errorState();

    Event doAction_errorState();

    void exitAction_errorState();

    void entryAction_resetState();

    Event doAction_resetState();

    void exitAction_resetState();

    /**
     * @brief check MQTT message if error
     * 
     * @return true 
     * @return false 
     */
    bool checkForError();

    bool dynamicBoxChoice();

    /**
     * @brief decodes the event
     * 
     * @param e - Event
     * @return String 
     */
    String decodeEvent(Event e);

    Event decodeI2cEvent();

    String decodeSorticState(SorticState s);

    /**
     * @brief decodes the line
     * 
     * @param line - Line
     * @return String 
     */
    String decodeLineToString(Line line);

    /**
     * @brief decodes line in string format to Line format
     * 
     * @param line - String
     * @return Line 
     */ 
    Line decodeIntToLine(int line);

    /**
     * @brief decodes consignor
     * 
     * @param consignor - Consignor
     * @return String
     */
    String decodeConsignor(Consignor consignor);

};




#endif