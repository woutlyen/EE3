/* The event group allows multiple bits for each event, but we only care about two events:
 * - we are connected to the AP with an IP
 * - we failed to connect after the maximum amount of retries */
#define WIFI_CONNECTED_BIT BIT0
#define WIFI_FAIL_BIT      BIT1

#define ESP_WIFI_SSID      "Galaxy-S20+e7ca"
#define ESP_WIFI_PASS      "jcec9528"
#define ESP_MAXIMUM_RETRY  10