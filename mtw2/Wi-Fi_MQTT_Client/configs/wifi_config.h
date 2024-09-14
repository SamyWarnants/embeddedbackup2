#ifndef WIFI_CONFIG_H_
#define WIFI_CONFIG_H_

#include "cy_wcm.h"

/*******************************************************************************
* Macros
********************************************************************************/
/* SSID of the Wi-Fi Access Point to which the MQTT client connects. */
#define WIFI_SSID                           "GalaxyVanrykel"
#define WIFI_SSID_2                         "wifi.pxl-ea-ict.be"
#define WIFI_SSID_3                         "Senna"
/* Passkey of the above mentioned Wi-Fi SSID. */
#define WIFI_PASSWORD                      "ogkm3652"
#define WIFI_PASSWORD_2                    "elektronica"
#define WIFI_PASSWORD_3                    "josse03091957"

#define onschool   (1)
/* Security type of the Wi-Fi access point. See 'cy_wcm_security_t' structure
 * in "cy_wcm.h" for more details.
 */
#define WIFI_SECURITY                     CY_WCM_SECURITY_WPA2_AES_PSK

/* Maximum Wi-Fi re-connection limit. */
#define MAX_WIFI_CONN_RETRIES             (3u)

/* Wi-Fi re-connection time interval in milliseconds. */
#define WIFI_CONN_RETRY_INTERVAL_MS       (50)

#endif /* WIFI_CONFIG_H_ */
