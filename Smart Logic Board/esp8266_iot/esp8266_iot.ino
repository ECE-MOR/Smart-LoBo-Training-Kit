#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

const char* ssid = "Smart Logic Board";
const char* password = "ECELOBOKIT";

const int buzzerPin = 12;
ESP8266WebServer server(80);
LiquidCrystal_I2C lcd(0x27, 20, 4);

int numQuestions = 0;
String problems[10];
int timerDurations[10];
String answers[10];

int currentQuestion = 0;
unsigned long startTime = 0;
unsigned long questionStartTime = 0;
bool displayProblem = false;
bool displayPreCountdown = false;
bool displayCountdown = false;
bool displayTimesUp = false;
bool displayAnswer = false;

void handleRoot() {
  String html = "<html><head><title>Smart LoBo Training Kit</title><style>"
                "body { background-color: cyan; display: flex; flex-direction: column; justify-content: center; align-items: center; height: 100vh; margin: 0; text-align: center; font-family: Arial, sans-serif; }"
                "h1 { margin-top: 20px; margin-bottom: 10px; color: darkblue; font-weight: bold; font-size: 48px; }" 
                "h2 { margin-bottom: 20px; color: darkblue; font-size: 24px; }"
                "form { display: flex; flex-direction: column; align-items: center; }"
                "label { font-size: 24px; margin: 10px 0; color: darkblue; }" 
                "input[type='number'], input[type='text'] { padding: 15px; width: 300px; font-size: 20px; margin: 10px 0; border-radius: 5px; border: 1px solid #ccc; }" 
                "input[type='submit'] { padding: 15px 20px; font-size: 20px; cursor: pointer; margin-top: 10px; border: none; border-radius: 5px; background-color: #4CAF50; color: white; }"
                "input[type='submit']:hover { background-color: #45a049; }"
                "</style></head><body>";
  html += "<h2>WELCOME TO</h2>";
  html += "<h1>Smart LoBo Training Kit</h1>";
  html += "<h2>(IoT-MODE)</h2>";
  html += "<form action=\"/setquestions\" method=\"POST\">";
  html += "<label for=\"numQuestions\">Set Number of Questions (1-10):</label>";
  html += "<input type=\"number\" id=\"numQuestions\" name=\"numQuestions\" min=\"1\" max=\"10\" required>";
  html += "<input type=\"submit\" value=\"Submit\">";
  html += "</form></body></html>";
  server.send(200, "text/html", html);
}

void handleSetQuestions() {
  if (server.hasArg("numQuestions")) {
    numQuestions = server.arg("numQuestions").toInt();

    String html = "<html><head><title>Smart LoBo Training Kit</title><style>"
                  "body { background-color: cyan; display: flex; flex-direction: column; justify-content: center; align-items: center; height: 100vh; margin: 0; text-align: center; font-family: Arial, sans-serif; }"
                  "h1 { margin-top: 50px; margin-bottom: 20px; color: darkblue; font-weight: bold; font-size: 48px; }" 
                  "h2 { color: darkblue; font-weight: bold; font-size: 25px; }"
                  "form { display: flex; flex-direction: column; align-items: center; }"
                  "label { font-size: 24px; margin: 10px 0; color: darkblue; }" 
                  "input[type='number'], input[type='text'] { padding: 15px; width: 300px; font-size: 20px; margin: 10px 0; border-radius: 5px; border: 1px solid #ccc; }" 
                  "input[type='submit'] { padding: 15px 20px; font-size: 20px; cursor: pointer; margin-top: 10px; border: none; border-radius: 5px; background-color: #4CAF50; color: white; }"
                  "input[type='submit']:hover { background-color: #45a049; }"
                  "</style></head><body>";
    html += "<h1>Smart LoBo Training Kit</h1>";
    html += "<h2>Enter Problems, Timers, and Answers</h2>";
    html += "<form action=\"/update\" method=\"POST\">";
    
    for (int i = 0; i < numQuestions; i++) {
      html += "<label for=\"problem" + String(i) + "\">Problem " + String(i + 1) + ":</label>";
      html += "<input type=\"text\" id=\"problem" + String(i) + "\" name=\"problem" + String(i) + "\" required>";
      html += "<label for=\"timer" + String(i) + "\">Timer (seconds):</label>";
      html += "<input type=\"number\" id=\"timer" + String(i) + "\" name=\"timer" + String(i) + "\" min=\"1\" required>";
      html += "<label for=\"answer" + String(i) + "\">Answer:</label>";
      html += "<input type=\"text\" id=\"answer" + String(i) + "\" name=\"answer" + String(i) + "\" required><br>";
    }

    html += "<input type=\"submit\" value=\"Submit\">";
    html += "</form></body></html>";
    server.send(200, "text/html", html);
  }
}

void handleUpdate() {
  for (int i = 0; i < numQuestions; i++) {
    problems[i] = server.arg("problem" + String(i));
    timerDurations[i] = server.arg("timer" + String(i)).toInt();
    answers[i] = server.arg("answer" + String(i));
  }

  Serial.println("All questions loaded. Starting the sequence...");

  currentQuestion = 0;
  startQuestion();

  String html = "<html><head><style>"
                "body { background-color: cyan; display: flex; flex-direction: column; justify-content: center; align-items: center; height: 100vh; margin: 0; text-align: center; font-family: Arial, sans-serif; }"
                "h1 { margin-bottom: 20px; color: darkblue; font-weight: bold; font-size: 48px; }" 
                "h2 { margin-bottom: 20px; color: darkblue; font-size: 24px; }" 
                "input[type='submit'] { padding: 15px 20px; font-size: 20px; cursor: pointer; margin-top: 10px; border: none; border-radius: 5px; background-color: #4CAF50; color: white; }"
                "input[type='submit']:hover { background-color: #45a049; }"
                "</style></head><body>";
  html += "<h1>Smart LoBo Training Kit</h1>";
  html += "<h2>The questions have been loaded, please check the display for the sequence.</h2>";
  html += "<form action=\"/reset\" method=\"POST\"><input type=\"submit\" value=\"Want to try again?\"></form>";
  html += "</body></html>";
  server.send(200, "text/html", html);
}

void handleReset() {
  numQuestions = 0;
  currentQuestion = 0;
  displayProblem = false;
  displayPreCountdown = false;
  displayCountdown = false;
  displayTimesUp = false;
  displayAnswer = false;
  startTime = 0;

  handleRoot();
}

void startQuestion() {
  lcd.clear();
  Serial.println("Problem " + String(currentQuestion + 1) + ": " + problems[currentQuestion]);
  lcd.setCursor(0, 0);
  lcd.print("Problem " + String(currentQuestion + 1) + ": " + problems[currentQuestion]);
  delay(10000);
  displayProblem = true;
  questionStartTime = millis(); 
  
}

void setup() {
  Serial.begin(9600);
  lcd.init();
  lcd.backlight();

  pinMode(buzzerPin, OUTPUT);

  WiFi.softAP(ssid, password);
  Serial.println("Access Point started");
  Serial.print("IP Address: ");
  lcd.setCursor(1, 1);
  lcd.print("Smart Logic Board");
  lcd.setCursor(3, 2);
  lcd.print("Is Now Online");

  server.on("/", HTTP_GET, handleRoot);
  server.on("/setquestions", HTTP_POST, handleSetQuestions);
  server.on("/update", HTTP_POST, handleUpdate);
  server.on("/reset", HTTP_POST, handleReset);
  server.begin();
}

void loop() {
  server.handleClient();

  if (displayProblem) {
    unsigned long elapsed = millis() - questionStartTime;
    int timeRemaining = timerDurations[currentQuestion] - (elapsed / 1000); 

    if (timeRemaining > 0) {
      lcd.setCursor(0, 3);
      lcd.print("Time left: " + String(timeRemaining) + ".");
    } else {
      displayProblem = false;
      displayTimesUp = true;
      startTime = millis(); 

      tone(buzzerPin, 1000);  
      delay(1000);              
      noTone(buzzerPin);
    }
  }

  if (displayTimesUp && millis() - startTime >= 5000) {
    displayTimesUp = false;
    displayAnswer = true;
    startTime = millis();
  }

  if (displayAnswer) {
    lcd.setCursor(0, 3);
    lcd.print("The Answer is: " + answers[currentQuestion]);
    delay(100); 
  }

  if (displayAnswer && millis() - startTime >= 5000) {
    displayAnswer = false;
    currentQuestion++;
    if (currentQuestion < numQuestions) {
      startQuestion();
    } else {
      lcd.clear();
      lcd.setCursor(1, 1);
      lcd.print("Training Completed!");
      lcd.setCursor(3, 2);
      lcd.print("End of Session.");
      delay(5000);
      lcd.clear();
      lcd.setCursor(1, 1);
      lcd.print("Smart Logic Board");
      lcd.setCursor(1, 2);
      lcd.print("Input Some Problem");
    }
  }
}
