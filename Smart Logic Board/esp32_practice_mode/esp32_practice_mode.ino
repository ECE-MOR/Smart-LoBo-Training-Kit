#include <Wire.h>
#include <LiquidCrystal_I2C.h>

#define BUTTON_SELECT 26
#define BUTTON_OPTION 25
#define BUTTON_BACK 33
#define BUZZER_PIN 32  

LiquidCrystal_I2C lcd(0x27, 20, 4); 

enum State {
  MAIN_MENU,
  PRACTICE_MODE,
  IOT_MODE,
  SELECT_DIFFICULTY,
  EASY_QUESTION,
  AVERAGE_QUESTION,
  DIFFICULT_QUESTION,
  QUESTION_OVER,
  TRY_AGAIN
};

State currentState = MAIN_MENU;
int menuSelection = 0;         
int difficultySelection = 0;    
unsigned long currentMillis = millis();
unsigned long previousMillis = 0;
const long interval = 1000; 
int questionTimer = 0;
int maxTime = 15; 


int EasyTimer = 60;
int AverageTimer = 180;
int DifficultTimer = 300;

String questionsEasy[5] = {"A'OR (A + D) = X, if A=0;D=1", "AB' + BC'= X, if A=B=0;C=1", "(A'D) OR (AB)' = X, if A=D=1;B=0", "ABD' + B'D' = X, if A=1;B=D=0", "(C' + D) OR (B'C') = X, if B=0;C=D=1"};
String questionsAverage[5] = {"((BE) + (BE)') OR (C + E)' = X, if B=0;C=E=1", "A'DE + (A' OR (AB)') + DE = X, if A=E=0;B=D=1", "(A'B'D) + ((D + E)' OR (AB)) = X, if A=D=1;B=E=0", "[(AB + C) OR (B'D + C'E)] + (AB + C)' = X, if A=B=D=1;C=E=0", "[(AE' XOR B)'] + [(B XOR C'E) OR (B + (A'D')')] = X IF A=D=0;B=C=E=1"};
String questionsDifficult[5] = {"(C + D)' + A'CD' + AB'C' + A'B'CD + ACD' = X, if A=D=1;B=C=0", "B'D' + D'E + BE + BD + DE' = X, if B=D=1;E=0", "A'CD + AC'D' + A'C'D' + AC'D + ACD = X, if A=D=0;C=1", "AB(C'D)' + A'BD ' B'C'D' + C' = X, if A=1;B=C=D=0", "BC' + B'D'E' + B'C'E + B'C'DE' = X, if B=C=1;D=E=0" };

int answersEasy[5] = {1, 0, 0, 1, 0};
int answersAverage[5] = {0, 1, 0, 1, 1};
String answersDifficult[5] = {
  "D' + A'B'C + AB'C ; OUTPUT=1",
  "B'D' + BE + DE' ; OUTPUT=1",
  "CD + AC' + C'D' ; OUTPUT=0",
  "AB + BD + C' ; OUTPUT=1",
  "C' + B'D'E' ; OUTPUT=0"
};

int currentQuestion = -1;  
int tryAgainSelection = 0; 


byte XNOR[] = {
  B00000,
  B01110,
  B10101,
  B11111,
  B10101,
  B01110,
  B00000,
  B00000
};
byte XOR[] = {
  B00000,
  B01110,
  B10001,
  B10101,
  B10001,
  B01110,
  B00000,
  B00000
};
byte OR[] = { 
  B00000,
  B00000,
  B00100,
  B01110,
  B01110,
  B00100,
  B00000,
  B00000
};

void setup() {  
  lcd.init();
  lcd.backlight();

  lcd.createChar(0, XNOR);
  lcd.createChar(1, XOR);
  lcd.createChar(2, OR);

  pinMode(BUTTON_SELECT, INPUT_PULLUP);
  pinMode(BUTTON_OPTION, INPUT_PULLUP);
  pinMode(BUTTON_BACK, INPUT_PULLUP);
  pinMode(BUZZER_PIN, OUTPUT);

  displayFlowingMessage("Smart Logic Board");
  displayMainMenu();
}

void loop() {
  currentMillis = millis();

  
  if (digitalRead(BUTTON_SELECT) == LOW) {
    handleSelectButton();
    delay(200); 
  }
  if (digitalRead(BUTTON_OPTION) == LOW) {
    handleOptionButton();
    delay(200); 
  }
  if (digitalRead(BUTTON_BACK) == LOW) {
    handleBackButton();
    delay(200); 
  }

  
  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;
    
    switch (currentState) {
      case EASY_QUESTION:
      case AVERAGE_QUESTION:
      case DIFFICULT_QUESTION:
        if (questionTimer > 0) {
          questionTimer--;
          displayCountdown(questionTimer);
        } else {
          currentState = QUESTION_OVER;
          displayTimesUp();
        }
        break;
      
      case QUESTION_OVER:
        delay(5000);  
        displayAnswer();
        delay(10000);  
        currentState = TRY_AGAIN; 
        displayTryAgain();
        break;

      case TRY_AGAIN:
        
        break;

      default:
        break;
    }
  }
}

void displayFlowingMessage(const String &message) {
  lcd.clear();
  for (int i = 0; i < message.length() + 20; i++) {
    lcd.clear();
    lcd.setCursor(20 - i, 0);
    lcd.print(message);
    delay(200);
  }
  lcd.clear();
}

void typewriterEffect(String text, int row, int delayTime) {
  lcd.setCursor(0, row); 
  for (int i = 0; i < text.length(); i++) {
    lcd.print(text[i]); 
    delay(delayTime); 
  }
}

void displayMainMenu() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Choose Mode");
  lcd.setCursor(0, 1);
  lcd.print((menuSelection == 0) ? "> Training Mode" : "  Training Mode");
  lcd.setCursor(0, 2);
  lcd.print((menuSelection == 1) ? "> IoT Mode" : "  IoT Mode");
}

void handleSelectButton() {
  switch (currentState) {
    case MAIN_MENU:
      if (menuSelection == 0) {
        currentState = PRACTICE_MODE;  
        displayPracticeMode();
      } else if (menuSelection == 1) {
        currentState = IOT_MODE;
        lcd.clear();
        lcd.setCursor(0, 0);
        typewriterEffect("TURN TOGGLE SWITCH  FOR IoT MODE :>", 0, 150);
        delay(2000);        
      }
      break;
      
    case PRACTICE_MODE:
      currentState = SELECT_DIFFICULTY;
      displayTrainingOptions();
      break;
      
    case SELECT_DIFFICULTY:
      if (difficultySelection == 0) currentState = EASY_QUESTION;
      else if (difficultySelection == 1) currentState = AVERAGE_QUESTION;
      else if (difficultySelection == 2) currentState = DIFFICULT_QUESTION;
      displayQuestion();
      break;
      
    case TRY_AGAIN:
      if (tryAgainSelection == 0) {
        currentState = SELECT_DIFFICULTY; 
        displayTrainingOptions();
      } else {
        currentState = MAIN_MENU; 
        displayFlowingMessage("Smart Logic Board");
        displayMainMenu();
      }
      break;

    default:
      break;
  }
}

void displayPracticeMode() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Practice Mode");
  lcd.setCursor(0, 1);
  lcd.print("Select Difficulty");
}

void displayTrainingOptions() {
  lcd.clear();
  lcd.setCursor(0, 1);
  lcd.print((difficultySelection == 0) ? "> Easy" : "  Easy");
  lcd.setCursor(0, 2);
  lcd.print((difficultySelection == 1) ? "> Average" : "  Average");
  lcd.setCursor(0, 3);
  lcd.print((difficultySelection == 2) ? "> Difficult" : "  Difficult");
}

void handleOptionButton() {
  if (currentState == MAIN_MENU) {
    menuSelection = (menuSelection + 1) % 2; // Toggle between 0 and 1
    displayMainMenu();
  } else if (currentState == SELECT_DIFFICULTY) {
    difficultySelection = (difficultySelection + 1) % 3; // Cycle through 0, 1, 2
    displayTrainingOptions();
  } else if (currentState == TRY_AGAIN) {
    tryAgainSelection = (tryAgainSelection + 1) % 2; // Toggle between Yes and No
    displayTryAgain();
  }
}

void handleBackButton() {
  if (currentState == EASY_QUESTION || currentState == AVERAGE_QUESTION || currentState == DIFFICULT_QUESTION) {
    currentState = SELECT_DIFFICULTY;
    displayTrainingOptions();
  } else if (currentState == PRACTICE_MODE || currentState == IOT_MODE || currentState == SELECT_DIFFICULTY) {
    currentState = MAIN_MENU;
    displayMainMenu();
  }
}

void displayCountdown(int time) {
  int minutes = time / 60;
  int seconds = time % 60;
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Time Remaining:");
  lcd.setCursor(0, 1);
  lcd.print(minutes);
  lcd.print(":");
  if (seconds < 10) lcd.print("0");
  lcd.print(seconds);
}

void displayTimesUp() {
  digitalWrite(BUZZER_PIN, HIGH); 
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Times Up!");
  delay(5000); 
  digitalWrite(BUZZER_PIN, LOW); 
}

void displayAnswer() {
  lcd.clear();
  lcd.setCursor(0, 0);
  
  
  if (difficultySelection == 0) { 
    lcd.print("Answer: ");
    lcd.print(answersEasy[currentQuestion]);
  } else if (difficultySelection == 1) { 
    lcd.print("Answer: ");
    lcd.print(answersAverage[currentQuestion]);
  } else { // Difficult
    lcd.print("Answer: ");
    lcd.setCursor(0, 1);  
    lcd.print(answersDifficult[currentQuestion]); 
  }
}

void displayTryAgain() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Try Again?");
  
  
  lcd.setCursor(0, 1);
  lcd.print((tryAgainSelection == 0) ? "> Yes" : "  Yes");
  lcd.setCursor(0, 2);
  lcd.print((tryAgainSelection == 1) ? "> No" : "  No");
}

void displayQuestion() {
  lcd.clear();
  
  String question = "";
  if (difficultySelection == 0) {
    lcd.setCursor(4, 1);
    lcd.print("DIRECTIONS:");
    delay(2000);
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Implement and deter-");
    lcd.setCursor(0, 1);
    lcd.print("mine the output (x) ");
    lcd.setCursor(0, 2);
    lcd.print("of the following lo-");
    lcd.setCursor(0, 3);
    lcd.print("gic circuits");
    delay(8000);
    lcd.clear();
    currentQuestion = random(0, 5);  
    question = questionsEasy[currentQuestion];
  } else if (difficultySelection == 1) {
    lcd.setCursor(4, 1);
    lcd.print("DIRECTIONS:");
    delay(2000);
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Implement and deter-");
    lcd.setCursor(0, 1);
    lcd.print("mine the output (x) ");
    lcd.setCursor(0, 2);
    lcd.print("of the following lo-");
    lcd.setCursor(0, 3);
    lcd.print("gic circuits");
    delay(8000);
    lcd.clear();
    currentQuestion = random(0, 5);
    question = questionsAverage[currentQuestion];
  } else {
    lcd.setCursor(4, 1);
    lcd.print("DIRECTIONS:");
    delay(2000);
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Simplify using Bool-");
    lcd.setCursor(0, 1);
    lcd.print("ean algebra techniq-");
    lcd.setCursor(0, 2);
    lcd.print("ues then implement");
    lcd.setCursor(0, 3);
    lcd.print("and determine the");
    delay(8000);
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("output (X) of the");
    lcd.setCursor(0, 1);
    lcd.print("following logic");
    lcd.setCursor(0, 2);
    lcd.print("circuits");
    delay(5000);
    lcd.clear();
    currentQuestion = random(0, 5);
    question = questionsDifficult[currentQuestion];
  }

  int cursorRow = 0;
  int cursorCol = 0;
  for (int i = 0; i < question.length(); i++) {
    if (question[i] == 'X' && i + 3 < question.length() && question.substring(i, i + 4) == "XNOR") {
      lcd.setCursor(cursorCol, cursorRow);
      lcd.write((byte)0); 
      i += 3; 
    } else if (question[i] == 'X' && i + 2 < question.length() && question.substring(i, i + 3) == "XOR") {
      lcd.setCursor(cursorCol, cursorRow);
      lcd.write((byte)1); 
      i += 2; 
    } else if (question[i] == 'O' && i + 1 < question.length() && question.substring(i, i + 2) == "OR") {
      lcd.setCursor(cursorCol, cursorRow);
      lcd.write((byte)2); 
      i += 1; 
    } else {
      lcd.setCursor(cursorCol, cursorRow);
      lcd.print(question[i]);
    }

    cursorCol++;
    if (cursorCol == 20) {
      cursorCol = 0;
      cursorRow++;
    }
  }
  
  delay(25000); 
  if (difficultySelection == 0) {
    questionTimer = EasyTimer; 
  } else if (difficultySelection == 1) {
    questionTimer = AverageTimer; 
  } else {
    questionTimer = DifficultTimer; 
  }
}