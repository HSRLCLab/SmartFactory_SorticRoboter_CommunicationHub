/**
 * @file CommunicationCtrl.cpp
 * @author Philip Zellweger (philip.zellweger@hsr.ch)
 * @brief The Communication Controll class contains the FSM for the Sortic Communication Hub
 * @version 0.1
 * @date 2019-11-25
 * 
 * @copyright Copyright (c) 2019
 * 
 */

#include "CommunicationCtrl.h"

CommunicationCtrl::CommunicationCtrl() : currentState(State::idle), doActionFPtr(&CommunicationCtrl::doAction_idle)
{
    DBFUNCCALLln("CommunicationCtrl::CommunicationCtrl()");    
}

CommunicationCtrl::~CommunicationCtrl()
{

}

void CommunicationCtrl::loop()
{
    DBFUNCCALLln("CommunicationCtrl::loop()");
    process((this->*doActionFPtr)());   // do actions
}

void CommunicationCtrl::loop(Event currentEvent)
{
    DBFUNCCALLln("CommunicationCtrl::loop(Event)");
    process(currentEvent);
    process((this->*doActionFPtr)());
}

void CommunicationCtrl::process(Event e)
{
    DBFUNCCALLln("CommunicationCtrl::process()");
    DBEVENTln(String("CommunicationCtrl ") + decodeEvent(e));
    switch (currentState)
    {
    case State::idle:
        if (Event::Publish == e)
        {
            exitAction_idle();
            entryAction_publish();
        }
        else if (Event::SearchBox == e)
        {
            exitAction_idle();
            entryAction_boxCommunication(e);
        }
        else if (Event::ArrivConfirmation == e)
        {
            exitAction_idle();
            entryAction_arrivCommunication();
        }
        else if (Event::Error == e)
        {
            exitAction_idle();
            entryAction_errorState();
        }
        break;
    case State::publish:
        if (Event::NoEvent == e)
        {
            exitAction_publish();
            entryAction_idle();
        }
        else if (Event::Error == e)
        {
            exitAction_publish();
            entryAction_errorState();
        }
        break;
    case State::boxCommunication:
        if (Event::SimulateBuffer == e)
        {
            exitAction_boxCommunication();
            entryAction_bufferSimulation();
        }
        else if (Event::BoxAvailable == e || Event::ReqBox == e)
        {
            exitAction_boxCommunication();
            entryAction_boxCommunication(e);
        }
        else if (Event::AnswerReceived == e)
        {
            exitAction_boxCommunication();
            entryAction_idle();
        }
        else if (Event::Error == e)
        {
            exitAction_boxCommunication();
            entryAction_errorState();
        }
        break;
    case State::arrivConfirmation:
        if (Event::AnswerReceived == e)
        {
            exitAction_arrivCommunication();
            entryAction_idle();
        }
        else if (Event::Error == e)
        {
            exitAction_arrivCommunication();
            entryAction_errorState();
        }
    case State::errorState:
        if (Event::Resume == e)
        {
            exitAction_errorState();
            switch (lastStateBeforeError)
            {
            case State::idle:
                entryAction_idle();
                break;
            case State::publish:
                entryAction_publish();
                break;
            case State::boxCommunication:
                entryAction_boxCommunication(e);
                break;
            case State::arrivConfirmation:
                entryAction_arrivCommunication();
                break;
            case State::bufferSimulation:
                entryAction_bufferSimulation();
                break;
            }
        }
        else if (Event::Reset == e)
        {
            exitAction_errorState();
            entryAction_resetState();
        }
    case State::resetState:
        if (Event::Resume == e)
        {
            exitAction_resetState();
            entryAction_idle();
        }
        break;
    case State::bufferSimulation:
        if (Event::AnswerReceived == e)
        {
            exitAction_bufferSimulation();
            entryAction_idle(); 
        }
        else if (Event::Error == e)
        {
            exitAction_bufferSimulation();
            entryAction_errorState();
        }       
        break;
    }
}

void CommunicationCtrl::entryAction_idle()
{
    DBSTATUSln("Entering State: idle");
    currentState = State::idle;  // state transition
    doActionFPtr = &CommunicationCtrl::doAction_idle;

    currentMillis = millis();
    previousMillisCheckMQTT = currentMillis;
    previousMillisCheckI2C = currentMillis;
}

CommunicationCtrl::Event CommunicationCtrl::doAction_idle()
{
    DBINFO1ln("State: idle");
    Event retVal = Event::NoEvent;


    currentMillis = millis();
    if ((currentMillis - previousMillisCheckI2C) > 400)
    {
        previousMillisCheckI2C = millis();
        pBus.readMessage();
    }
    
    
    if(strcmp((char*)(gReceivedI2cMessage.event),"null#######"))
    {
        DBINFO2ln("Decode I2c Event");
        return decodeI2cEvent();
    }
    
    
    
    // TEST
    /*
    sortic.targetReg = "East";
    sortic.ack = "SB1";
    strcpy((pReceivedI2cMessage.event), "ArrivConf#");
    if(!strcmp((pReceivedI2cMessage.event),"ArrivConf#"))
    {
        return Event::ArrivConfirmation;
    }*/
    // TEST


    // check for mqtt messages
    currentMillis = millis();
    if ((currentMillis - previousMillisCheckMQTT) > 900)
    {
        DBINFO2ln("Check for MQTT message")
        previousMillisCheckMQTT = millis();
        pComm.loop(); // Unhandled exception here, worked at date 13.12.19 and now not anymore
    }
    if (!errorMessageBuffer.empty()) // Check for error
    {
        Event retVal = Event::NoEvent;
        while ( errorMessageBuffer.size() > 0)
        {
            if ( errorMessageBuffer.front()->error &&  errorMessageBuffer.front()->token)
            {
                retVal = Event::Error;
                errorMessageBuffer.pop_front();
            }
        }
        return retVal;
    }
    return retVal;
}

void CommunicationCtrl::exitAction_idle()
{
    DBSTATUSln("Leaving State: idle");
}

void CommunicationCtrl::entryAction_publish()
{
    DBSTATUSln("Entering State: publish");
    currentState = State::publish;  // state transition
    doActionFPtr = &CommunicationCtrl::doAction_publish;
}

CommunicationCtrl::Event CommunicationCtrl::doAction_publish()
{
    DBINFO1ln("State: publish")
    if (!strcmp(gReceivedI2cMessage.event, "PublishSTA#"))
    {
        DBINFO2ln("Publish state");
        std::shared_ptr<SOStateMessage> tempMessage (new SOStateMessage());
        tempMessage->setMessage(idCounter++, Consignor::SO1, decodeSorticState((SorticState)(gReceivedI2cMessage.state)));
        pComm.publishMessage("Sortic/SO1/status", Message::translateStructToString(tempMessage));
    }
    if (!strcmp(gReceivedI2cMessage.event, "PublishPOS#"))
    {
        DBINFO2ln("Publish position");
        std::shared_ptr<SOPositionMessage> tempMessage (new SOPositionMessage());
        tempMessage->setMessage(idCounter++, Consignor::SO1, gReceivedI2cMessage.position);
        pComm.publishMessage("Sortic/SO1/position", Message::translateStructToString(tempMessage));
    }
    if (!strcmp(gReceivedI2cMessage.event, "PublishPAC#"))
    {
        DBINFO2ln("Publish package");
        std::shared_ptr<PackageMessage> tempMessage (new PackageMessage());
        // correct interpretation of targetDest
            // TODO
        // get target reg from package
            // TODO
        tempMessage->setMessage(idCounter++, Consignor::SO1, gReceivedI2cMessage.packageId, "cargo", (String)(gReceivedI2cMessage.targetDest), sortic.targetReg);
        pComm.publishMessage("Sortic/SO1/package", Message::translateStructToString(tempMessage));
    }
    if (!strcmp(gReceivedI2cMessage.event, "PublishERR#"))
    {
        DBINFO2ln("Publish error");
        std::shared_ptr<ErrorMessage> tempMessage (new ErrorMessage());
        tempMessage->setMessage(idCounter++, Consignor::SO1, gReceivedI2cMessage.error, gReceivedI2cMessage.token);
        pComm.publishMessage("Sortic/SO1/error", Message::translateStructToString(tempMessage));
    }
    if (!strcmp(gReceivedI2cMessage.event, "PublishINI#"))
    {
        // needed?
        DBINFO2ln("Publish init message");
        std::shared_ptr<SOInitMessage> tempMessage (new SOInitMessage());
        tempMessage->setMessage();
        pComm.publishMessage("Sortic/SO1/status", Message::translateStructToString(tempMessage));
    }
    return CommunicationCtrl::Event::NoEvent;
}

void CommunicationCtrl::exitAction_publish()
{
    DBSTATUSln("Leaving State: publish");
    // Reset I2c Event
    strcpy(gReceivedI2cMessage.event, "null#######");
}


void CommunicationCtrl::entryAction_boxCommunication(Event event)
{
    DBSTATUSln("Entering State: boxCommunication");
    currentState = State::boxCommunication;  // state transition
    doActionFPtr = &CommunicationCtrl::doAction_boxCommunication;
    currentEvent = event;
}

CommunicationCtrl::Event CommunicationCtrl::doAction_boxCommunication()
{
    DBINFO1ln("State: boxCommunication");
    pComm.loop();                       //Check for new Messages
    
    if (!errorMessageBuffer.empty())       // Check for error
    {
        Event retVal = Event::NoEvent;
        while( errorMessageBuffer.size() > 0)
        {
            if ( errorMessageBuffer.front()->error &&  errorMessageBuffer.front()->token)
            {
                retVal = Event::Error;
                errorMessageBuffer.pop_front();
            }
        }
        return retVal;
    }

    switch (this->currentEvent)
    {
    case Event::SearchBox:
    {
        // Subscribe to available boxes
        if (sortic.actualLine == Line::UploadLine)
        {
            pComm.subscribe("Box/+/available");
        }

        // Check for Available Box and store best one
        delay(TIME_BETWEEN_SUBSCRIBE);

        // stay in the loop while no box available, because it's worsed case
        if (!sbAvailableMessageBuffer.empty())
        {
            // dynamic box choice funktion noch implementieren
            for (int i = 0; i < sbAvailableMessageBuffer.size(); i++)
            {
                if (sbAvailableMessageBuffer.at(i)->targetReg == sortic.targetReg)
                {
                    DBINFO2ln("Available box for target region detected");
                    sortic.req = decodeConsignor(sbAvailableMessageBuffer.at(i)->msgConsignor);
                    sortic.targetLine = (CommunicationCtrl::Line)sbAvailableMessageBuffer.at(i)->line;
                    pComm.unsubscribe("Box/+/available");
                    sbAvailableMessageBuffer.clear();
                    previousMillisPublish = millis() - TIME_BETWEEN_PUBLISH; // for next state
                    return Event::BoxAvailable;
                }
            }                
            for (int i = 0; i < sbAvailableMessageBuffer.size(); i++)
            {
                if ((sbAvailableMessageBuffer.at(i)->targetReg == "-1"))
                {
                    // Implement dynamic box choice, now it will return everytime false!
                        // TODO
                    if (dynamicBoxChoice())
                    {
                        DBINFO2ln("Available box for target region detected");
                        sortic.req = decodeConsignor(sbAvailableMessageBuffer.at(i)->msgConsignor);
                        sortic.targetLine = (CommunicationCtrl::Line)sbAvailableMessageBuffer.at(i)->line;
                        pComm.unsubscribe("Box/+/available");
                        sbAvailableMessageBuffer.clear();
                        previousMillisPublish = millis() - TIME_BETWEEN_PUBLISH; // for next state
                        return Event::BoxAvailable;
                    }                  
                }
            }
            pComm.unsubscribe("Box/+/available");
            sbAvailableMessageBuffer.clear();
            return Event::SimulateBuffer;
        }
        return Event::NoEvent;
        break;
    }
    case Event::BoxAvailable:
    {
        // Publish request to choiced box
        std::shared_ptr<SBToSOHandshakeMessage> tempMessage(new SBToSOHandshakeMessage());
        tempMessage->setMessage(idCounter++, Consignor::SO1, sortic.req);
        pComm.subscribe("Box/" + String(sortic.req) + "/handshake");

        currentMillis = millis();
        if ((currentMillis - previousMillisPublish) > TIME_BETWEEN_PUBLISH)
        {
            previousMillisPublish = millis();
            pComm.publishMessage("Sortic/SO1/handshake", Message::translateStructToString(tempMessage));
        }
        if (!handshakeMessageSBToSOBuffer.empty() && (decodeConsignor( handshakeMessageSBToSOBuffer.front()->msgConsignor) == sortic.req) && ( handshakeMessageSBToSOBuffer.front()->req == (String)"SO1"))
        {
            sortic.ack = sortic.req;
            pComm.unsubscribe("Box/" + String(sortic.req) + "/handshake");
            handshakeMessageSBToSOBuffer.clear();
            previousMillisPublish = millis() - TIME_BETWEEN_PUBLISH;
            return Event::ReqBox;
        } 
        return Event::NoEvent;
        break;
    }
    case Event::ReqBox:
    {
        // Receive Request from choiched box
        std::shared_ptr<SBToSOHandshakeMessage> tempMessage2(new SBToSOHandshakeMessage());
        tempMessage2->setMessage(idCounter++, Consignor::SO1, sortic.req, sortic.ack, sortic.cargo, sortic.targetReg, (int)sortic.targetLine);
        pComm.subscribe("Box/" + (String)sortic.ack + "/handshake");

        currentMillis = millis();
        if ((currentMillis - previousMillisPublish) > TIME_BETWEEN_PUBLISH)
        {
            previousMillisPublish = millis();
            pComm.publishMessage("Sortic/SO1/handshake", Message::translateStructToString(tempMessage2));
        }
        if (!handshakeMessageSBToSOBuffer.empty() && (decodeConsignor( handshakeMessageSBToSOBuffer.front()->msgConsignor) == sortic.ack) && (handshakeMessageSBToSOBuffer.front()->ack == (String) "SO1"))
        {
            pComm.unsubscribe("Box/" + String(sortic.ack) + "/handshake");
            handshakeMessageSBToSOBuffer.clear();
            return Event::AnswerReceived;
        }
        return Event::NoEvent;
        break;
    }
    default:
        return Event::Error;
    }
}

void CommunicationCtrl::exitAction_boxCommunication()
{
    DBSTATUSln("Leaving State: boxCommunication");
    // reset received i2c event
    strcpy(gReceivedI2cMessage.event, "null#######");

    // set i2c write event
    strcpy(gWriteI2cMessage.event, "SortPackage");
    gWriteI2cMessage.targetLine = (uint8_t)sortic.targetLine;
    // write message
    pBus.writeMessage();
}

void CommunicationCtrl::entryAction_arrivCommunication()
{
    DBSTATUSln("Entering State: arrivCommunication");
    currentState = State::arrivConfirmation;  // state transition
    doActionFPtr = &CommunicationCtrl::doAction_arrivCommunication;
    pComm.subscribe("Box/" + (String)sortic.ack + "/state");
}

CommunicationCtrl::Event CommunicationCtrl::doAction_arrivCommunication()
{
    DBINFO1ln("State: arrivCommunication");
    pComm.loop();

    if (!errorMessageBuffer.empty())       // Check for error
    {
        Event retVal = Event::NoEvent;
        while( errorMessageBuffer.size() > 0)
        {
            if ( errorMessageBuffer.front()->error &&  errorMessageBuffer.front()->token)
            {
                retVal = Event::Error;
                errorMessageBuffer.pop_front();
            }
        }
        return retVal;
    }
    if (!sbStateMessageBuffer.empty())
    {
        if ((sbStateMessageBuffer.front()->state).equals("RetreivedPackage"))
        {
            pComm.unsubscribe("Box/" + (String)sortic.ack + "/state");
            sbStateMessageBuffer.clear();
            strcpy(gWriteI2cMessage.event, "PackageArri");
            pBus.writeMessage();
            return Event::AnswerReceived;
        }
    }
    return Event::NoEvent;
}

void CommunicationCtrl::exitAction_arrivCommunication()
{
    DBSTATUSln("Leaving State: arrivCommunication");

    // reset received i2c event
    strcpy(gReceivedI2cMessage.event, "null#######");
}

void CommunicationCtrl::entryAction_bufferSimulation()
{
    DBSTATUSln("Entering State: bufferSimulation");
    currentState = State::bufferSimulation;  // state transition
    doActionFPtr = &CommunicationCtrl::doAction_bufferSimulation;

    std::shared_ptr<BufferMessage> tempMessage (new BufferMessage());
    tempMessage->setMessage(idCounter++, Consignor::SO1, true, false);
    pComm.publishMessage("SO1/buffer", tempMessage->parseStructToString());
    pComm.subscribe("SO1/buffer");
}

CommunicationCtrl::Event CommunicationCtrl::doAction_bufferSimulation()
{
    DBINFO1ln("State: bufferSimulation");
    pComm.loop();

    if (!errorMessageBuffer.empty())       // Check for error
    {
        Event retVal = Event::NoEvent;
        while( errorMessageBuffer.size() > 0)
        {
            if ( errorMessageBuffer.front()->error &&  errorMessageBuffer.front()->token)
            {
                retVal = Event::Error;
                errorMessageBuffer.pop_front();
            }
        }
        return retVal;
    }

    if (!soBufferMessageBuffer.empty())
    {
        if (!soBufferMessageBuffer.front()->full && soBufferMessageBuffer.front()->cleared)
        {
            pComm.unsubscribe("SO1/buffer");
            soBufferMessageBuffer.clear();
            strcpy(gWriteI2cMessage.event, "PackageArri");
            pBus.writeMessage();
            return Event::AnswerReceived;
        }
    }
    return Event::NoEvent;
}

void CommunicationCtrl::exitAction_bufferSimulation()
{
    DBSTATUSln("Entering State: bufferSimulation");
    strcpy(gReceivedI2cMessage.event, "null#######");
}

void CommunicationCtrl::entryAction_errorState()
{
    DBERROR("Entering State: errorState");
    lastStateBeforeError = currentState;
    currentState = State::errorState;  // state transition
    doActionFPtr = &CommunicationCtrl::doAction_errorState;

    DBINFO2ln("Publish state");
    std::shared_ptr<SOStateMessage> tempMessage (new SOStateMessage());
    tempMessage->setMessage(idCounter++, Consignor::SO1, (String)("errorState"));
    pComm.publishMessage("Sortic/SO1/status", Message::translateStructToString(tempMessage));
    
}

CommunicationCtrl::Event CommunicationCtrl::doAction_errorState()
{
    DBINFO1ln("State: errorState");
    CommunicationCtrl::Event retVal = Event::NoEvent;
    while (!errorMessageBuffer.empty()) 
    {
        if (!errorMessageBuffer.front()->error && !errorMessageBuffer.front()->token) 
        {
            retVal = Event::Resume;
            errorMessageBuffer.pop_front();
        } 
        else if (errorMessageBuffer.front()->error && errorMessageBuffer.front()->token) 
        {
            retVal = Event::Reset;
            errorMessageBuffer.pop_front();
        }
    }
    return retVal;
}

void CommunicationCtrl::exitAction_errorState()
{
    DBSTATUSln("Leaving State: errorState");
}

void CommunicationCtrl::entryAction_resetState()
{
    DBERROR("Entering State: resetState");
    lastStateBeforeError = currentState;
    currentState = State::resetState;  // state transition
    doActionFPtr = &CommunicationCtrl::doAction_resetState;

    DBINFO2ln("Publish state");
    std::shared_ptr<SOStateMessage> tempMessage (new SOStateMessage());
    tempMessage->setMessage(idCounter++, Consignor::SO1, (String)("errorState"));
    pComm.publishMessage("Sortic/SO1/status", Message::translateStructToString(tempMessage));
}

CommunicationCtrl::Event CommunicationCtrl::doAction_resetState()
{
    DBINFO1ln("State: resetState");

    errorMessageBuffer.clear();
    sbAvailableMessageBuffer.clear();
    sbPositionMessageBuffer.clear();
    sbStateMessageBuffer.clear();
    handshakeMessageSBToSOBuffer.clear();
    soBufferMessageBuffer.clear();

    return Event::Resume;
}

void CommunicationCtrl::exitAction_resetState() 
{

    DBSTATUSln("Leaving State: resetState");
    sortic = {};  //reset struct

}

bool CommunicationCtrl::dynamicBoxChoice()
{
    return false;
}

String CommunicationCtrl::decodeEvent(Event e)
{
    switch (e)
    {
    case Event::NoEvent:
        return (String)"NoEvent";
    case Event::Publish:
        return (String)"Publish";
    case Event::SearchBox:
        return (String)"SearchBox";
    case Event::BoxAvailable:
        return (String)"BoxAvailable";
    case Event::ReqBox:
        return (String)"ReqBox";
    case Event::AnswerReceived:
        return (String)"AnswerReceived";
    case Event::NoAnswerReceived:
        return (String)"NoAnswerReceived";
    case Event::SimulateBuffer:
        return (String)"SimulateBuffer";
    case Event::ArrivConfirmation:
        return (String)"ArrivConfirmation";
    case Event::Error:
        return (String)"Error";    
    default:
        return (String)"Decode failed";
    }
}

CommunicationCtrl::Event CommunicationCtrl::decodeI2cEvent()
{
    DBFUNCCALLln("CommunicationCtrl::decodeI2cEvent()");
    String eventString = gReceivedI2cMessage.event;
    if (eventString.equals("null#######"))
    {
        return Event::NoEvent;
    }
    else if (eventString.equals("PublishSTA#"))
    {
        return CommunicationCtrl::Event::Publish;
    }
    else if (eventString.equals("PublishPOS#"))
    {
        return CommunicationCtrl::Event::Publish;
    }
    else if (eventString.equals("PublishERR#"))
    {
        return CommunicationCtrl::Event::Publish;
    }
    else if (eventString.equals("PublishPAC#"))
    {
        return CommunicationCtrl::Event::Publish;
    }
    else if (eventString.equals("BoxComm####"))
    {
        return CommunicationCtrl::Event::SearchBox;
    }
    else if (eventString.equals("ArrivConf##"))
    {
        return CommunicationCtrl::Event::ArrivConfirmation;
    }
    else
    {
        return CommunicationCtrl::Event::Error;
    }
}

String CommunicationCtrl::decodeSorticState(SorticState s)
{
    switch(s)
    {
        case SorticState::readRfidVal:
            return "State::readRfidVal";
        case SorticState::waitForSort:
            return "State::waitForSort";
        case SorticState::sortPackageInBox:
            return "State::SortPackageCtrl";
        case SorticState::waitForArriv:
            return "State::waitForArriv";
        case SorticState::errorState:
            return "State::errorState";
        case SorticState::resetState:
            return "State::resetState";
        default:
            return "ERROR: No matching state";
    }
}

String CommunicationCtrl::decodeLineToString(Line line)
{
    switch (line)
    {
    case Line::UploadLine:
        return (String)"UploadLine";
    case Line::Line1:
        return (String)"Line1";
    case Line::Line2:
        return (String)"Line2";
    case Line::Line3:
        return (String)"Line3";
    default:
        return (String)"NoLineDetected";
    }
}

String CommunicationCtrl::decodeConsignor(Consignor consignor)
{
    switch (consignor)
    {
    case Consignor::DEFUALTCONSIGNOR:
        return "DEFAULTCONSIGNOR";
    case Consignor::SB1:
        return "SB1";
    case Consignor::SB2:
        return "SB2";
    case Consignor::SB3:
        return "SB3";
    case Consignor::SO1:
        return "SO1";
    default:
        return "Error";
    }
}


/**
 * @brief 
 * 
 * @param topic 
 * @param payload 
 * @param length 
 */
void CommunicationCtrl::callback(char* topic, byte* payload, unsigned int length) 
{
    DBFUNCCALLln("callback(const char[] topic, byte* payload, unsigned int length)");
    String topic_str = String((char*)topic);
    char payload_str[MAX_JSON_PARSE_SIZE];

    for (unsigned int i = 0; i < length; i++) 
    {  // iterate message till lentgh caus it's not 0-terminated
        payload_str[i] = (char)payload[i];
    }
    // void pointer to receive translatet messagestruct to store in correct messagebuffer
    const std::shared_ptr<Message> tempMessage = Message::translateJsonToStruct(payload_str, MAX_JSON_PARSE_SIZE);
    // store messagestruct to correct buffer, check for dublicated messages
    switch ((Message::MessageType)tempMessage->msgType)
    {
    case Message::MessageType::Error:
        for (int i = 0; i < errorMessageBuffer.size(); i++)
        {
            if ((errorMessageBuffer.at(i)->msgId == tempMessage->msgId) && ( errorMessageBuffer.at(i)->msgConsignor == tempMessage->msgConsignor))
            {
                DBINFO3("Duplicated Message");
                break;
            } 
        }
        DBINFO3ln("Pushed error message to buffer");
        errorMessageBuffer.push_front(std::dynamic_pointer_cast<ErrorMessage, Message>(tempMessage));
        break;
    case Message::MessageType::SBAvailable:
        for (int i = 0; i <  sbAvailableMessageBuffer.size(); i++)
        {
            if (( sbAvailableMessageBuffer.at(i)->msgId == tempMessage->msgId) && ( sbAvailableMessageBuffer.at(i)->msgConsignor == tempMessage->msgConsignor))
            {
                DBINFO3ln("Duplicated Message");
                return;
            } 
        }
        DBINFO3ln("Pushed smartbox available message to buffer");
        sbAvailableMessageBuffer.push_front(std::dynamic_pointer_cast<SBAvailableMessage, Message>(tempMessage));
        break;
    case Message::MessageType::SBToSOHandshake:
        for (int i = 0; i <  handshakeMessageSBToSOBuffer.size(); i++)
        {
            if (( handshakeMessageSBToSOBuffer.at(i)->msgId == tempMessage->msgId) && ( handshakeMessageSBToSOBuffer.at(i)->msgConsignor == tempMessage->msgConsignor))
            {
                DBINFO3("Duplicated Message");
                break;
            } 
        }
        DBINFO3ln("Pushed smartbox to sortic handshake message to buffer");
        handshakeMessageSBToSOBuffer.push_front(std::dynamic_pointer_cast<SBToSOHandshakeMessage, Message>(tempMessage));
        break;
    case Message::MessageType::SOBuffer:
        for (int i = 0; i <  soBufferMessageBuffer.size(); i++)
        {
            if (( soBufferMessageBuffer.at(i)->msgId == tempMessage->msgId) && ( soBufferMessageBuffer.at(i)->msgConsignor == tempMessage->msgConsignor))
            {
                DBINFO3("Duplicated Message");
                break;
            } 
        }
        DBINFO3ln("Pushed smartbox to sortic handshake message to buffer");
        soBufferMessageBuffer.push_front(std::dynamic_pointer_cast<BufferMessage, Message>(tempMessage));
        break;
    default:
        break;
    }
    String currentMessage = topic_str + " " + payload_str;
    DBINFO3("CurrMessage: ");
    DBINFO3ln(currentMessage);
}
