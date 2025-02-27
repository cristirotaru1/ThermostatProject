#include <WiFi.h>
#include <WebServer.h>

// default ip: 192.168.4.1
const char* ssid = "Rotaru's Thermostat";
const char* password = "2001thermo";

WebServer server(80);

// Variables
float temperature = 0.0;
float threshold = 25.0;
bool inputThreshold = false;
bool serialThreshold = false;
char heating[4];

// HTML Page
String createHTML() {
  String html = "<html><body>";
  html += "<meta http-equiv='refresh' content='5'>"; // Refresh every 5 seconds
  html += "<h1>Temperature Control</h1>";
  html += "<p>Current Temperature: " + String(temperature) + " °C</p>";
  html += "<p>Heating: " + String(heating) + " </p>";
  html += "<p>Threshold: " + String(threshold) + " °C</p>";
  html += "<button onclick=\"location.href='/increase'\">Increase Threshold</button>";
  html += "<button onclick=\"location.href='/decrease'\">Decrease Threshold</button>";
  html += "</body></html>";
  return html;
}

void handleRoot() {
  server.send(200, "text/html", createHTML());
}

void handleIncrease() {
  threshold += 0.5;
  inputThreshold = true;

  server.sendHeader("Location", "/", true);  // HTTP redirection
  server.send(302, "text/plain", "Redirecting to /");  // 302 Found response
}

void handleDecrease() {
  threshold -= 0.5;
  inputThreshold = true;

  server.sendHeader("Location", "/", true);  // HTTP redirection
  server.send(302, "text/plain", "Redirecting to /");  // 302 Found response
}

void setup() {
  Serial.begin(9600);
  if (WiFi.softAP(ssid, password)) {
      Serial.println("Access Point started!");
      Serial.print("IP Address: ");
      Serial.println(WiFi.softAPIP());
  } else {
      Serial.println("Failed to start Access Point!");
      while (1); // Halt execution
  }

  server.on("/", handleRoot);
  server.on("/increase", handleIncrease);
  server.on("/decrease", handleDecrease);
  server.begin();
}

void loop() {
  // Handle incoming client requests
  server.handleClient();
  if (inputThreshold) {
    Serial.print("SET_THRESHOLD:");
    // Serial.println(String(threshold));
    Serial.println(threshold, 1);
    inputThreshold = false;
  }

  delay(1000);

}

void serialEvent() {
  // Read data from Arduino
  if (Serial.available()) {
    String data = Serial.readStringUntil('\n');
    parseData(data);
  }
}

void parseData(String data) {
  // Variables to hold parsed values
  float temp;
  String heat;
  float thresh;

  // Parse the temperature
  int tempStart = data.indexOf("TEMP:") + 5;
  int tempEnd = data.indexOf(",", tempStart);
  temp = data.substring(tempStart, tempEnd).toFloat();

  // Parse the heating status
  int heatStart = data.indexOf("HEATING:") + 8;
  int heatEnd = data.indexOf(",", heatStart);
  heat = data.substring(heatStart, heatEnd);

  // Parse the threshold
  int threshStart = data.indexOf("THRESH:") + 7;
  thresh = data.substring(threshStart).toFloat();

  // Round temperature and threshold to one decimal
  temp = round(temp * 10.0) / 10.0;
  thresh = round(thresh * 10.0) / 10.0;

  // Update global variables
  temperature = temp;
  heating[0] = '\0'; // Reset heating string
  heat.toCharArray(heating, sizeof(heating)); // Convert String to char array
  threshold = thresh;
}
