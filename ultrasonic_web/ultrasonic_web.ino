#include <WiFi.h>
#include <WebServer.h>
#include "soc/soc.h"
#include "soc/rtc_cntl_reg.h"   

#define TRIG_PIN 5
#define ECHO_PIN 18

const char* ap_ssid = "Test";
const char* ap_password = "12345678";

WebServer server(80);

float distance = 0;
String statusText = "CLEAR";

const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1">
    <title>Ultrasonic Distance Monitor</title>
    <style>
        body { font-family: Arial; background: #f0f0f0; text-align: center; padding: 30px; margin: 0; }
        .container { max-width: 400px; margin: auto; background: white; padding: 30px; border: 1px solid #ccc; }
        h1 { font-size: 20px; color: #333; margin-bottom: 30px; }
        .label { font-size: 12px; color: #666; text-transform: uppercase; margin-bottom: 5px; }
        .value { font-size: 48px; font-weight: bold; color: #000; margin-bottom: 25px; }
        .status { font-size: 20px; padding: 10px; border: 1px solid #999; margin-bottom: 20px; }
        .very-near { background: #ffcccc; color: #cc0000; }
        .near { background: #ffffcc; color: #cc8800; }
        .clear { background: #ccffcc; color: #006600; }
        .footer { font-size: 12px; color: #999; margin-top: 20px; }
    </style>
</head>
<body>
    <div class="container">
        <h1>ULTRASONIC DISTANCE MONITOR</h1>
        <div class="label">Current Distance</div>
        <div class="value" id="distance">-- cm</div>
        <div class="label">Status</div>
        <div class="status" id="status">--</div>
        <div class="footer">Updates every 500ms</div>
    </div>
    <script>
        function updateData() {
            fetch('/data')
                .then(r => r.json())
                .then(d => {
                    document.getElementById('distance').textContent = d.distance.toFixed(1) + ' cm';
                    var s = document.getElementById('status');
                    s.textContent = d.status;
                    if (d.status === 'VERY NEAR') s.className = 'status very-near';
                    else if (d.status === 'NEAR') s.className = 'status near';
                    else s.className = 'status clear';
                })
                .catch(e => console.log(e));
        }
        setInterval(updateData, 500);
        updateData();
    </script>
</body>
</html>
)rawliteral";

float measureDistance() {
    digitalWrite(TRIG_PIN, LOW);
    delayMicroseconds(2);
    digitalWrite(TRIG_PIN, HIGH);
    delayMicroseconds(10);
    digitalWrite(TRIG_PIN, LOW);
    long duration = pulseIn(ECHO_PIN, HIGH, 30000);
    if (duration == 0) return 999;
    return duration * 0.0343 / 2;
}

void setup() {
    WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0);
    Serial.begin(115200);
    Serial.println("ULTRASONIC DISTANCE MONITOR");

    pinMode(TRIG_PIN, OUTPUT);
    pinMode(ECHO_PIN, INPUT);

    WiFi.softAP(ap_ssid, ap_password);
    Serial.print("AP IP: ");
    Serial.println(WiFi.softAPIP());

    server.on("/", []() {
        server.send(200, "text/html", index_html);
    });

    server.on("/data", []() {
        String json = "{";
        json += "\"distance\":" + String(distance) + ",";
        json += "\"status\":\"" + statusText + "\"";
        json += "}";
        server.send(200, "application/json", json);
    });

    server.begin();
    Serial.println("Server started!");
}

void loop() {
    server.handleClient();

    static unsigned long lastMeasure = 0;
    if (millis() - lastMeasure > 500) {
        lastMeasure = millis();
        distance = measureDistance();

        if (distance <= 10) statusText = "VERY NEAR";
        else if (distance <= 30) statusText = "NEAR";
        else statusText = "CLEAR";

        Serial.print("Distance: ");
        Serial.print(distance, 1);
        Serial.print(" cm | ");
        Serial.println(statusText);
    }
}