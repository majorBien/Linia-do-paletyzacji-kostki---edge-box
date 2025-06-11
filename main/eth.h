/*
 * eth.h
 *
 *  Created on: 24 lut 2025
 *      Author: majorBien
 */

#ifndef MAIN_PHY_ETH_H_
#define MAIN_PHY_ETH_H_

#include <stdint.h>
#include <string.h>
#include <stdio.h>

#include "esp_eth.h" 
#include <arpa/inet.h>



#define ETH_AP_IP					"192.168.21.56"		// AP default IP
#define ETH_AP_GATEWAY				"192.168.21.1"		// AP default Gateway 
#define ETH_AP_NETMASK				"255.255.255.0"		// AP netmask
#define ETH_AP_DNS1                 "8.8.8.8"           // AP dns1
#define ETH_AP_DNS2					"8.8.4.4"			// AP dns2
#define ETH_AP_DHCP					0					// DHCP on/off

typedef struct {
    uint8_t v[4]; 
} IP_ADDR;


typedef struct {
    IP_ADDR ip;
    IP_ADDR netmask;
    IP_ADDR gw;
    IP_ADDR dns1;
    IP_ADDR dns2;
    uint8_t dhcp;
} CFG;


typedef union _DWORD_VAL {
    uint32_t Val;
    uint16_t w[2];
    uint8_t v[4];
} DWORD_VAL;


static CFG AppConfig;


void ethernet_init(void);

void getIPAddressFromString(IP_ADDR *ip, const char *ipStr);

void ethernetParamConfig(CFG *config);

void setStaticIP(CFG * config);

void eth_app_task(void);



#endif /* MAIN_PHY_ETH_H_ */
