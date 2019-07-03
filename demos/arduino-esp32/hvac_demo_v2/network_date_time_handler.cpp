#include "network_date_time_handler.h"

char * strLocalDate(struct tm * timeinfo)
{
  static char datestr[32];
  int rc = 1;
  if (timeinfo == NULL) {
    timeinfo = new struct tm;
    rc = getLocalTime(timeinfo);
  }
  if(!rc){
    snprintf(datestr, sizeof(datestr), "Failed to obtain date/time");
  } else {
    strftime(datestr, sizeof(datestr), "%Y-%m-%d", timeinfo);
  }
  return datestr;
}


char * strLocalTime(struct tm * timeinfo)
{
  static char datestr[32];
  int rc = 1;
  if (timeinfo == NULL) {
    timeinfo = new struct tm;
    rc = getLocalTime(timeinfo);
  }
  if(!rc){
    snprintf(datestr, sizeof(datestr), "Failed to obtain date/time");
  } else {
    strftime(datestr, sizeof(datestr), "%H:%M:%S", timeinfo);
  }
  return datestr;
}


char * strLocalDateTime(struct tm * timeinfo)
{
  static char datestr[32];
  int rc = 1;
  if (timeinfo == NULL) {
    timeinfo = new struct tm;
    rc = getLocalTime(timeinfo);
  }
  if(!rc){
    snprintf(datestr, sizeof(datestr), "Failed to obtain date/time");
  } else {
    strftime(datestr, sizeof(datestr), "%Y-%m-%d %H:%M:%S", timeinfo);
  }
  return datestr;
}

void initNetworkTime(const char * ssid, const char * password)
{
  if (ssid == NULL) {
    Serial.printf("Connecting to previously saved network");
    WiFi.begin();
  } else {
    Serial.printf("Connecting to %s ", ssid);
  }
  WiFi.begin(ssid, password);

  int status = WiFi.status();
  int connect_attempts = 0;
  while (connect_attempts < WIFI_MAX_ATTEMPTS && status != WL_CONNECTED && status != WL_NO_SHIELD) {
    delay(500);
    Serial.print(".");
    status = WiFi.status();
    connect_attempts += 1;
  }

  configTime(GMT_OFFSET_SECS, DST_OFFSET_SECS, NTP_SERVER);

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println(" CONNECTED");
    struct tm timeinfo;
    getLocalTime(&timeinfo);
    WiFi.disconnect(true); //disconnect WiFi as it's no longer needed
    WiFi.mode(WIFI_OFF);
  }
  else {
    Serial.println(" NOT CONNECTED");
    struct timeval tv;
    tv.tv_sec = DEFAULT_WIFI_TIME;
    settimeofday(&tv, NULL);
  }
}
