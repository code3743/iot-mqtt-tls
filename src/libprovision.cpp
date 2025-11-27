#include <WiFi.h>
#include <WebServer.h>
#include <libstorage.h>
#include <libprovision.h>

static WebServer server(80);
static bool s_isProvisioning = false;

static const char FORM_HTML[] PROGMEM = R"HTML(
<!DOCTYPE html><html lang="es"><head><meta charset="UTF-8" /><meta name="viewport" content="width=device-width, initial-scale=1.0" /><title>Configurar Wi-Fi</title><style> body { margin: 0; padding: 0; font-family: "Inter", sans-serif; background: linear-gradient(135deg, #4f46e5, #3b82f6); color: #ffffff; display: flex; justify-content: center; align-items: center; height: 100vh; } .card { background: #ffffff; color: #1f2937; width: 90%; max-width: 380px; border-radius: 20px; padding: 32px; box-shadow: 0 20px 40px rgba(0,0,0,0.2); text-align: center; animation: fadeIn 0.8s ease; } @keyframes fadeIn { from {opacity: 0; transform: translateY(10px);} to {opacity: 1; transform: translateY(0);} } h2 { margin-bottom: 12px; font-size: 24px; font-weight: 700; color: #111827; } p { margin-bottom: 20px; color: #4b5563; font-size: 14px; } input { width: 100%; padding: 12px; margin-bottom: 16px; border: 2px solid #e5e7eb; border-radius: 12px; font-size: 16px; outline: none; transition: 0.3s; } input:focus { border-color: #6366f1; box-shadow: 0 0 0 3px rgba(99, 102, 241, 0.3); } button { width: 100%; padding: 14px; background: #4f46e5; color: white; font-size: 16px; border: none; border-radius: 12px; cursor: pointer; transition: 0.3s; font-weight: 600; } button:hover { background: #4338ca; } .footer { margin-top: 16px; font-size: 12px; color: #9ca3af; } </style></head><body><div class="card"><h2>Conectar a Wi-Fi</h2><p>Introduce tus credenciales para que el dispositivo se conecte a tu red.</p><form method="POST" action="/save"><input placeholder="SSID" name="ssid" required /><input placeholder="Contraseña" name="password" type="password" /><button type="submit">Guardar configuración</button></form><div class="footer">ESP32 — Config Portal</div></div></body></html>
)HTML";

static void handleRoot() { server.send(200, "text/html", FORM_HTML); }

static void handleSave() {
  if (!server.hasArg("ssid")) { server.send(400, "text/plain", "ssid requerido"); return; }
  String ssid = server.arg("ssid");
  String pwd  = server.arg("password");
  if (!saveWiFiCredentials(ssid, pwd)) { server.send(500, "text/plain", "no se pudo guardar"); return; }
  server.send(200, "text/plain", "Guardado. Reiniciando...");
  delay(500);
  ESP.restart();
}

void startProvisioningAP() {
  WiFi.mode(WIFI_AP);
  String apName = String("ESP32-Setup-") + String((uint32_t)ESP.getEfuseMac(), HEX);
  WiFi.softAP(apName.c_str());
  IPAddress ip = WiFi.softAPIP();
  Serial.print("Provisioning AP "); Serial.print(apName); Serial.print(" IP "); Serial.println(ip);
  server.on("/", HTTP_GET, handleRoot);
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
  server.begin();
  s_isProvisioning = true;
}

void provisioningLoop() {
  if (!s_isProvisioning) return;
  server.handleClient();
}

bool isProvisioning() { return s_isProvisioning; }
