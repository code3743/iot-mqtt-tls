#include "libportal.h"
#include <WiFi.h>
#include <WebServer.h>
#include <LittleFS.h>
#include <DNSServer.h>
#include <libstorage.h>

static WebServer server(80);
static DNSServer dns;
static bool s_active = false;
static PortalSaveCallback saveCallback = nullptr;
static const byte DNS_PORT = 53;

// -----------------------
// Tipo MIME
// -----------------------
String getContentType(const String &path) {
  if (path.endsWith(".html")) return "text/html";
  if (path.endsWith(".css")) return "text/css";
  if (path.endsWith(".js")) return "application/javascript";
  if (path.endsWith(".png")) return "image/png";
  if (path.endsWith(".jpg")) return "image/jpeg";
  if (path.endsWith(".ico")) return "image/x-icon";
  return "text/plain";
}

// -----------------------
// Manejo de archivos
// -----------------------
void handleFileRequest() {
  String path = server.uri();

  // Si vienen de google.com u otra URL con path raro
  if (!path.startsWith("/")) {
    server.sendHeader("Location", "/");
    server.send(302, "text/plain", "");
    return;
  }

  if (path == "/") path = "/index.html";

  if (!LittleFS.exists(path)) {
    server.sendHeader("Location", "/index.html");
    server.send(302, "text/plain", "");
    return;
  }

  File file = LittleFS.open(path, "r");
  server.streamFile(file, getContentType(path));
  file.close();
}

// -----------------------
// Guardado de WiFi
// -----------------------
void handleSave() {
  if (!server.hasArg("ssid")) {
    server.send(400, "text/plain", "ssid requerido");
    return;
  }

  String ssid = server.arg("ssid");
  String pwd  = server.arg("password");

  if (!saveWiFiCredentials(ssid, pwd)) {
    server.send(500, "text/plain", "no se pudo guardar");
    return;
  }

  if (saveCallback) saveCallback(ssid, pwd);

  server.send(200, "text/plain", "Guardado. Reiniciando...");
  delay(500);
  ESP.restart();
}


void portalBegin() {
  LittleFS.begin(true);


  WiFi.softAPConfig(
    IPAddress(192,168,4,1),
    IPAddress(192,168,4,1),
    IPAddress(255,255,255,0)
  );

  WiFi.mode(WIFI_AP);
  String apName = String("ESP32-Portal-") + String((uint32_t)ESP.getEfuseMac(), HEX);
  WiFi.softAP(apName.c_str());


  dns.start(DNS_PORT, "*", WiFi.softAPIP());


  server.on("/generate_204", []() {  // Android
    server.sendHeader("Location", "/");
    server.send(302, "text/plain", "");
  });

  server.on("/hotspot-detect.html", []() { // iOS/macOS
    server.sendHeader("Location", "/");
    server.send(302, "text/plain", "");
  });

  server.on("/fwlink", []() { // Windows
    server.sendHeader("Location", "/");
    server.send(302, "text/plain", "");
  });

  server.on("/ncsi.txt", []() { // Linux
    server.sendHeader("Location", "/");
    server.send(302, "text/plain", "");
  });

  server.on("/save", HTTP_POST, handleSave);


  server.onNotFound(handleFileRequest);

  server.begin();
  s_active = true;
}

void portalLoop() {
  if (!s_active) return;
  dns.processNextRequest();
  server.handleClient();
}

bool portalActive() { return s_active; }

void portalOnSave(PortalSaveCallback cb) {
  saveCallback = cb;
}
