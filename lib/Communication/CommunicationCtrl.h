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
#include <Wire.h>
#include <WiFi.h>

// own files:
#include "LogConfiguration.h"
#include "MainConfiguration.h"
#include "I2cCommunication.h"
#include "Messages.h"

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
        UploadLine = 0,
        Line1 = 1,
        Line2 = 2,
        Line3 = 3
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
        String ack = "null";                             ///< ack for handshake vehicle
        String req = "null";                             ///< req for handshake vehicle
    } sortic;

    /**
     * @brief Enum class holds all possible events
     * 
     */
    enum class Event
    {
        NoEvent = 0,
        PublishState = 1,
        PublishPosition = 2,
        PublishPackage = 3,
        PublishedPackage = 4,
        BoxCommunication = 5,
        ArrivConfirmation = 6,
        AnswerReceived = 7,
        NoAnswerReceived = 8,
        SimulateBuffer = 9,
        ClearGui = 10,
        Error = 11
    };
    
    /**
     * @brief Construct a new Communication Ctrl object
     * 
     */
    CommunicationCtrl();

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

    //======================PRIVATE==========================================================
    private:

    I2cCommunication pBus;          ///< Instance of I2cCommunication
    PackageMessage pPackage;        ///< Instance of PackageMessage

    /**
     * @brief Enum class holds all possible states
     * 
     */
    enum class State
    {
        idle = 0,                       ///< idle state
        publishStates = 1,              ///< publish states state
        publishPackage = 2,             ///< publish package state
        publishPosition = 3,            ///< publish position state
        gui = 4,                        ///< gui state
        boxCommunication = 5,           ///< box communication state
        arrivConfirmation = 6           ///< arriv confirmation state
    };


    State lastStateBeforeError;         ///< holds last state to return after error                   
    State currentState;                 ///< holds current state of the FSM
    Event currentEvent;                 ///< holds current event of the FSM

    unsigned long currentMillis = 0;          ///< store current time
    unsigned long previousMillis = 0;         ///< store last time
    unsigned long previousMillisPublish = 0;  ///< store last publish time

    /**
     * @brief Functionpointer to call the current states do-functions
     * 
     */
    Event (CommunicationCtrl::*doActionFPtr)(void) = nullptr;

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
    void entryAction_publishStates();

    /**
     * @brief main action of the state publish states
     * 
     * - publish state message
     *
     * @return Event - gerated Event
     */
    Event doAction_publishStates();

    /**
     * @brief exit action of the state publish states
     * 
     */
    void exitAction_publishStates();

    /**
     * @brief entry action of the state publish position
     * 
     */
    void entryAction_publishPosition();
    
    /**
     * @brief main acion of the state publish position
     * 
     * - publish position message
     * 
     * @return Event - generated Event
     */
    Event doAction_publishPosition();

    /**
     * @brief exit action of the state publish position
     * 
     */
    void exitAction_publishPosition();

    /**
     * @brief entry action of the state publish package
     * 
     */
    void entryAction_publishPackage();

    /**
     * @brief main action of the state publish package
     * 
     * - get target region of the package
     * - publish package message
     * 
     * @return Event - generated Event
     */
    Event doAction_publishPackage();

    /**
     * @brief exit action of the state publish package
     * 
     */
    void exitAction_publishPackage();

    /**
     * @brief entry action of the state box communication
     * 
     * - subscribe to available boxes
     * - reset sortic params ack and req
     */
    void entryAction_boxCommunication();

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


    void entryAction_gui(Event e);

    Event doAction_gui();

    void exitAction_gui();

    void entryAction_bufferSimulation();

    Event doAction_bufferSimulation();

    void exitAction_bufferSimulation();

    void entryAction_errorState();

    Event doAction_errorState();

    void exitAction_errorState();

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

    /**
     * @brief decodes the line
     * 
     * @param line - Line
     * @return String 
     */
    String decodeLine(Line line);
};




#endif