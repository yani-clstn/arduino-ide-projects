#include <WiFi.h>
#include <WebServer.h>
#include <DHT.h>


const char* ssid = "narzo 50 Pro 5G";
const char* password = "sniggers";

#define DHTPIN 4
#define DHTTYPE DHT11
#define LEDPIN 5

DHT dht(DHTPIN, DHTTYPE);
WebServer server(80);
bool ledState = false;


const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1">
    <title>ESP32</title>
    <style>
        body { font-family: Arial; background: #1a1a2e; color: white; text-align: center; padding: 20px; }
        .box { max-width: 350px; margin: auto; background: #16213e; padding: 25px; border-radius: 15px; }
        h2 { color: #00d4ff; }
        .val { font-size: 2em; margin: 5px 0; }
        .lbl { color: #888; font-size: 0.8em; }
        .status { margin: 15px 0; padding: 10px; background: #0f3460; border-radius: 8px; }
        .on { color: #00ff88; }
        .off { color: #ff4444; }
        .btn { display: inline-block; padding: 12px 25px; margin: 5px; border-radius: 25px; font-size: 1.1em; font-weight: bold; text-decoration: none; color: white; width: 100px; }
        .btn-on { background: #00b894; }
        .btn-off { background: #e17055; }
        .btn-on:hover { background: #00a381; }
        .btn-off:hover { background: #d63031; }
        .foot { margin-top: 15px; color: #555; font-size: 0.7em; }
    </style>
</head>
<body>
    <div class="box">
        <h2>ESP32 Monitor</h2>
        <div class="lbl">Temp</div>
        <div class="val"><span id="temp">--</span> °C</div>
        <div class="lbl">Humidity</div>
        <div class="val"><span id="hum">--</span> %</div>
        <div class="status">LED: <span id="ledStatus" class="off">OFF</span></div>
        <div>
            <a href="/led?state=on" class="btn btn-on">ON</a>
            <a href="/led?state=off" class="btn btn-off">OFF</a>
        </div>
        <div class="foot">Updates every 3s</div>
    </div>
    <script>
        function updateData() {
            fetch('/data')
                .then(r => r.json())
                .then(d => {
                    document.getElementById('temp').textContent = d.temperature.toFixed(1);
                    document.getElementById('hum').textContent = d.humidity.toFixed(1);
                })
                .catch(e => console.log(e));
        }
        function updateLED() {
            fetch('/status')
                .then(r => r.text())
                .then(d => {
                    const el = document.getElementById('ledStatus');
                    if (d === 'ON') {
                        el.textContent = 'ON';
                        el.className = 'on';
                    } else {
                        el.textContent = 'OFF';
                        el.className = 'off';
                    }
                })
                .catch(e => console.log(e));
        }
        setInterval(() => { updateData(); updateLED(); }, 3000);
        updateData();
        updateLED();
    </script>
</body>
</html>
)rawliteral";

void setup() {
    Serial.begin(115200);
    Serial.println("  ESP32 STARTING...");

    dht.begin();
    pinMode(LEDPIN, OUTPUT);
    digitalWrite(LEDPIN, LOW);

    for (int i = 0; i < 3; i++) {
        digitalWrite(LEDPIN, HIGH);
        delay(200);
        digitalWrite(LEDPIN, LOW);
        delay(200);
    }
    Serial.println("LED test done");
    Serial.print("Connecting to WiFi: ");
    Serial.println(ssid);

    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, password);

    int attempts = 0;
    while (WiFi.status() != WL_CONNECTED && attempts < 30) {
        delay(500);
        Serial.print(".");
        attempts++;
        if (attempts % 10 == 0) {
            Serial.println();
            Serial.print("   Still trying... (");
            Serial.print(attempts);
            Serial.println(" seconds)");
        }
    }
    Serial.println();

    if (WiFi.status() == WL_CONNECTED) {
        Serial.println("WiFi Connected!");
        Serial.print("IP Address: ");
        Serial.println(WiFi.localIP());
    } else {
        Serial.println("WiFi FAILED!");
        Serial.println("\n Please check your hotspot settings and restart.");
        while (1) {
            delay(1000);
            Serial.print(".");
        }
    }


    server.on("/", []() {
        server.send(200, "text/html", index_html);
    });

    server.on("/data", []() {
        float t = dht.readTemperature();
        float h = dht.readHumidity();
        if (isnan(t) || isnan(h)) {
            server.send(500, "application/json", "{\"error\":1}");
            return;
        }
        String json = "{\"temperature\":" + String(t) + ",\"humidity\":" + String(h) + "}";
        server.send(200, "application/json", json);
    });

    server.on("/led", []() {
        if (server.hasArg("state")) {
            String state = server.arg("state");
            if (state == "on") {
                digitalWrite(LEDPIN, HIGH);
                ledState = true;
                Serial.println("LED ON");
            } else if (state == "off") {
                digitalWrite(LEDPIN, LOW);
                ledState = false;
                Serial.println("LED OFF");
            }
        }
        server.sendHeader("Location", "/");
        server.send(303);
    });

    server.on("/status", []() {
        server.send(200, "text/plain", ledState ? "ON" : "OFF");
    });

    server.begin();
    Serial.println("Server started!");
    Serial.print("Open browser at: http://");
    Serial.println(WiFi.localIP());
    Serial.println("========================================\n");
}

void loop() {
    server.handleClient();
    delay(10);
}