#ifndef _NETWORK_DATE_TIME_HANDLER_H_
#define _NETWORK_DATE_TIME_HANDLER_H_

#include <WiFi.h>
#include "time.h"

const long GMT_OFFSET_SECS = -8 * 3600;
const int  DST_OFFSET_SECS = 3600;
const char NTP_SERVER[] = "pool.ntp.org";

const long int DEFAULT_WIFI_TIME = 1561847971;
const int WIFI_MAX_ATTEMPTS = 10;

char * strLocalDate(struct tm * timeinfo = NULL);
char * strLocalTime(struct tm * timeinfo = NULL);
char * strLocalDateTime(struct tm * timeinfo = NULL);

void initNetworkTime(const char * ssid = NULL, const char * password = NULL);

#endif  /* _NETWORK_DATE_TIME_HANDLER_H_ */
