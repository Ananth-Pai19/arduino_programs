/*
 * wifi_control.cpp — hotspot + tiny web server for the phone page.
 * You normally only touch the CONFIG block.
 */
#include <Arduino.h>
#include <WiFiS3.h>
#include "wifi_control.h"
#include "page.h"

// ====================== CONFIG ======================
static const char* AP_SSID = "ColourBot";
static const char* AP_PASS = "colourbot123";   // >= 8 characters
// ====================================================

static WiFiServer server(80);

// What the phone sees
static String stState = "idle", stMsg = "Ready", stPick = "", stDrop = "";
static bool   stMagnet = false;

// What the sketch picks up
static bool   missionPending = false, stopPending = false;
static String mPick, mDrop;

static bool busy() { return missionPending || stState != "idle"; }

static String normColour(String s) {
  s.trim(); s.toUpperCase();
  return (s == "RED" || s == "GREEN" || s == "BLUE") ? s : "";
}

// ---------- API for the sketch ----------
bool wifiGetMission(String& pick, String& drop) {
  if (!missionPending) return false;
  missionPending = false;
  pick = mPick; drop = mDrop;
  return true;
}

bool wifiStopRequested() {
  if (!stopPending) return false;
  stopPending = false;
  return true;
}

void wifiSetStatus(const char* state, const String& msg, bool magnetOn) {
  stState = state; stMsg = msg; stMagnet = magnetOn;
}

// ---------- HTTP ----------
struct Reply { int code; String body; };

static String statusJson() {
  String j = "{";
  j += "\"state\":\"" + stState + "\",";
  j += "\"msg\":\"" + stMsg + "\",";
  j += "\"pick\":\"" + stPick + "\",";
  j += "\"drop\":\"" + stDrop + "\",";
  j += "\"magnet\":" + String(stMagnet ? "true" : "false") + ",";
  j += "\"busy\":" + String(busy() ? "true" : "false");
  j += "}";
  return j;
}
static Reply ok()                         { return { 200, statusJson() }; }
static Reply err(int code, const char* m) { return { code, String("{\"error\":\"") + m + "\"}" }; }

static String getArg(const String& query, const char* key) {
  String k = String(key) + "=";
  int from = 0;
  while (from <= (int)query.length()) {
    int amp = query.indexOf('&', from);
    if (amp < 0) amp = query.length();
    String part = query.substring(from, amp);
    if (part.startsWith(k)) return part.substring(k.length());
    from = amp + 1;
  }
  return "";
}

static Reply handleMission(const String& q) {
  if (busy()) return err(409, "Arm is busy - wait or press STOP");
  String p = normColour(getArg(q, "pick")), d = normColour(getArg(q, "drop"));
  if (p == "" || d == "") return err(400, "Choose both colours");
  if (p == d)             return err(400, "Pick and drop colours must differ");
  mPick = p; mDrop = d; missionPending = true;
  stPick = p; stDrop = d; stMsg = "Starting...";
  return ok();
}

static Reply handleStop() {
  stopPending = true;
  missionPending = false;
  stMsg = "Stopping...";
  return ok();
}

static const char* reasonPhrase(int code) {
  switch (code) { case 200: return "OK"; case 400: return "Bad Request";
                  case 404: return "Not Found"; case 409: return "Conflict"; default: return "Error"; }
}
static void writeHeaders(WiFiClient& c, int code, const char* type, size_t len) {
  c.print("HTTP/1.1 "); c.print(code); c.print(" "); c.println(reasonPhrase(code));
  c.print("Content-Type: "); c.println(type);
  c.print("Content-Length: "); c.println(len);
  c.println("Cache-Control: no-store");
  c.println("Connection: close");
  c.println();
}

void wifiLoop() {
  WiFiClient client = server.available();
  if (!client) return;

  // Read "GET /path?query HTTP/1.1", then skip headers.
  String line, requestLine;
  bool haveRequestLine = false;
  unsigned long t0 = millis();
  while (client.connected() && millis() - t0 < 1000) {
    if (!client.available()) continue;
    char ch = client.read();
    if (ch == '\r') continue;
    if (ch == '\n') {
      if (!haveRequestLine) { requestLine = line; haveRequestLine = true; }
      else if (line.length() == 0) break;
      line = "";
    } else if (line.length() < 300) line += ch;
  }

  int s1 = requestLine.indexOf(' ');
  int s2 = requestLine.indexOf(' ', s1 + 1);
  String target = (s1 > 0 && s2 > s1) ? requestLine.substring(s1 + 1, s2) : "/";
  int qm = target.indexOf('?');
  String path  = qm >= 0 ? target.substring(0, qm) : target;
  String query = qm >= 0 ? target.substring(qm + 1) : "";

  if (path == "/") {
    size_t len = strlen(PAGE_HTML);
    writeHeaders(client, 200, "text/html", len);
    for (size_t i = 0; i < len; i += 512) {        // send in chunks
      size_t n = (len - i < 512) ? (len - i) : 512;
      client.write((const uint8_t*)PAGE_HTML + i, n);
    }
  } else {
    Reply r;
    if      (path == "/status")  r = ok();
    else if (path == "/mission") r = handleMission(query);
    else if (path == "/stop")    r = handleStop();
    else                         r = err(404, "Not found");
    writeHeaders(client, r.code, "application/json", r.body.length());
    client.print(r.body);
  }
  client.flush();
  client.stop();
}

void wifiSetup() {
  if (WiFi.status() == WL_NO_MODULE) {
    Serial.println("[wifi] module not responding - run Tools > Firmware Updater");
    return;
  }
  WiFi.config(IPAddress(192, 168, 4, 1));
  if (WiFi.beginAP(AP_SSID, AP_PASS) != WL_AP_LISTENING) {
    Serial.println("[wifi] failed to start hotspot");
    return;
  }
  delay(2000);
  server.begin();
  Serial.print("[wifi] hotspot \""); Serial.print(AP_SSID);
  Serial.print("\" up - open http://"); Serial.println(WiFi.localIP());
}
