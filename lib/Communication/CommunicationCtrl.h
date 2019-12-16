/**
 * @file CommunicationCtrl.h
 * @author Philip Zellweger (philip.zellweger@hsr.ch)
 * @brief The Communication Controll class contains the FSM for the Sortic Communication Hub
 * @version 1.1
 * @date 2019-12-16
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

#define MASTER

static struct ReceivedI2cMessage gReceivedI2cMessage;                                           ///< global instance of reiceved i2c message struc
static struct WriteI2cMessage gWriteI2cMessage;                                                 ///< global instance of write i2c message struct



static std::deque<std::shared_ptr<ErrorMessage>> errorMessageBuffer;                            ///< global instance of deque with type ErrorMessage
static std::deque<std::shared_ptr<SBAvailableMessage>> sbAvailableMessageBuffer;                ///< global instance of deque with type SBAvailableMessage
static std::deque<std::shared_ptr<SBPositionMessage>> sbPositionMessageBuffer;                  ///< global instance of deque with type SBPositionMessage
static std::deque<std::shared_ptr<SBStateMessage>> sbStateMessageBuffer;                        ///< global instance of deque with type SBStateMessage
static std::deque<std::shared_ptr<SBToSOHandshakeMessage>> handshakeMessageSBToSOBuffer;        ///< global instance of deque with type SBToSOHandshakeMessage
static std::deque<std::shared_ptr<BufferMessage>> soBufferMessageBuffer;                        ///< global instance of deque with type BufferMessage


/**
 * @brief The Communication Controll class contains the FSM for the Sortic Communication Hub
 * 
 */
class CommunicationCtrl
{
    //======================PUBLIC===========================================================
    public:

    /**
     * @brief Enum class holds the different lines on the gametable
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
        String id = DEFAULT_HOSTNAME;                   ///< Sorticname / Hostname of the Sortic
        Line actualLine = Line::UploadLine;             ///< actual line
        Line targetLine = Line::UploadLine;             ///< target line
        String status = "null";                         ///< status of the Box FSM
        String cargo = "null";                          ///< cargo of the package
        String ack = "-1";                              ///< ack for handshake vehicle
        String req = "-1";                              ///< req for handshake vehicle
        String targetReg = "null";                      ///< target region of the package
        String targetDet = "null";                      ///< target destination of the package
        unsigned int packageId = 0;                     ///< package id
    } sortic;                                           ///< instance of the sortic struct

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

    /**
     * @brief MQTT callback function 
     * 
     * - function will be used if a new message is available to receive and store the message
     * 
     * @param topic 
     * @param payload 
     * @param length 
     */
    static void callback(char* topic, byte* payload, unsigned int length);

    //======================PRIVATE==========================================================
    private:

    /**
     * @brief Enum class holds all possible states
     * 
     */
    enum class State
    {
        idle,                           
        publish,                        
        boxCommunication,               
        arrivConfirmation,              
        bufferSimulation,
        errorState,
        resetState
    };

    /**
     * @brief Enum class holds all possible states of the sortic roboter -> used for the i2c communication
     * 
     */
    enum class SorticState
    {
        readRfidVal,
        waitForSort,
        sortPackageInBox,
        waitForArriv,
        errorState,
        resetState
    };

    I2cCommunication pBus = I2cCommunication( I2CSLAVEADDRUNO, &gReceivedI2cMessage, &gWriteI2cMessage);            ///< instance of i2c communication
    Communication pComm = Communication(DEFAULT_HOSTNAME, &callback);                                               ///< instance of mqtt communication
    PackageMessage pPackage;                                                                                        ///< Instance of PackageMessage  

    unsigned long long idCounter = 0;                                                                               ///< id counter to give every message a new id
    State lastStateBeforeError;                                                                                     ///< holds last state to return after error                   
    State currentState;                                                                                             ///< holds current state of the FSM
    Event currentEvent;                                                                                             ///< holds current event of the FSM
    unsigned long currentMillis = 0;                                                                                ///< store current time
    unsigned long previousMillis = 0;                                                                               ///< store last time
    unsigned long previousMillisPublish = 0;                                                                        ///< store last publish time
    unsigned long previousMillisCheckMQTT = 0;                                                                      ///< store last time of check mqtt
    unsigned long previousMillisCheckI2C = 0;                                                                       ///< store last time of chek i2c


    /**
     * @brief Functionpointer to call the current states do-functions
     * 
     */
    Event (CommunicationCtrl::*doActionFPtr)(void) = nullptr;

    /**
     * @brief Functionpointer to hand over mqtt callback function
     * 
     */
    void (CommunicationCtrl::*callbackFPtr)(char* topic, byte* payload, unsigned int length) = nullptr;

    /**
     * @brief changes the state of the FSM based on the event
     * 
     */
    void process(Event);


    /**
     * @brief entry action of the state idle
     * 
     */
    void entryAction_idle();

    /**
     * @brief main action of the idle state
     * 
     * - check for i2c messages
     * - check for mqtt messages
     * - if message retreived, do actions
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
     * - dependent on the received i2c event publish some stuff
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
     */
    void entryAction_boxCommunication(Event event);

    /**
     * @brief main action of the state box communication
     * 
     * - subscribe to available box
     * - wait to receive messages
     * - choice optimal box
     * - publish request to the chocen box
     * - unsubscribe to available box
     * - subsrice to handshake requested box
     * - wait to receive message
     * - if ok, publish acknoledge
     * - wait to receive message
     * - if ok, unsubscribe to handshake requested box
     * 
     * @return Event - generated Event
     */
    Event doAction_boxCommunication();

    /**
     * @brief exit action of the state box communication
     * 
     * - reset i2c received message
     * - set i2c write message to sort package
     * - write i2c message to slave
     */
    void exitAction_boxCommunication();

    /**
     * @brief entry action of the state arriv communication
     * 
     * - subscribe to state of requested box
     */
    void entryAction_arrivCommunication();

    /**
     * @brief main action of the state arriv communication
     * 
     * - wait till box updated state
     * - set i2c write message that package is arrived
     * - write i2c message to slave
     * 
     * @return Event - generated Event
     */
    Event doAction_arrivCommunication();

    /**
     * @brief exit action of the state arriv communication
     * 
     * - reset received i2c message
     */
    void exitAction_arrivCommunication();

    /**
     * @brief entry action of the state buffer simulation
     * - publish buffer message to buffer topic
     * - subscribe to buffer topic
     */
    void entryAction_bufferSimulation();

    /**
     * @brief main action of the state buffer simulation
     * 
     * - wait till buffer is cleared
     * - set i2c write message that package is arrived
     * - write i2c message to slave
     * 
     * @return Event 
     */
    Event doAction_bufferSimulation();

    /**
     * @brief exit action of the state buffer simulation
     * 
     * - reset received i2c message
     * 
     */
    void exitAction_bufferSimulation();

    /**
     * @brief entry action of the state error state
     * 
     * - publish error state to sortic state topic
     * 
     */
    void entryAction_errorState();

    /**
     * @brief main action of the state error state
     * 
     * - check error message buffer
     * - if no error, resume
     * - if error, reset
     * 
     * @return Event 
     */
    Event doAction_errorState();

    /**
     * @brief exit action of the state error state
     * 
     */
    void exitAction_errorState();

    /**
     * @brief entry action of the state reset state
     * 
     * - publish current state
     * 
     */
    void entryAction_resetState();

    /**
     * @brief main action of the state reset state
     * 
     * - reset all buffers
     * 
     * @return Event 
     */
    Event doAction_resetState();

    /**
     * @brief exit action of the state reset state
     * 
     */
    void exitAction_resetState();

    /**
     * @brief Choiche a possible box based on datas of how many boxes are available 
     *        and how many packages for a target region are ther to sort
     * 
     * @todo Implement dynamic box choice
     * 
     * @return true 
     * @return false 
     */
    bool dynamicBoxChoice();

    /**
     * @brief decodes the event of the communication control to a string
     * 
     * @param e - Event
     * @return String 
     */
    String decodeEvent(Event e);

    /**
     * @brief decodes the received i2c event to communication control event
     * 
     * @return Event 
     */
    Event decodeI2cEvent();

    /**
     * @brief decodes the state of the sortic control to a string
     * 
     * @param s 
     * @return String 
     */
    String decodeSorticState(SorticState s);

    /**
     * @brief decodes the line to a string
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
     * @brief decodes the consignor to a string
     * 
     * @param consignor - Consignor
     * @return String
     */
    String decodeConsignor(Consignor consignor);

};

#endif // COMMUNICATIONCTRL_H__