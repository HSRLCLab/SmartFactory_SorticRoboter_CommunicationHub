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

CommunicationCtrl::CommunicationCtrl()
{
    
}

void CommunicationCtrl::loop()
{
    DBFUNCCALLln("CommunicationCtrl::loop");
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
    DBFUNCCALL("CommunicationCtrl::process ");
    DBEVENTln(String("CommunicationCtrl ") + decodeEvent(e));
    switch (currentState)
    {
    case State::idle:
        if (Event::NoEvent == e)
        {
            exitAction_idle();
            entryAction_idle();
        }
        else if (Event::PublishState == e)
        {
            exitAction_idle();
            entryAction_publishStates();
        }
        else if (Event::PublishPosition == e)
        {
            exitAction_idle();
            entryAction_publishPosition();
        }
        else if (Event::PublishPackage == e)
        {
            exitAction_idle();
            entryAction_publishPackage();
        }
        else if (Event::BoxCommunication == e)
        {
            exitAction_idle();
            entryAction_boxCommunication();
        }
        else if (Event::ArrivConfirmation == e)
        {
            exitAction_idle();
            entryAction_arrivCommunication();
        }
        else if (Event::ClearGui == e)
        {
            exitAction_idle();
            entryAction_gui(Event::ClearGui);
        }
        else if (Event::Error == e)
        {
            exitAction_idle();
            entryAction_errorState();
        }
        
        break;
    case State::publishPosition:
        if (Event::NoEvent == e)
        {
            exitAction_publishPosition();
            entryAction_idle();
        }
        else if (Event::Error == e)
        {
            exitAction_publishPosition();
            entryAction_errorState();
        }
        break;
    case State::boxCommunication:
        if (Event::SimulateBuffer == e)
        {
            exitAction_boxCommunication();
            entryAction_bufferSimulation();
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
    case State::publishPackage:
        if (Event::NoEvent == e)
        {
            exitAction_publishPackage();
            entryAction_idle();
        }
        else if (Event::Error == e)
        {
            exitAction_publishPackage();
            entryAction_errorState();
        }
        break;
    case State::publishStates:
        if (Event::NoEvent == e)
        {
            exitAction_publishStates();
            entryAction_idle(); // do we need explicit gui call? think not??
        }
        else if (Event::Error == e)
        {
            exitAction_publishPosition();
            entryAction_errorState();
        }
    case State::gui:
        // code
        break;
    default:
        break;
    }
}

void CommunicationCtrl::entryAction_idle()
{
    DBSTATUSln("Entering State: readRfidVal");
    currentState = State::idle;  // state transition
    doActionFPtr = &CommunicationCtrl::doAction_idle;

}

CommunicationCtrl::Event CommunicationCtrl::doAction_idle()
{
    DBINFO1ln("State: idle");

    // check for i2c message
    if (pBus.getReadFlag_I2c())
    {
        pBus.setReadFlag_I2c(false);
        return decodeI2cEvent();        
    }

    // check for mqtt messages
    pComm.loop();  //Check for new Messages
    if (checkForNewError()) 
    {
        return Event::Error;
    }
    

    return Event::NoEvent;
}

void CommunicationCtrl::exitAction_idle()
{
    DBSTATUSln("Leaving State: idle");
}

void CommunicationCtrl::entryAction_publishStates()
{
    DBSTATUSln("Entering State: publishStates");
    currentState = State::publishStates;  // state transition
    doActionFPtr = &CommunicationCtrl::doAction_publishStates;
}

CommunicationCtrl::Event CommunicationCtrl::doAction_publishStates()
{
    DBINFO1ln("State: publishStates")
    new SOStateMessage tempMessage;
    ParseSOStateMessage::setMessage(&tempMessage, idCounter++, Consignor::SO1, (String)"Sortic/SO1/status", pBus.getReceivedState());
    pComm.publishMessage(tempMessage.topic, translateStructToString<SOStateMessage>(&tempMessage));
    delete tempMessage;
    return CommunicationCtrl::Event::NoEvent;
}

void CommunicationCtrl::exitAction_publishStates()
{
    DBSTATUSln("Leaving State: publishStates");
    //pBus.resetMessages();
}

void CommunicationCtrl::entryAction_publishPosition()
{
    DBSTATUSln("Entering State: publishPosition");
    currentState = State::publishPosition;  // state transition
    doActionFPtr = &CommunicationCtrl::doAction_publishPosition;
}

CommunicationCtrl::Event CommunicationCtrl::doAction_publishPosition()
{
    DBINFO1ln("State: publishPosition")
    new SOPositionMessage tempMessage;
    ParseSOPositionMessage::setMessage(&tempMessage, idCounter++, Consignor::SO1, (String)"Sortic/SO1/position", pBus.getReceivedPosition());
    pComm.publishMessage(translateStructToString<SOPositionMessage>(&tempMessage));
    delete tempMessage;;
    return CommunicationCtrl::Event::NoEvent;
}

void CommunicationCtrl::exitAction_publishPosition()
{
    DBSTATUSln("Leaving State: publishPosition");
    // pBus.resetMessages();
}

void CommunicationCtrl::entryAction_publishPackage()
{
    DBSTATUSln("Entering State: publishPackage");
    currentState = State::publishPackage;  // state transition
    doActionFPtr = &CommunicationCtrl::doAction_publishPackage;
    pComm.clear();
}

CommunicationCtrl::Event CommunicationCtrl::doAction_publishPackage()
{
    DBINFO1ln("State: publishPackage");

    // store received packageinformations
    pPackage = pBus.getReceivedPackageInformation();

    // get target region of package
        //TODO

    // publish package information via MQTT
    new PackageMessage tempMessage;
    ParsePackageMessage::setMessage(&tempMessage, idCounter++, Consignor::SO1, (String)"Sortic/SO1/package", pPackage.id, pPackage.cargo, pPackage.targetDest, pPackage.targetReg);
    pComm.publishMessage(tempMessage.topic, translateStructToString<PackageMessage>(&tempMessage));
    delete tempMessage;

    return CommunicationCtrl::Event::NoEvent;
}

void CommunicationCtrl::exitAction_publishPackage()
{
    DBSTATUSln("Leaving State: publishPackage");
    // pBus.resetMessages();
}

void CommunicationCtrl::entryAction_boxCommunication()
{
    DBSTATUSln("Entering State: boxCommunication");
    currentState = State::boxCommunication;  // state transition
    doActionFPtr = &CommunicationCtrl::doAction_boxCommunication;

    previousMillis = millis();
    previousMillisPublish = previousMillis;
    currentMillis = previousMillis;

    pComm.clear();
    // Subscribe to available boxes
    if (sortic.actualLine == Line::UploadLine)
    {
        pComm.subscribe("Box/+/available");
    }     
    sortic.ack = "null";
    sortic.req = "null";

    // unsubscribe old topics
}

CommunicationCtrl::Event CommunicationCtrl::doAction_boxCommunication()
{
    DBINFO1ln("State: bocCommunication");
    // wait time to receive messages
    do
    {
        delay(50);
        currentMillis = millis();
    } while (!(currentMillis - previousMillis > SORTIC_WAITFOR_BOX_SECONDS * 1000));
    

    pComm.loop();               //Check for new Messages
    if (checkForNewError())     // Check for error
    {
        return Event::Error;
    }

    // Check for Available Box and store best one
    if (pComm.checkForNewAvailableBox(Consignor::SB1) || pComm.checkForNewAvailableBox(Consignor::SB1) || pComm.checkForNewAvailableBox(Consignor::SB3))
    {
        // dynamic box choice funktion noch implementieren
        if (!dynamicBoxChoice()) // set here sortic.req with req SB id
        {
            return Event::SimulateBuffer;
        } 
    }
    else
    {
        return Event::SimulateBuffer; // may something other?
    }

    // Publish request to choiced box
    new SBtoSOHandshakeMessage tempMessage;
    ParseSBtoSOHandshakeMessage::setMessage(&tempMessage, idCounter++, Consignor::SO1, (String)"Sortic/SO1/handshake", (String)sortic.req);
    pComm.subscribe("Box/" + String(sortic.req) + "/handshake");
    
    // publish 4 times
    for (int i = 0; i < 4; i++)
    {
        previousMillis = millis();
        previousMillisPublish = previousMillis;
        currentMillis = previousMillis;
        pComm.publishMessage(tempMessage.topic, translateStructToString<SBtoSOHandshakeMessage>(&tempMessage));
        do
        { 
            delay(50);
            currentMillis = millis();
        } while (!((currentMillis - previousMillisPublish) > TIME_BETWEEN_PUBLISH));
    }
    delete tempMessage;
    
    
    // Receive Request from choiched box
    pComm.loop(); // check for new messages
    if (checkForNewError()) // check for error
    {
        return Event::Error;
    }
    if (checkForNewHandshakeSBToSO(Consignor::SB)) // get right consignor!!
    {
        if (((handshakeMessageSBToSOBuffer[0][Consignor::SB - 1]).req == sortic.id) && ((handshakeMessageSBToSOBuffer[0][Consignor::SB - 1]).id == sortic.req))
        {
            sortic.ack = sortic.req;
            sortic.targetLine = availableMessageBuffer[0][Consignor::SB -1];
        }
        else
        {
            DBWARNINGln("Request failed");
            // return
        } 
    }
    else
    {
        return Event::NoAnswerReceived;
    }


    // Publish acknoladge to choiced box
    new SBtoSOHandshakeMessage tempMessage;
    ParseSBtoSOHandshakeMessage::setMessage(&tempMessage, idCounter++, Consignor::SO1, (String)"Sortic/SO1/handshake", (String)sortic.req, sortic.ack, pPackage.targetReg, sortic.targetLine);
    pComm.subscribe("Box/" + String(sortic.req) + "/handshake");
    
    // publish 4 times
    for (int i = 0; i < 4; i++)
    {
        previousMillis = millis();
        previousMillisPublish = previousMillis;
        currentMillis = previousMillis;
        pComm.publishMessage(tempMessage.topic, translateStructToString<SBtoSOHandshakeMessage>(&tempMessage));
        do
        { 
            delay(50);
            currentMillis = millis();
        } while (!((currentMillis - previousMillisPublish) > TIME_BETWEEN_PUBLISH));
    }
    delete tempMessage;

    // Receive acknoledge from choiced box
    do
    {
        if (checkForNewHandshakeSBToSO(Consignor::SB))
        {
            // check if message is from ack box and if box does ack sortic
            if (((handshakeMessageSBToSOBuffer[0][Consignor::SB - 1]).ack == sortic.id) && ((handshakeMessageSBToSOBuffer[0][Consignor::SB - 1]).id == sortic.ack))
            {
                sortic.ack = tempMessage.id;
                return Event::AnswerReceived;
            }
            else
            {
                return Event::NoAnswerReceived;
            }
        }
        currentMillis = millis();
    } while (!checkForNewHandshakeSBToSO(Consignor::SB) || (currentMillis - previousMillis) < (SORTIC_ITERATION_VACKS_SECONDS * 1000));
    
    // return if no acknoledge
    if (sortic.ack == "null")
    {
        return Event::NoAnswerReceived;
    }

}

void CommunicationCtrl::exitAction_boxCommunication()
{
    DBSTATUSln("Leaving State: boxCommunication");
    // pBus.resetMessages();
    // reset MQTT-Messages
}

CommunicationCtrl::Event CommunicationCtrl::decodeI2cEvent()
{
    String eventString = pBus.getReceivedEvent();
    if (eventString.equals("PublishState"))
    {
        return CommunicationCtrl::Event::PublishState;
    }
    else if (eventString.equals("PublishPosition"))
    {
        return CommunicationCtrl::Event::PublishPosition;
    }
    else if (eventString.equals("PublishPackage"))
    {
        return CommunicationCtrl::Event::PublishPackage;
    }
    else if (eventString.equals("BoxCommunication"))
    {
        return CommunicationCtrl::Event::BoxCommunication;
    }
    else if (eventString.equals("ArrivConfirmation"))
    {
        return CommunicationCtrl::Event::ArrivConfirmation;
    }
    else if (eventString.equals("ClearGui"))
    {
        return CommunicationCtrl::Event::ClearGui;
    }
    else
    {
        return CommunicationCtrl::Event::Error;
    }
}

String CommunicationCtrl::decodeLine(Line line)
{
    switch (line)
    {
    case Line::UploadLine:
        return (String)"UploadLine";
        break;
    case Line::Line1:
        return (String)"Line1";
        break;
    case Line::Line2:
        return (String)"Line2";
        break;
    case Line::Line3:
        return (String)"Line3";
    default:
        return (String)"NoLineDetected";
        break;
    }
}

bool CommunicationCtrl::checkForError() {
    DBFUNCCALLln("CommunicationCtrl::checkForError()");
    if (!pComm.isEmpty()) {
        DBINFO3ln(pComm.size());
        DBINFO3ln(pComm.last().error);
        if (errorMessageBuffer) {
            pComm.shift();
            return true;
        }
    }
    return false;
}

