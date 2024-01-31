## MQTT client for embedded systems
The purpose of this project was to implement MQTT client library from scratch - keeping in mind requirements for usage in embedded systems. The most important features:
- written to be compliant with MQTT 3.1.1 - handles QoS 0, 1 and 2
- uses raw API of LwIP network stack - operating system is not required
- configurable maximum length of messages and maximum number of active conversations with the broker.

## Examples on how to use the library
In the file main.c there is an example on how to use the library. It includes:
- initialization of the client, setting connection parameters and pointer to function returning elapsed time [ms] from start of the system
- setting functions to be called when message is sent/received or subscription is completed
- connection to the broker
- publishing messages and subscribing to topics
- looping to receive messages and handle keepalive.

## Test setup
The library was run on NUCLEO-F429ZI development board, connection tested with Mosquitto broker.
