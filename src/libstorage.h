/*
 * Storage utilities for persisting credentials in NVS
 */

#ifndef LIBSTORAGE_H
#define LIBSTORAGE_H

#include <Arduino.h>
extern String versionFirmware;
// Wi‑Fi credentials
bool saveWiFiCredentials(const String &ssid, const String &password);
bool loadWiFiCredentials(String &outSsid, String &outPassword);
bool clearWiFiCredentials();
bool hasWiFiCredentials();
bool saveVersionFirmware(const String version);
void loadVersionFirmware();
#endif /* LIBSTORAGE_H */
