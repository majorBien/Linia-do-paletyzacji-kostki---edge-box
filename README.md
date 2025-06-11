# EthernetLib-ESP32-LAN8720

A simple Ethernet library for the **ESP32** with the **LAN8720** chip using the **ESP-IDF** framework.

## Features
This library provides the following functions:
- `void ethernet_init(void)`  
  Initialize Ethernet connection
- `void getIPAddressFromString(IP_ADDR *ip, const char *ipStr)`  
  Convert IP string to address structure
- `void ethernetParamConfig(CFG *config)`  
  Configure Ethernet parameters
- `void setStaticIP(CFG *config)`  
  Set static IP configuration
- `void eth_app_task(void)`  
  Main Ethernet application task

## Building and Flashing
Make sure you have ESP-IDF installed and set up. Then use:


idf.py build
idf.py flash
idf.py monitor
Requirements
ESP-IDF v5.x or newer

ESP32 board with LAN8720 chip
