/*
 * eth.c
 *
 *  Created on: 24 lut 2025
 *      Author: majorBien
 */



#include "eth.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_event.h"
#include "esp_netif.h"
#include "esp_eth.h"
#include "lwip/dns.h"
#include "esp_log.h"
#include "tasks_common.h"
#include "driver/gpio.h"

#define PIN_PHY_CLK_EN 2



static const char *TAG = "eth";

static esp_netif_t *eth_netif = NULL;
static esp_eth_handle_t eth_handle = NULL;

static void eth_event_handler(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data) {
    uint8_t mac_addr[6] = {0};
    esp_eth_handle_t eth_handle = *(esp_eth_handle_t *)event_data;

    switch (event_id) {
        case ETHERNET_EVENT_CONNECTED:
            esp_eth_ioctl(eth_handle, ETH_CMD_G_MAC_ADDR, mac_addr);
            ESP_LOGI(TAG, "Ethernet Link Up");
            ESP_LOGI(TAG, "Ethernet HW Addr %02x:%02x:%02x:%02x:%02x:%02x",
                     mac_addr[0], mac_addr[1], mac_addr[2], mac_addr[3], mac_addr[4], mac_addr[5]);
            break;
        case ETHERNET_EVENT_DISCONNECTED:
            ESP_LOGI(TAG, "Ethernet Link Down");
            break;
        case ETHERNET_EVENT_START:
            ESP_LOGI(TAG, "Ethernet Started");
            break;
        case ETHERNET_EVENT_STOP:
            ESP_LOGI(TAG, "Ethernet Stopped");
            break;
        default:
            break;
    }
}


static void got_ip_event_handler(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data) {
    ip_event_got_ip_t *event = (ip_event_got_ip_t *)event_data;
    const esp_netif_ip_info_t *ip_info = &event->ip_info;

    ESP_LOGI(TAG, "Ethernet Got IP Address");
    ESP_LOGI(TAG, "~~~~~~~~~~~");
    ESP_LOGI(TAG, "ETHIP:" IPSTR, IP2STR(&ip_info->ip));
    ESP_LOGI(TAG, "ETHMASK:" IPSTR, IP2STR(&ip_info->netmask));
    ESP_LOGI(TAG, "ETHGW:" IPSTR, IP2STR(&ip_info->gw));
    ESP_LOGI(TAG, "~~~~~~~~~~~");
}


void ethernet_init(void) {

    esp_rom_gpio_pad_select_gpio(PIN_PHY_CLK_EN);
    gpio_set_direction(PIN_PHY_CLK_EN, GPIO_MODE_OUTPUT);
    gpio_set_level(PIN_PHY_CLK_EN, 1);

    esp_netif_config_t cfg = ESP_NETIF_DEFAULT_ETH();
    eth_netif = esp_netif_new(&cfg);

    ESP_ERROR_CHECK(esp_event_handler_register(ETH_EVENT, ESP_EVENT_ANY_ID, &eth_event_handler, NULL));
    ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_ETH_GOT_IP, &got_ip_event_handler, NULL));

    eth_mac_config_t mac_config = ETH_MAC_DEFAULT_CONFIG();
    eth_phy_config_t phy_config = ETH_PHY_DEFAULT_CONFIG();
    phy_config.autonego_timeout_ms = 2000; 
    phy_config.phy_addr = 0;
    phy_config.reset_gpio_num = 5;

    eth_esp32_emac_config_t esp32_emac_config = ETH_ESP32_EMAC_DEFAULT_CONFIG();
    esp32_emac_config.smi_mdc_gpio_num = CONFIG_EXAMPLE_ETH_MDC_GPIO; 
    esp32_emac_config.smi_mdio_gpio_num = CONFIG_EXAMPLE_ETH_MDIO_GPIO; 

    esp_eth_mac_t *mac = esp_eth_mac_new_esp32(&esp32_emac_config, &mac_config);

    esp_eth_phy_t *phy = esp_eth_phy_new_lan87xx(&phy_config);
    esp_eth_config_t config = ETH_DEFAULT_CONFIG(mac, phy);
    
    ESP_ERROR_CHECK(esp_eth_driver_install(&config, &eth_handle));


    ESP_ERROR_CHECK(esp_netif_attach(eth_netif, esp_eth_new_netif_glue(eth_handle)));
    ESP_ERROR_CHECK(esp_eth_start(eth_handle));
}

void getIPAddressFromString(IP_ADDR *ip, const char *ipStr) {
    inet_aton(ipStr, (struct in_addr *)ip->v);
}


void ethernetParamConfig(CFG *config)
{
    getIPAddressFromString(&config->ip, ETH_AP_IP);
    getIPAddressFromString(&config->netmask, ETH_AP_NETMASK);
    getIPAddressFromString(&config->gw, ETH_AP_GATEWAY);
    getIPAddressFromString(&config->dns1, ETH_AP_DNS1);
    getIPAddressFromString(&config->dns2, ETH_AP_DNS2);
    config->dhcp = ETH_AP_DHCP; 
}

void setStaticIP(CFG * config)
{
    if (config->dhcp == 0) {
        esp_netif_ip_info_t ip_info;
        ip_info.ip.addr = htonl((config->ip.v[0] << 24) | (config->ip.v[1] << 16) | (config->ip.v[2] << 8) | config->ip.v[3]);
        ip_info.netmask.addr = htonl((config->netmask.v[0] << 24) | (config->netmask.v[1] << 16) | (config->netmask.v[2] << 8) | config->netmask.v[3]);
        ip_info.gw.addr = htonl((config->gw.v[0] << 24) | (config->gw.v[1] << 16) | (config->gw.v[2] << 8) | config->gw.v[3]);

        ESP_ERROR_CHECK(esp_netif_dhcpc_stop(eth_netif)); 
        ESP_ERROR_CHECK(esp_netif_set_ip_info(eth_netif, &ip_info));

        ip_addr_t dns1, dns2;
        dns1.u_addr.ip4.addr = (config->dns1.v[0] << 24) | (config->dns1.v[1] << 16) | (config->dns1.v[2] << 8) | config->dns1.v[3];
        dns2.u_addr.ip4.addr = (config->dns2.v[0] << 24) | (config->dns2.v[1] << 16) | (config->dns2.v[2] << 8) | config->dns2.v[3];

        dns_setserver(0, &dns1);
        dns_setserver(1, &dns2);
    }

}

void eth_app_task(void)
{
	ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
	ethernetParamConfig(&AppConfig);
    ethernet_init();
	setStaticIP(&AppConfig);

}


