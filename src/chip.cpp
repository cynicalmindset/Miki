#include <WiFi.h>
#include <WebServer.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <esp_camera.h>

#define PWDN_GPIO_NUM     32
#define RESET_GPIO_NUM    -1
#define XCLK_GPIO_NUM      0
#define SIOD_GPIO_NUM     26
#define SIOC_GPIO_NUM     27
#define Y9_GPIO_NUM       35
#define Y8_GPIO_NUM       34
#define Y7_GPIO_NUM       39
#define Y6_GPIO_NUM       36
#define Y5_GPIO_NUM       21
#define Y4_GPIO_NUM       19
#define Y3_GPIO_NUM       18
#define Y2_GPIO_NUM        5
#define VSYNC_GPIO_NUM    25
#define HREF_GPIO_NUM     23
#define PCLK_GPIO_NUM     22

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_SDA 13
#define OLED_SCL 2


#define SERVO_PIN 14
#define SERVO_CHANNEL 4

int ref_eye_height = 35;
int ref_eye_width = 30;
int ref_space_between_eye = 6;
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

void handleStream() {
  WiFiClient client = server.client();

  String boundary = "frame";
  client.println("HTTP/1.1 200 OK");
  client.println("Content-Type: multipart/x-mixed-replace; boundary=" + boundary);
  client.println();

  while (client.connected()) {
    camera_fb_t* fb = esp_camera_fb_get();
    if (!fb) {
      break;
    }

    uint8_t* jpgBuf = NULL;
    size_t jpgLen = 0;
    bool converted = frame2jpg(fb, 80, &jpgBuf, &jpgLen);
    esp_camera_fb_return(fb);

    if (!converted) {
      break;
    }

    client.println("--" + boundary);
    client.println("Content-Type: image/jpeg");
    client.println("Content-Length: " + String(jpgLen));
    client.println();
    client.write(jpgBuf, jpgLen);
    client.println();

    free(jpgBuf);

    if (!client.connected()) {
      break;
    }
  }
}

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

bool initCamera() {
  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = Y2_GPIO_NUM;
  config.pin_d1 = Y3_GPIO_NUM;
  config.pin_d2 = Y4_GPIO_NUM;
  config.pin_d3 = Y5_GPIO_NUM;
  config.pin_d4 = Y6_GPIO_NUM;
  config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM;
  config.pin_d7 = Y9_GPIO_NUM;
  config.pin_xclk = XCLK_GPIO_NUM;
  config.pin_pclk = PCLK_GPIO_NUM;
  config.pin_vsync = VSYNC_GPIO_NUM;
  config.pin_href = HREF_GPIO_NUM;
  config.pin_sscb_sda = SIOD_GPIO_NUM;
  config.pin_sscb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;
  config.pixel_format = PIXFORMAT_RGB565;
  config.frame_size = FRAMESIZE_QVGA;
  config.fb_count = 1;

  esp_err_t err = esp_camera_init(&config);
  return err == ESP_OK;
}

void handleCapture() {
  camera_fb_t* fb = esp_camera_fb_get();
  if (!fb) {
    server.send(500, "text/plain", "Camera capture failed");
    return;
  }

  uint8_t* jpgBuf = NULL;
  size_t jpgLen = 0;
  bool converted = frame2jpg(fb, 80, &jpgBuf, &jpgLen);

  esp_camera_fb_return(fb);

  if (!converted) {
    server.send(500, "text/plain", "JPEG conversion failed");
    return;
  }

  server.send_P(200, "image/jpeg", (const char*)jpgBuf, jpgLen);
  free(jpgBuf);
}

void nodYes() {
  for (int i = 0; i < 2; i++) {
    servoWrite(60);
    delay(300);
    servoWrite(120);
    delay(300);
  }
  servoWrite(90);
}

void shakeNo() {
  for (int i = 0; i < 3; i++) {
    servoWrite(70);
    delay(150);
    servoWrite(110);
    delay(150);
  }
  servoWrite(90);
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
    String msg = server.arg("msg");

    if (msg.indexOf("servo yes") != -1) {
      nodYes();
    }
    if (msg.indexOf("servo no") != -1) {
      shakeNo();
    }

    showText(msg);
    center_eyes(true);
    lastBlinkTime = millis();
  }
  server.sendHeader("Location", "/");
  server.send(303);
}

void initServo() {
  ledcAttach(SERVO_PIN, 50, 16); // pin, frequency (Hz), resolution (bits)
}

void servoWrite(int angle) {
  int pulseWidth = map(angle, 0, 180, 500, 2400);
  int duty = (int)((pulseWidth / 20000.0) * 65536);
  ledcWrite(SERVO_PIN, duty); // pin instead of channel
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

  if (!initCamera()) {
    Serial.println("Camera init failed");
  } else {
    Serial.println("Camera OK");
  }

  server.on("/", handleRoot);
  server.on("/update", HTTP_POST, handleUpdate);
  server.on("/capture", handleCapture);
  server.on("/stream", handleStream);
  server.begin();

  initServo();

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