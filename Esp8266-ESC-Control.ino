// Tucker Shannon 2023

#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>

#define PWM_PIN D4
#define PWM_FREQUENCY 50
#define PWM_RESOLUTION 10
#define MIN_PWM 220
#define MAX_PWM 300

// Wi-Fi credentials
const char* ssid = "ssid";
const char* password = "password";

ESP8266WebServer server(80);  // Create a webserver object that listens for HTTP request on port 80

// Variable to hold the current PWM speed
volatile int currentPWM = MIN_PWM;

// Variable to hold the console text
String consoleText = "";

void setup() {
  // Start Serial communication
  Serial.begin(9600);
  while (!Serial); // Wait for serial to initialize

  // Connect to Wi-Fi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi!");

  // Setup the PWM
  analogWriteRange((1 << PWM_RESOLUTION) - 1); // Set the PWM range
  analogWriteFreq(PWM_FREQUENCY); // Set the PWM frequency

  // Set up the webpage
  server.on("/", HTTP_GET, [](){
    String html = "<html><body>";
    html += "<h1>ESC Control</h1>";
    html += "<button onclick=\"location.href='/arm'\">Arm ESC</button><br>";
    html += "<button onclick=\"location.href='/speed_change'\">Change Speed</button><br>";
    html += "Current PWM: <span id='pwm'>" + String(currentPWM) + "</span><br>";
    html += "<textarea id='console' style='width:500px;height:300px;'>" + consoleText + "</textarea>";
    html += "<script>setInterval(function(){fetch('/speed').then(response => response.text()).then(text => document.getElementById('pwm').innerText = text);}, 1000);</script>";
    html += "</body></html>";
    server.send(200, "text/html", html);
  });

  // Set up the arming sequence route
  server.on("/arm", HTTP_GET, [](){
    printToConsole("Arming the ESC...");
    analogWrite(PWM_PIN, MAX_PWM); // Send maximum throttle signal
    delay(200); // Wait 200ms
    analogWrite(PWM_PIN, MIN_PWM); // Send minimum throttle signal
    delay(200); // Wait 200ms
    printToConsole("ESC arming complete!");
    server.sendHeader("Location", "/");
    server.send(303);
  });

  // Set up the speed change sequence route
  server.on("/speed_change", HTTP_GET, [](){
    printToConsole("Changing motor speed...");
    for (int pwm = MIN_PWM; pwm <= MAX_PWM; pwm++) {
      currentPWM = pwm;
      printToConsole("Increasing speed... " + String(currentPWM));
      analogWrite(PWM_PIN, pwm);
      delay(20);
    }
    for (int pwm = MAX_PWM; pwm >= MIN_PWM; pwm--) {
      currentPWM = pwm;
      printToConsole("Decreasing speed... " + String(currentPWM));
      analogWrite(PWM_PIN, pwm);
      delay(20);
    }
    printToConsole("Motor speed change sequence complete!");
    server.sendHeader("Location", "/");
    server.send(303);
  });

  // Set up the current speed route
  server.on("/speed", HTTP_GET, [](){
    server.send(200, "text/plain", String(currentPWM));
  });

  // Start the server
  server.begin();
}

void loop() {
  server.handleClient();
}

void printToConsole(String text) {
  Serial.println(text);
  consoleText += text + "\n";
}
