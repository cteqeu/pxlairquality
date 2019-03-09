#ifndef APP_CONF_H
#define APP_CONF_H

/* ----- data uploading style ----- */
#define APP_CONF_USE_CBOR          1u
#define APP_CONF_USE_BINARY        0u
#define APP_CONF_USE_JSON          0u

#define APP_CONF_N_SENSORS         5u

/* ----- Mbili config ----- */
#define APP_CONF_DEBUG_STREAM      Serial
#define APP_CONF_MODEM_STREAM      Serial1
#define APP_CONF_MODEM_ON_OFF_PIN  23u

/* ----- Sensor pin config ----- */
#define APP_CONF_GPS_RX_PIN        20u
#define APP_CONF_GPS_TX_PIN        21u

#define APP_CONF_PARTICLE_RX_PIN   18u
#define APP_CONF_PARTICLE_TX_PIN   19u
#define APP_CONF_PARTICLE_BAUD     9600u

#endif/*APP_CONF_H*/
