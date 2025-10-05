#include<ESP8266WiFi.h>
#include<WiFiClient.h>
#include<ESP8266WebServer.h>
#include<WebSocketsServer.h>
#include<DHT.h>
#include<Hash.h>

const char* ssid = "TIAwifi";
const char* password = "#tia@12345";

#define DHTPIN 2
#define DHTTYPE DHT11

DHT dht(DHTPIN, DHTTYPE);

String webPage = "<!DOCTYPE html>"
"<html>"
"<head>"
"<style>"
    "body{background-color: #28a745;font-family: Arial, sans-serif;color:white;text-align: center;}"
    "h1{font-size: 2em;font-weight: bold;}"
    ".databox {font-size: 1.5em;font-weight: bold;margin-top: 20px;padding: 10px;border: 2px solid white;display: inline-block; width: 200px;height: 100px;background-color: #3e8e41;border-radius: 5px;}"
"</style>" 
"</head>"
"<body>"
    "<h1>Temperature and Humidity Data</h1>"
    "<div class = 'databox' id = 'tempbox'>Temperature : --C</div>"
    "<div class = 'databox' id = 'humbox'>Humidity : --%</div>"
"<script>"
    "var Socket;"
    "function start() {"
        "Socket = new WebSocket('ws://' + window.location.hostname + ':81/');"
        "Socket.onmessage = function(evt) {"
            "var data = evt.data.split(',');"
            "document.getElementById('tempbox').innerText = 'Temperature: ' +  data[0] + 'C';"
            "document.getElementById('humbox').innerText = 'Humidity: ' + data[1] + '%';"
        "}"
    "}"
    "window.onload = start;"
"</script>"
"</body>"
"</html>";

WebSocketsServer webSocket = WebSocketsServer(81);
ESP8266WebServer server(80);

void setup() {
  Serial.begin(115200);
  WiFi.begin(ssid, password);
  Serial.println("");

  while(WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());

  server.on("/", [](){
    server.send(200, "text/html", webPage);
  });

  server.begin();

  webSocket.begin();
  webSocket.onEvent(webSocketEvent);

  dht.begin();
}

void loop() {
  webSocket.loop();
  server.handleClient();

  float h = dht.readHumidity();
  float t = dht.readTemperature();

  if(isnan(h) || isnan(t)) {
    Serial.println("Failed to read from DHT sensor!");
    return;
  }

  String sensorData = String(t) + "," + String(h);

  webSocket.broadcastTXT(sensorData);

  Serial.println("Temperature: " + String(t) + "C" + "," + "Humidity: " + String(h) + "%");
  delay(2000);
}

void webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t length) {
  if(type == WStype_TEXT) {
    for(int i=0; i < length; i++) {
      Serial.print((char)payload[i]);
    }
    Serial.println(); 
  }
}