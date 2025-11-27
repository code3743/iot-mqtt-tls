
#pragma once
#include <Arduino.h>
#include <LittleFS.h>


void portalBegin();
void portalLoop();
bool portalActive();

typedef void (*PortalSaveCallback)(const String &ssid, const String &password);
void portalOnSave(PortalSaveCallback cb);
