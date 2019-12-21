# SmartFactory_SorticRoboter_CommunicationHub

The [SmartFactory_SorticRoboter_CommunicationHub](https://github.com/philipzellweger/SmartFactory_SorticRoboter_CommunicationHub) is the communication module for the [SmartFactory_SorticRoboter](https://github.com/philipzellweger/SmartFactory_SorticRoboter). It enables communication via Wifi based on the MQTT protocol amongts all participants in the SmartFactory project, for example with the [SmartFactory_Box-Sortic](https://github.com/LMazzole/SmartFactory_Box-Sortic). The connection to the [SmartFactory_SorticRoboter](https://github.com/philipzellweger/SmartFactory_SorticRoboter) is via i2c data bus. The received messages are serialized for simple and dynamic message management.

The programmed algorithm is based on the finite state machine design pattern. 
The design pattern allows to give the roboter a state and to change its state to another state by events.
The robot is built by different nested state machines and thus executes its tasks.


## Table of contents

- Tools and technologies
   - Visual Studio Code
   - Doxygen
   - I2C
   - MQTT
   - Deque
   - Shared pointer
- Hardware
   - SorticRoboter CommunicationHub
     - ESP32-DevKitC
     - Logic Level converter
- Software
   - Finite State Machine
   - Communication
   - UML
   - Dependency Graph
   - Collaboration Diagram
- ToDo's
- Contributors
- Changelog
- License
- Links to SmartFactory

## Tools and technologies

The source code is implemented in the programming language C++. In the following, the tools for editing the project are listed.

#### Visual Studio Code
The development environment used is [Visual Studio Code](https://code.visualstudio.com/) with the [PlatformIO extension](https://docs.platformio.org/en/latest/ide/vscode.html). The development environment can be downloaded open sourced. For an installation guide look [here](https://github.com/philipzellweger/SmartFactory_SorticRoboter_CommunicationHub/blob/master/docs/Installation_Guide_SmartFactory.pdf).  

#### Doxygen
Doxygen was used for documenting the source code. For using Doxygen in Visual Studio code, the [Doxygen Documentation Generator](https://marketplace.visualstudio.com/items?itemName=cschlosser.doxdocgen) extension is available.

#### I2C

The connection to the hub is via i2c. For an explanation of the technology and the library look [here](https://github.com/philipzellweger/SmartFactory_I2cCommunication).

#### MQTT

The communication protocol used to communicate via Wifi is MQTT. For an explanation of the technology look [here](https://github.com/philipzellweger/SmartFactory_MQTTCommunication).

#### Deque

The messages received via MQTT are stored in a Deque of shared pointers after serialization. A Deque is a Queue, which can be accessed from both sides. Here you can find an explanation about [Deque](https://de.cppreference.com/w/cpp/container/deque).

#### Shared pointer

Shared pointers are used to handke dynamically created objects. The advantage of shared pointers is that direct control over deleting dynamically created objects is not needed. As soon as no pointer points to the created object, the object is automatically deleted. This concept makes the factory design pattern very powerful. An explanation of shared pointer can be found [here](https://de.cppreference.com/w/cpp/memory/shared_ptr).

## Hardware 

For the i2c connection between the Arduino Uno and the ESP32-DevKitC a Logic Level converter is required. This is necessary due to the different voltage levels of the microcontrollers, otherwise damage can occur or communication cannot be stable guaranteed. 

### SorticRoboter CommunicationHub

The communication hub is structured with the following elements.
- ESP32-DevKitC
- Logic Level converter

#### ESP32-DevKitC

The microcontroller used for the communication hub is an [Esp32-DevKitC](https://www.espressif.com/en/products/hardware/esp32-devkitc/overview). The microcontroller is very powerful and has a large memory and is very cheap compared to other microcontrollers. In addition, it has a Wifi chip which enables communication via MQTT.

![esp32](https://cdn.sos.sk/productdata/90/d5/9dcaac3b/esp32-devkitc.jpg)

[Image: [SOS electronic: Esp32 DevKitC](https://www.soselectronic.de/products/espressif/esp32-devkitc-ver-d-305403)]

#### Logic Level converter

A Logic Level converter is required for the i2c connection. The Logic Level converter transforms the signal voltage of the Arduino Uno from 5V to the signal voltage of the Esp32-DevKitC from 3.3V.

![levelconverter](https://www.distrelec.ch/Web/WebShopImages/landscape_large/1-/01/Adafruit-757-30091221-01.jpg)
[Image: [Distrelec: Logic Level converter](https://www.distrelec.ch/en/bss138-bi-directional-logic-level-converter-adafruit-757-logic-level-converter/p/30091221)]

## Software

#### Finite State Machine

The design pattern used to implement the software is the Finite State Machine. The robot always has a state. The states are transformed into other states by events. The figure below shows the finite state machine of the [SmartFactory_SorticRoboter_CommunicationHub](https://github.com/philipzellweger/SmartFactory_SorticRoboter_CommunicationHub) seen in the read area. The [SmartFactory_SorticRoboter_CommunicationHub](https://github.com/philipzellweger/SmartFactory_SorticRoboter_CommunicationHub) has only one Finite State Machine

![FSM](https://github.com/philipzellweger/SmartFactory_SorticRoboter_CommunicationHub/blob/master/docs/FSM_MASTER.jpg)

[Image: Finite State Machine SorticRoboter with SorticRoboter CommunicationHub]

#### Communication

Communication with the box via the MQTT protocol is done via the topics shown in the figure below. The messages with defined topics are sent to via the broker to to any opposite participant which subscribed to the defined topic.

![TopicTree](https://github.com/philipzellweger/SmartFactory_SorticRoboter_CommunicationHub/blob/master/docs/Topics.jpg)

[Image: Topic tree communication between SorticRoboter and SmartBox]

If an available package needs to be sorted in a [SmartFactory_Box-Sortic](https://github.com/LMazzole/SmartFactory_Box-Sortic) a handshake with an available [SmartFactory_Box-Sortic](https://github.com/LMazzole/SmartFactory_Box-Sortic) is performed. The process flow is shown in the graph below.

![Communicationflow](https://github.com/philipzellweger/SmartFactory_SorticRoboter_CommunicationHub/blob/master/docs/SorticToSmartBox.jpg)

[Image: Process flow communication between SorticRoboter and SmartBox]

#### UML

The figure below shows the data model in UML notation. The core of the communication hub is the serialization of the received messages. A library has been implemented for this purpose, which performs this serialization.

<p align="center">
    <a href=https://github.com/philipzellweger/SmartFactory_SorticRoboter_CommunicationHub/blob/master/docs/html/class_communication_ctrl.html>
        <img src="https://github.com/philipzellweger/SmartFactory_SorticRoboter_CommunicationHub/blob/master/docs/html/class_communication_ctrl__coll__graph.png" style="border:none;"/>
    </a>
    <p align="center"><small>Click on the image to open doxygen-documentation.</p>
</p>

#### Dependency Graph

The figure below shows the dependency tree of the main FSM CommunicationCtrl.
The used extern libraries for the project:
* [SmartFactory_I2cCommunication](https://github.com/philipzellweger/SmartFactory_I2cCommunication)
* [SmartFactory_MQTTCommunication](https://github.com/philipzellweger/SmartFactory_MQTTCommunication)
* [SmartFactory_Messages](https://github.com/philipzellweger/SmartFactory_Messages)

<p align="center">
    <a href=https://github.com/philipzellweger/SmartFactory_SorticRoboter_CommunicationHub/blob/master/docs/html/main_8cpp.html>
        <img src="https://github.com/philipzellweger/SmartFactory_SorticRoboter_CommunicationHub/blob/master/docs/html/main_8cpp__incl.png" style="border:none;"/>
    </a>
    <p align="center"><small>Click on the image to open doxygen-documentation.</p>
</p>

## ToDo's

- [ ]
- [ ]

# Contributors
- [Philip Zellweger](https://github.com/philipzellweger)

# Changelog

V 1.0   -	Release SA HS20 -	[Philip Zellweger](https://github.com/philipzellweger)

# License

MIT License

# Links to SmartFactory
- [SmartFactory-Sortic](https://github.com/LMazzole/SmartFactory-Sortic)
- [SmartFactory_Box-Sortic](https://github.com/LMazzole/SmartFactory_Box-Sortic)
- [SmartFactory_Vehicle-Sortic](https://github.com/LMazzole/SmartFactory_Vehicle-Sortic)
- [SmartFactory_Vehicle-Basis](https://github.com/LMazzole/SmartFactory_Vehicle-Basis)
- [SmartFactory_SorticRoboter](https://github.com/philipzellweger/SmartFactory_SorticRoboter)
- [SmartFactory_SorticRoboter_CommunicationHub](https://github.com/philipzellweger/SmartFactory_SorticRoboter_CommunicationHub)
- [SmartFactory_MQTTCommunication](https://github.com/LMazzole/SmartFactory_MQTTCommunication) for Adafruit Feather M0 Wifi
- [SmartFactory_MQTTCommunication](https://github.com/philipzellweger/SmartFactory_MQTTCommunication) for Esp32 DevKitC
- [SmartFactory_I2cCommunication](https://github.com/philipzellweger/SmartFactory_I2cCommunication)
- [SmartFactory_Messages](https://github.com/philipzellweger/SmartFactory_Messages)
