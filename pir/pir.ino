#include <WiFi.h>
#include <WebServer.h>

#define PIR_PIN 27
#define LED_PIN 23
#define HOLD_TIME 10000

const char* ap_ssid = "ClassroomMonitor";
const char* ap_password = "monitor1234";

WebServer server(80);

bool motionDetected = false;
bool roomOccupied = false;
unsigned long lastMotionTime = 0;
int motionCount = 0;

// Manual LED control
bool manualMode = false;      // true = dashboard buttons control the LED
bool manualLedState = false;  // LED state requested by the buttons
bool ledOn = false;           // actual LED state (reported to dashboard)

const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1">
    <title>Smart Classroom Occupancy Monitor</title>
    <style>
        body { font-family: Arial; background: #1a1a2e; color: white; text-align: center; padding: 20px; margin: 0; }
        .box { max-width: 450px; margin: auto; background: #16213e; padding: 30px; border-radius: 20px; }
        h1 { color: #00d4ff; font-weight: 300; margin-bottom: 25px; font-size: 1.5em; }
        .card { background: #0f3460; padding: 20px; border-radius: 15px; margin: 15px 0; }
        .label { color: #aaa; font-size: 0.8em; text-transform: uppercase; letter-spacing: 1px; margin-bottom: 8px; }
        .value { font-size: 1.8em; font-weight: bold; }
        .occupied { color: #ff6b6b; }
        .unoccupied { color: #00ff88; }
        .motion-on { color: #ffaa00; }
        .motion-off { color: #888; }
        .led-on { color: #00ff88; }
        .led-off { color: #ff4444; }
        .counter { color: #00d4ff; }
        .last-motion { color: #ffaa00; font-size: 1.2em; }
        .btn { background: #00d4ff; color: #0f3460; border: none; padding: 12px 20px; margin: 5px; border-radius: 10px; font-size: 1em; font-weight: bold; cursor: pointer; }
        .btn:active { transform: scale(0.95); }
        .mode { color: #aaa; font-size: 0.9em; margin-top: 10px; }
    </style>
</head>
<body>
    <div class="box">
        <h1>SMART CLASSROOM OCCUPANCY MONITOR</h1>
        <div class="card">
            <div class="label">Motion Sensor</div>
            <div class="value" id="motionStatus">--</div>
        </div>
        <div class="card">
            <div class="label">Room Status</div>
            <div class="value" id="roomStatus">--</div>
        </div>
        <div class="card">
            <div class="label">Light Status</div>
            <div class="value" id="ledStatus">--</div>
        </div>
        <div class="card">
            <div class="label">Manual Light Control</div>
            <button class="btn" onclick="setLed('on')">ON</button>
            <button class="btn" onclick="setLed('off')">OFF</button>
            <button class="btn" onclick="setLed('auto')">AUTO</button>
            <div class="mode" id="modeStatus">--</div>
        </div>
        <div class="card">
            <div class="label">Last Motion</div>
            <div class="value last-motion" id="lastMotion">--</div>
        </div>
        <div class="card">
            <div class="label">Motion Events Today</div>
            <div class="value counter" id="motionCount">0</div>
        </div>
    </div>
    <script>
        function setLed(state) {
            fetch('/led?state=' + state, { cache: 'no-store' })
                .then(updateData)
                .catch(e => console.log(e));
        }

        function updateData() {
            fetch('/data', { cache: 'no-store' })
                .then(r => r.json())
                .then(d => {
                    const motionEl = document.getElementById('motionStatus');
                    motionEl.textContent = d.motion ? 'MOTION DETECTED' : 'NO MOTION';
                    motionEl.className = 'value ' + (d.motion ? 'motion-on' : 'motion-off');
                    const roomEl = document.getElementById('roomStatus');
                    roomEl.textContent = d.occupied ? 'OCCUPIED' : 'UNOCCUPIED';
                    roomEl.className = 'value ' + (d.occupied ? 'occupied' : 'unoccupied');
                    const ledEl = document.getElementById('ledStatus');
                    ledEl.textContent = d.led ? 'ON' : 'OFF';
                    ledEl.className = 'value ' + (d.led ? 'led-on' : 'led-off');
                    document.getElementById('lastMotion').textContent = d.lastMotion;
                    document.getElementById('motionCount').textContent = d.count;
                    document.getElementById('modeStatus').textContent = d.manual ? 'Mode: MANUAL' : 'Mode: AUTO (PIR)';
                })
                .catch(e => console.log(e));
        }
        setInterval(updateData, 1000);
        updateData();
    </script>
</body>
</html>
)rawliteral";

String getLastMotionText() {
    if (lastMotionTime == 0) return "No motion yet";
    unsigned long elapsed = (millis() - lastMotionTime) / 1000;
    if (elapsed < 60) return String(elapsed) + " seconds ago";
    else return String(elapsed / 60) + " minutes ago";
}

void setup() {
    Serial.begin(115200);
    Serial.println("\nSMART OCCUPANCY SYSTEM STARTING...");

    pinMode(PIR_PIN, INPUT);
    pinMode(LED_PIN, OUTPUT);
    digitalWrite(LED_PIN, LOW);

    Serial.println("Warming up PIR sensor (30 seconds)...");
    delay(30000);
    Serial.println("PIR Ready!");

    WiFi.softAP(ap_ssid, ap_password);
    Serial.print("AP IP Address: ");
    Serial.println(WiFi.softAPIP());

    server.on("/", []() {
        server.send(200, "text/html", index_html);
    });

    server.on("/data", []() {
        String lastMotion = getLastMotionText();
        String json = "{";
        json += "\"motion\":" + String(motionDetected ? "true" : "false") + ",";
        json += "\"occupied\":" + String(roomOccupied ? "true" : "false") + ",";
        json += "\"led\":" + String(ledOn ? "true" : "false") + ",";
        json += "\"manual\":" + String(manualMode ? "true" : "false") + ",";
        json += "\"lastMotion\":\"" + lastMotion + "\",";
        json += "\"count\":" + String(motionCount);
        json += "}";
        server.sendHeader("Cache-Control", "no-store");
        server.send(200, "application/json", json);
    });

    server.on("/led", []() {
        String state = server.arg("state");
        Serial.print("Button pressed: ");
        Serial.println(state);
        if (state == "on") {
            manualMode = true;
            manualLedState = true;
        } else if (state == "off") {
            manualMode = true;
            manualLedState = false;
        } else if (state == "auto") {
            manualMode = false;
        }
        server.sendHeader("Cache-Control", "no-store");
        server.send(200, "text/plain", "OK");
    });

    server.begin();
    Serial.println("Server started!");
    Serial.print("Connect to WiFi: ");
    Serial.println(ap_ssid);
    Serial.print("Password: ");
    Serial.println(ap_password);
    Serial.print("Open browser: http://");
    Serial.println(WiFi.softAPIP());
}

void loop() {
    server.handleClient();

    int motion = digitalRead(PIR_PIN);

    if (motion == HIGH) {
        motionDetected = true;
        if (!roomOccupied) {
            motionCount++;
            Serial.print("NEW MOTION EVENT! Count: ");
            Serial.println(motionCount);
        }
        lastMotionTime = millis();
        roomOccupied = true;
    } else {
        motionDetected = false;
        if (roomOccupied && (millis() - lastMotionTime > HOLD_TIME)) {
            roomOccupied = false;
            Serial.println("Room UNOCCUPIED");
        }
    }

    // The LED is set in ONE place only, so nothing can override the buttons.
    ledOn = manualMode ? manualLedState : roomOccupied;
    digitalWrite(LED_PIN, ledOn ? HIGH : LOW);
}
