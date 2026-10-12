#include <WiFi.h>
#include <WebServer.h>
#include "soc/soc.h"
#include "soc/rtc_cntl_reg.h"

#define IR_PIN 34
#define LED_PIN 23
#define BUZZER_PIN 14

// 0 = buzzer stays on while an object is detected
// 1 = short beep only when an object first appears
#define BEEP_ONLY_ON_NEW 0

const char* ap_ssid = "ESP32_IR";
const char* ap_password = "12345678";

WebServer server(80);

bool objectDetected = false;

const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1">
    <title>IR Sensor Monitor</title>
    <style>
        body { font-family: Arial; background: #0f0f23; color: white; text-align: center; padding: 20px; margin: 0; }
        .box { max-width: 400px; margin: auto; background: #1a1a3e; padding: 30px; border-radius: 20px; }
        h1 { color: #00d4ff; font-weight: 300; }
        .card { background: #252550; padding: 20px; border-radius: 15px; margin: 15px 0; }
        .label { color: #888; font-size: 0.75em; text-transform: uppercase; letter-spacing: 1.5px; }
        .value { font-size: 2em; font-weight: bold; }
        .detected { color: #ff4444; }
        .clear { color: #00ff88; }
        .foot { margin-top: 20px; color: #555; font-size: 0.75em; }
    </style>
</head>
<body>
    <div class="box">
        <h1>IR SENSOR MONITOR</h1>
        <div class="card">
            <div class="label">Status</div>
            <div class="value" id="status">--</div>
        </div>
        <div class="foot">Updates every 500ms</div>
    </div>
    <script>
        function updateData() {
            fetch('/data')
                .then(r => r.json())
                .then(d => {
                    const el = document.getElementById('status');
                    el.textContent = d.detected ? 'OBJECT DETECTED' : 'CLEAR';
                    el.className = 'value ' + (d.detected ? 'detected' : 'clear');
                })
                .catch(e => console.log(e));
        }
        setInterval(updateData, 500);
        updateData();
    </script>
</body>
</html>
)rawliteral";

void printNetworkInfo() {
    Serial.println();
    Serial.println("=== ESP32 ACCESS POINT ===");
    Serial.print("SSID     : ");
    Serial.println(ap_ssid);
    Serial.print("Password : ");
    Serial.println(ap_password);
    Serial.print("Open     : http://");
    Serial.println(WiFi.softAPIP());
    Serial.println("==========================");
}

void setup() {
    WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0);
    Serial.begin(115200);
    delay(1500);  // gives the Serial Monitor time to connect
    Serial.println("IR SENSOR MONITOR STARTING");

    pinMode(IR_PIN, INPUT);
    pinMode(LED_PIN, OUTPUT);
    digitalWrite(LED_PIN, LOW);
    pinMode(BUZZER_PIN, OUTPUT);
    digitalWrite(BUZZER_PIN, LOW);

    WiFi.mode(WIFI_AP);
    bool ok = WiFi.softAP(ap_ssid, ap_password);
    Serial.println(ok ? "AP started" : "AP FAILED to start");
    delay(500);
    printNetworkInfo();

    server.on("/", []() {
        server.send(200, "text/html", index_html);
    });

    server.on("/data", []() {
        String json = "{";
        json += "\"detected\":" + String(objectDetected ? "true" : "false");
        json += "}";
        server.send(200, "application/json", json);
    });

    server.begin();
    Serial.println("Server started!");
}

void loop() {
    server.handleClient();

    static unsigned long lastRead = 0;
    static unsigned long lastInfo = 0;
    static bool lastDetected = false;

    // Reprint the IP every 5 seconds
    if (millis() - lastInfo > 5000) {
        lastInfo = millis();
        printNetworkInfo();
    }

    if (millis() - lastRead > 500) {
        lastRead = millis();
        int irValue = digitalRead(IR_PIN);
        objectDetected = (irValue == LOW);
        digitalWrite(LED_PIN, objectDetected ? HIGH : LOW);

#if BEEP_ONLY_ON_NEW
        if (objectDetected && !lastDetected) {
            digitalWrite(BUZZER_PIN, HIGH);
            delay(150);
            digitalWrite(BUZZER_PIN, LOW);
        }
#else
        digitalWrite(BUZZER_PIN, objectDetected ? HIGH : LOW);
#endif

        lastDetected = objectDetected;
        Serial.println(objectDetected ? "OBJECT DETECTED" : "CLEAR");
    }
}