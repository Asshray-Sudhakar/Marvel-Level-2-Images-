#include <WiFi.h>
#include <WebServer.h>
#include <Servo.h>

// ===== WiFi Credentials =====
const char* ssid = "ESP32_Car";
const char* password = "12345678";

// ===== Motor Driver Pins =====
#define IN1 0
#define IN2 32
#define IN3 33
#define IN4 14
#define IN5 27
#define IN6 19
#define IN7 18

// ===== IR Sensors =====
#define IR_LEFT 4
#define IR_RIGHT 2

// ===== Ultrasonic =====
#define TRIG 23
#define ECHO 22

// ===== Servo =====
#define SERVO_PIN 21
Servo servo;

// ===== LDR & LED =====
#define LDR_LEFT 34
#define LDR_RIGHT 35
#define LED_LEFT 25
#define LED_RIGHT 26
#define LDR_THRESHOLD 1000

// ===== Variables =====
WebServer server(80);
int mode = 0; // 0 - Line Follower, 1 - Obstacle Avoider, 2 - Manual

// ===== Movement Functions =====
void stopCar() {
  digitalWrite(IN1, LOW); digitalWrite(IN2, LOW);
  digitalWrite(IN3, LOW); digitalWrite(IN4, LOW);
  digitalWrite(IN5, LOW); digitalWrite(IN6, LOW);
  digitalWrite(IN7, LOW);
}

void forward() {
  digitalWrite(IN1, HIGH); digitalWrite(IN2, LOW);
  digitalWrite(IN3, HIGH); digitalWrite(IN4, LOW);
  digitalWrite(IN5, HIGH); digitalWrite(IN6, LOW);
  digitalWrite(IN7, HIGH);
}

void backward() {
  digitalWrite(IN1, LOW); digitalWrite(IN2, HIGH);
  digitalWrite(IN3, LOW); digitalWrite(IN4, HIGH);
  digitalWrite(IN5, LOW); digitalWrite(IN6, HIGH);
  digitalWrite(IN7, LOW);
}

void left() {
  digitalWrite(IN1, LOW); digitalWrite(IN2, HIGH);
  digitalWrite(IN3, HIGH); digitalWrite(IN4, LOW);
  digitalWrite(IN5, LOW); digitalWrite(IN6, HIGH);
  digitalWrite(IN7, HIGH);
}

void right() {
  digitalWrite(IN1, HIGH); digitalWrite(IN2, LOW);
  digitalWrite(IN3, LOW); digitalWrite(IN4, HIGH);
  digitalWrite(IN5, HIGH); digitalWrite(IN6, LOW);
  digitalWrite(IN7, LOW);
}

// ===== Distance Measurement =====
long getDistance() {
  digitalWrite(TRIG, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG, LOW);
  long duration = pulseIn(ECHO, HIGH);
  return duration * 0.034 / 2;
}

// ===== Auto Headlight Function =====
void checkHeadlight() {
  int ldrLeft = analogRead(LDR_LEFT);
  int ldrRight = analogRead(LDR_RIGHT);
  if (ldrLeft < LDR_THRESHOLD || ldrRight < LDR_THRESHOLD) {
    digitalWrite(LED_LEFT, HIGH);
    digitalWrite(LED_RIGHT, HIGH);
  } else {
    digitalWrite(LED_LEFT, LOW);
    digitalWrite(LED_RIGHT, LOW);
  }
}

// ===== Web Server Control =====
void handleRoot() {
  String html = "<h1>ESP32 Car Control</h1><br>";
  html += "<a href=\"/forward\">Forward</a><br>";
  html += "<a href=\"/backward\">Backward</a><br>";
  html += "<a href=\"/left\">Left</a><br>";
  html += "<a href=\"/right\">Right</a><br>";
  html += "<a href=\"/stop\">Stop</a><br>";
  html += "<a href=\"/mode0\">Mode 0 - Line Follower</a><br>";
  html += "<a href=\"/mode1\">Mode 1 - Obstacle Avoider</a><br>";
  html += "<a href=\"/mode2\">Mode 2 - Manual</a><br>";
  server.send(200, "text/html", html);
}

void handleForward() { forward(); server.send(200, "text/plain", "Forward"); }
void handleBackward() { backward(); server.send(200, "text/plain", "Backward"); }
void handleLeft() { left(); server.send(200, "text/plain", "Left"); }
void handleRight() { right(); server.send(200, "text/plain", "Right"); }
void handleStop() { stopCar(); server.send(200, "text/plain", "Stop"); }
void handleMode0() { mode = 0; server.send(200, "text/plain", "Mode 0 - Line Follower"); }
void handleMode1() { mode = 1; server.send(200, "text/plain", "Mode 1 - Obstacle Avoider"); }
void handleMode2() { mode = 2; server.send(200, "text/plain", "Mode 2 - Manual"); }

// ===== Setup =====
void setup() {
  Serial.begin(115200);

  pinMode(IN1, OUTPUT); pinMode(IN2, OUTPUT);
  pinMode(IN3, OUTPUT); pinMode(IN4, OUTPUT);
  pinMode(IN5, OUTPUT); pinMode(IN6, OUTPUT);
  pinMode(IN7, OUTPUT);

  pinMode(IR_LEFT, INPUT);
  pinMode(IR_RIGHT, INPUT);
  pinMode(TRIG, OUTPUT);
  pinMode(ECHO, INPUT);

  pinMode(LDR_LEFT, INPUT);
  pinMode(LDR_RIGHT, INPUT);
  pinMode(LED_LEFT, OUTPUT);
  pinMode(LED_RIGHT, OUTPUT);

  servo.attach(SERVO_PIN);

  WiFi.softAP(ssid, password);
  Serial.println(WiFi.softAPIP());

  server.on("/", handleRoot);
  server.on("/forward", handleForward);
  server.on("/backward", handleBackward);
  server.on("/left", handleLeft);
  server.on("/right", handleRight);
  server.on("/stop", handleStop);
  server.on("/mode0", handleMode0);
  server.on("/mode1", handleMode1);
  server.on("/mode2", handleMode2);
  server.begin();
}

// ===== Loop =====
void loop() {
  server.handleClient();
  checkHeadlight(); // Always check headlight in all modes

  if (mode == 0) {
    // Line Follower Mode
    int leftIR = digitalRead(IR_LEFT);
    int rightIR = digitalRead(IR_RIGHT);

    if (leftIR == LOW && rightIR == LOW) forward();
    else if (leftIR == LOW && rightIR == HIGH) left();
    else if (leftIR == HIGH && rightIR == LOW) right();
    else stopCar();
  }

  else if (mode == 1) {
    // Obstacle Avoider Mode
    servo.write(90);
    long frontDist = getDistance();
    if (frontDist < 15) {
      stopCar();
      delay(500);

      servo.write(30);
      delay(500);
      long leftDist = getDistance();

      servo.write(150);
      delay(500);
      long rightDist = getDistance();

      servo.write(90);
      delay(300);

      if (leftDist > rightDist) left();
      else right();
      delay(500);
    } else {
      forward();
    }
  }

  else if (mode == 2) {
    // Manual Mode - do nothing, only server commands
  }
}
