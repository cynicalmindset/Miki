#include <WiFi.h>
#include <WebServer.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_SDA 13
#define OLED_SCL 2

int ref_eye_height = 40;
int ref_eye_width = 40;
int ref_space_between_eye = 10;
int ref_corner_radius = 10;
int left_eye_height = ref_eye_height;
int left_eye_width = ref_eye_width;
int left_eye_x = 32;
int left_eye_y = 32;
int right_eye_x = 32 + ref_eye_width + ref_space_between_eye;
int right_eye_y = 32;
int right_eye_height = ref_eye_height;
int right_eye_width = ref_eye_width;

unsigned long lastBlinkTime = 0;
const unsigned long blinkInterval = 5000; // blink every 5 seconds

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);
WebServer server(80);

const char* ssid     = "14685";
const char* password = "yash14685";

void draw_eyes(bool update = true) {
  display.clearDisplay();
  int x = int(left_eye_x - left_eye_width / 2);
  int y = int(left_eye_y - left_eye_height / 2);
  display.fillRoundRect(x, y, left_eye_width, left_eye_height, ref_corner_radius, SSD1306_WHITE);
  x = int(right_eye_x - right_eye_width / 2);
  y = int(right_eye_y - right_eye_height / 2);
  display.fillRoundRect(x, y, right_eye_width, right_eye_height, ref_corner_radius, SSD1306_WHITE);
  if (update) {
    display.display();
  }
}

void center_eyes(bool update = true) {
  left_eye_height = ref_eye_height;
  left_eye_width = ref_eye_width;
  right_eye_height = ref_eye_height;
  right_eye_width = ref_eye_width;

  left_eye_x = SCREEN_WIDTH / 2 - ref_eye_width / 2 - ref_space_between_eye / 2;
  left_eye_y = SCREEN_HEIGHT / 2;
  right_eye_x = SCREEN_WIDTH / 2 + ref_eye_width / 2 + ref_space_between_eye / 2;
  right_eye_y = SCREEN_HEIGHT / 2;

  draw_eyes(update);
}

void blink(int speed = 12) {
  draw_eyes();
  for (int i = 0; i < 3; i++) {
    left_eye_height -= speed;
    right_eye_height -= speed;
    draw_eyes();
    delay(1);
  }
  for (int i = 0; i < 3; i++) {
    left_eye_height += speed;
    right_eye_height += speed;
    draw_eyes();
    delay(1);
  }
}

void sleepEyes() {
  left_eye_height = 2;
  right_eye_height = 2;
  draw_eyes(true);
}

void wakeup() {
  sleepEyes();
  for (int h = 0; h <= ref_eye_height; h += 2) {
    left_eye_height = h;
    right_eye_height = h;
    draw_eyes(true);
  }
}

void happy_eye() {
  center_eyes(false);
  int offset = ref_eye_height / 2;
  for (int i = 0; i < 10; i++) {
    display.fillTriangle(left_eye_x - left_eye_width / 2 - 1, left_eye_y + offset,
                          left_eye_x + left_eye_width / 2 + 1, left_eye_y + 5 + offset,
                          left_eye_x - left_eye_width / 2 - 1, left_eye_y + left_eye_height + offset,
                          SSD1306_BLACK);
    display.fillTriangle(right_eye_x + right_eye_width / 2 + 1, right_eye_y + offset,
                          right_eye_x - left_eye_width / 2 - 1, right_eye_y + 5 + offset,
                          right_eye_x + right_eye_width / 2 + 1, right_eye_y + right_eye_height + offset,
                          SSD1306_BLACK);
    offset -= 2;
    display.display();
    delay(1);
  }
  display.display();
  delay(1000);
}

void scrollDisplay(String lines[], int lineCount) {
  for (int i = 0; i < lineCount; i += 2) {
    display.clearDisplay();
    display.setTextSize(2);
    display.setTextColor(SSD1306_WHITE);

    display.setCursor(0, 0);
    display.println(lines[i]);

    if (i + 1 < lineCount) {
      display.setCursor(0, 20);
      display.println(lines[i + 1]);
    }

    display.display();
    delay(2000);
  }
}

int wrapText(String txt, String lines[], int maxCharsPerLine) {
  int lineCount = 0;
  String currentLine = "";
  String remaining = txt;

  while (remaining.length() > 0) {
    int spaceIndex = remaining.indexOf(' ');
    String word;

    if (spaceIndex == -1) {
      word = remaining;
      remaining = "";
    } else {
      word = remaining.substring(0, spaceIndex);
      remaining = remaining.substring(spaceIndex + 1);
    }

    if (currentLine.length() + word.length() + 1 > maxCharsPerLine) {
      lines[lineCount] = currentLine;
      lineCount++;
      currentLine = word;
    } else {
      if (currentLine.length() > 0) {
        currentLine += " ";
      }
      currentLine += word;
    }
  }

  if (currentLine.length() > 0) {
    lines[lineCount] = currentLine;
    lineCount++;
  }

  return lineCount;
}

void showText(String txt) {
  String lines[10];
  int lineCount = wrapText(txt, lines, 10);
  scrollDisplay(lines, lineCount);
}

const char htmlPage[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<body style="font-family:sans-serif;text-align:center;margin-top:40px;">
  <h2>ESP32-CAM Display</h2>
  <form action="/update" method="POST">
    <input type="text" name="msg" style="font-size:20px;padding:8px;width:250px;" autofocus>
    <br><br>
    <input type="submit" value="Send" style="font-size:20px;padding:8px 20px;">
  </form>
</body>
</html>
)rawliteral";

void handleRoot() {
  server.send(200, "text/html", htmlPage);
}

void handleUpdate() {
  if (server.hasArg("msg")) {
    showText(server.arg("msg"));
    center_eyes(true);
    lastBlinkTime = millis(); // reset blink timer so it doesn't blink right after a message
  }
  server.sendHeader("Location", "/");
  server.send(303);
}

void setup() {
  Serial.begin(115200);

  Wire.begin(OLED_SDA, OLED_SCL);
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println("OLED Failed");
    while (true);
  }
  Serial.println("OLED OK");

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println();
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  server.on("/", handleRoot);
  server.on("/update", HTTP_POST, handleUpdate);
  server.begin();

  wakeup();
  lastBlinkTime = millis();
}

void loop() {
  server.handleClient();

  if (millis() - lastBlinkTime > blinkInterval) {
    blink();
    lastBlinkTime = millis();
  }
}