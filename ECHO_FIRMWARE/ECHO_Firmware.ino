#include <Arduino.h>
#include <ArduinoJson.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ST7735.h>
#include <WiFi.h>
#include <SpotifyEsp32.h>
#include <SPI.h>

char* SSID     = "My WiFi SSID";
char* PASS     = "My WiFi Password";
const char* CLID  = "My Spotify Client ID";
const char* CLSEC = "My Spotify Client Secret";

#define TFT_CS    3
#define TFT_RST   1
#define TFT_DC    2
#define TFT_SCLK  4
#define TFT_MOSI  5

#define BTN_L  6
#define BTN_M  7
#define BTN_R  8

Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS, TFT_DC, TFT_MOSI, TFT_SCLK, TFT_RST);
Spotify sp(CLID, CLSEC);

#define SW 160
#define SH 128

// rough mood colors - not using audio features api, just proxying off name length for now
// TODO: hook into actual valence/energy endpoint when i have time
#define C_HYPE   tft.color565(180, 0, 0)
#define C_HAPPY  tft.color565(180, 120, 0)
#define C_CHILL  tft.color565(0, 80, 160)
#define C_SAD    tft.color565(40, 0, 80)
#define C_DEF    tft.color565(0, 0, 0)

String trkName  = "";
String trkArtist = "";
bool   playing  = false;
int    scrollOff = 0;
unsigned long tScroll = 0;
unsigned long tPoll   = 0;
uint16_t moodCol = C_DEF;

int bH[3] = {10, 20, 15};
int bT[3] = {10, 20, 15};
unsigned long tBar = 0;

// progress is faked rn - SpotifyEsp32 doesnt expose it cleanly
// leaving at 0.5 until i find a workaround
float prog = 0.5;

bool b1Lst = HIGH, b2Lst = HIGH, b3Lst = HIGH;
unsigned long bHoldT = 0;
bool held = false;

bool gMode = false;

#define N_INV  12
#define N_BLTS  4

struct Inv { int x, y; bool alive; };
struct Blt { int x, y; bool on; };

Inv invs[N_INV];
Blt pBullets[N_BLTS];
Blt iBullet;

int pX    = SW / 2;
int lives = 3;
int score = 0;
int iDir  = 1;
unsigned long tTick = 0;
bool gOver = false;
bool gWon  = false;

void drawSp();
void drawBars();
uint16_t moodColor(String t);
void checkHold();
void runGame();
void initGame();
void drawGame();
void moveInvs();
void movePBullets();
void moveIBullet();
void checkHits();
void drawShip();
void clearShip(int x);
void scrGameOver();
void scrWin();

void setup() {
  Serial.begin(115200);

  pinMode(BTN_L, INPUT_PULLUP);
  pinMode(BTN_M, INPUT_PULLUP);
  pinMode(BTN_R, INPUT_PULLUP);

  tft.initR(INITR_BLACKTAB);
  tft.setRotation(1);
  tft.fillScreen(ST77XX_BLACK);

  tft.setTextSize(3);
  tft.setTextColor(ST77XX_WHITE);
  tft.setCursor(40, 50);
  tft.print("ECHO");
  delay(1000);
  tft.fillScreen(ST77XX_BLACK);

  tft.setTextSize(1);
  tft.setTextColor(ST77XX_WHITE);
  tft.setCursor(0, 0);
  tft.print("Connecting...");
  WiFi.begin(SSID, PASS);
  int dots = 0;
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    tft.print(".");
    if (++dots > 20) {
      tft.fillScreen(ST77XX_BLACK);
      tft.setCursor(0, 0);
      dots = 0;
    }
  }

  tft.fillScreen(ST77XX_BLACK);
  tft.setCursor(0, 0);
  tft.print("WiFi up");
  tft.setCursor(0, 12);
  tft.print(WiFi.localIP().toString().c_str());
  delay(2000);

  tft.fillScreen(ST77XX_BLACK);
  tft.setCursor(0, 0);
  tft.print("Spotify auth...");
  tft.setCursor(0, 12);
  tft.print("open IP in browser");
  sp.begin();
  while (!sp.is_auth()) sp.handle_client();

  tft.fillScreen(ST77XX_BLACK);
  tft.setCursor(0, 0);
  tft.print("good to go");
  delay(800);
  tft.fillScreen(ST77XX_BLACK);
}

void loop() {
  checkHold();

  if (gMode) {
    runGame();
    return;
  }

  if (millis() - tPoll > 2000) {
    tPoll = millis();
    drawSp();
  }

  // scroll if name too long
  if (millis() - tScroll > 200 && trkName.length() > 18) {
    tScroll = millis();
    if (++scrollOff > (int)trkName.length()) scrollOff = 0;
    tft.fillRect(0, 40, SW, 16, moodCol);
    tft.setTextColor(ST77XX_WHITE);
    tft.setTextSize(2);
    tft.setCursor(0, 40);
    String s = trkName.substring(scrollOff) + "   " + trkName.substring(0, scrollOff);
    tft.print(s.substring(0, 13));
  }

  if (millis() - tBar > 150 && playing) {
    tBar = millis();
    drawBars();
  }
}

void drawSp() {
  String trk = sp.current_track_name();
  String art  = sp.current_artist_names();
  bool   isP  = sp.is_playing();

  if (trk == "Something went wrong" || trk == "null" || trk.isEmpty()) return;

  bool changed = (trk != trkName || art != trkArtist || isP != playing);
  if (!changed) {
    // still update progress bar even if nothing changed
    int bw = (int)(prog * SW);
    tft.fillRect(0, SH - 6, SW, 6, tft.color565(50, 50, 50));
    tft.fillRect(0, SH - 6, bw, 6, ST77XX_GREEN);
    return;
  }

  trkName   = trk;
  trkArtist = art;
  playing   = isP;
  scrollOff = 0;

  moodCol = moodColor(trk);
  tft.fillScreen(moodCol);

  tft.setTextSize(1);
  tft.setTextColor(ST77XX_WHITE);
  tft.setCursor(0, 0);
  tft.print(isP ? ">> NOW PLAYING" : "|| PAUSED");

  tft.setCursor(0, 20);
  String a = art.length() > 26 ? art.substring(0, 26) : art;
  tft.print(a.c_str());

  tft.setTextSize(2);
  tft.setCursor(0, 40);
  tft.print(trk.substring(0, 13).c_str());

  drawBars();

  int bw = (int)(prog * SW);
  tft.fillRect(0, SH - 6, SW, 6, tft.color565(50, 50, 50));
  tft.fillRect(0, SH - 6, bw, 6, ST77XX_GREEN);
}

void drawBars() {
  int bw = 8, gap = 4;
  int sx = SW - (3 * bw + 2 * gap) - 4;
  int by = SH - 10;
  int mh = 30;

  for (int i = 0; i < 3; i++) {
    if (random(10) > 6) bT[i] = random(6, mh);
    if (bH[i] < bT[i]) bH[i]++;
    else if (bH[i] > bT[i]) bH[i]--;
    int x = sx + i * (bw + gap);
    tft.fillRect(x, by - mh, bw, mh, moodCol);
    tft.fillRect(x, by - bH[i], bw, bH[i], ST77XX_GREEN);
  }
}

uint16_t moodColor(String t) {
  int l = t.length();
  if (l < 5)  return C_SAD;
  if (l < 10) return C_CHILL;
  if (l < 15) return C_HAPPY;
  return C_HYPE;
}

void checkHold() {
  bool b1 = digitalRead(BTN_L) == LOW;
  bool b2 = digitalRead(BTN_M) == LOW;
  bool b3 = digitalRead(BTN_R) == LOW;

  if (b1 && b2 && b3) {
    if (!held) { held = true; bHoldT = millis(); }
    else if (millis() - bHoldT > 2000) {
      gMode = !gMode;
      held  = false;
      if (gMode) initGame();
      else { tft.fillScreen(ST77XX_BLACK); trkName = ""; }
      delay(500);
    }
  } else {
    held = false;
  }
}

void initGame() {
  tft.fillScreen(ST77XX_BLACK);
  pX = SW / 2; lives = 3; score = 0;
  gOver = false; gWon = false; iDir = 1;

  for (int i = 0; i < N_BLTS; i++) pBullets[i].on = false;
  iBullet.on = false;

  int idx = 0;
  for (int r = 0; r < 3; r++)
    for (int c = 0; c < 4; c++)
      invs[idx++] = { 10 + c * 35, 10 + r * 18, true };

  drawGame();
}

void runGame() {
  if (gOver) { scrGameOver(); return; }
  if (gWon)  { scrWin();      return; }

  bool b1 = digitalRead(BTN_L) == LOW;
  bool b2 = digitalRead(BTN_M) == LOW;
  bool b3 = digitalRead(BTN_R) == LOW;

  if (b1 && pX > 4)         { clearShip(pX); pX -= 3; }
  if (b3 && pX < SW - 12)   { clearShip(pX); pX += 3; }

  static unsigned long tShot = 0;
  if (b2 && millis() - tShot > 300) {
    tShot = millis();
    for (int i = 0; i < N_BLTS; i++) {
      if (!pBullets[i].on) {
        pBullets[i] = { pX + 4, SH - 20, true };
        break;
      }
    }
  }

  if (millis() - tTick > 60) {
    tTick = millis();
    moveInvs(); movePBullets(); moveIBullet(); checkHits(); drawGame();
  }
}

void drawGame() {
  tft.fillScreen(ST77XX_BLACK);
  tft.setTextSize(1);
  tft.setTextColor(ST77XX_WHITE);
  tft.setCursor(0, 0);  tft.print("SC:"); tft.print(score);
  tft.setCursor(100, 0); tft.print("HP:"); tft.print(lives);

  for (int i = 0; i < N_INV; i++) {
    if (!invs[i].alive) continue;
    tft.fillRect(invs[i].x, invs[i].y, 10, 8, ST77XX_GREEN);
    tft.drawPixel(invs[i].x + 2, invs[i].y - 2, ST77XX_GREEN);
    tft.drawPixel(invs[i].x + 7, invs[i].y - 2, ST77XX_GREEN);
  }

  for (int i = 0; i < N_BLTS; i++)
    if (pBullets[i].on)
      tft.fillRect(pBullets[i].x, pBullets[i].y, 2, 6, ST77XX_YELLOW);

  if (iBullet.on)
    tft.fillRect(iBullet.x, iBullet.y, 2, 6, ST77XX_RED);

  drawShip();
}

void drawShip() {
  tft.fillTriangle(pX, SH-10, pX+8, SH-10, pX+4, SH-18, ST77XX_CYAN);
}

void clearShip(int x) {
  tft.fillRect(x, SH - 20, 10, 12, ST77XX_BLACK);
}

void moveInvs() {
  static int mv = 0;
  if (++mv < 3) return;
  mv = 0;

  bool edge = false;
  for (int i = 0; i < N_INV; i++) {
    if (!invs[i].alive) continue;
    invs[i].x += iDir * 2;
    if (invs[i].x > SW - 12 || invs[i].x < 0) edge = true;
  }
  if (edge) {
    iDir *= -1;
    for (int i = 0; i < N_INV; i++)
      if (invs[i].alive) invs[i].y += 6;
  }

  if (!iBullet.on && random(20) == 0) {
    int s = random(N_INV);
    for (int i = 0; i < N_INV; i++) {
      int idx = (s + i) % N_INV;
      if (invs[idx].alive) {
        iBullet = { invs[idx].x + 4, invs[idx].y + 8, true };
        break;
      }
    }
  }
}

void movePBullets() {
  for (int i = 0; i < N_BLTS; i++) {
    if (!pBullets[i].on) continue;
    pBullets[i].y -= 5;
    if (pBullets[i].y < 0) pBullets[i].on = false;
  }
}

void moveIBullet() {
  if (!iBullet.on) return;
  iBullet.y += 4;
  if (iBullet.y > SH) iBullet.on = false;
}

void checkHits() {
  for (int b = 0; b < N_BLTS; b++) {
    if (!pBullets[b].on) continue;
    for (int i = 0; i < N_INV; i++) {
      if (!invs[i].alive) continue;
      if (pBullets[b].x >= invs[i].x && pBullets[b].x <= invs[i].x + 10 &&
          pBullets[b].y >= invs[i].y && pBullets[b].y <= invs[i].y + 8) {
        invs[i].alive  = false;
        pBullets[b].on = false;
        score += 10;
      }
    }
  }

  if (iBullet.on &&
      iBullet.x >= pX && iBullet.x <= pX + 8 &&
      iBullet.y >= SH - 18) {
    iBullet.on = false;
    if (--lives <= 0) gOver = true;
  }

  for (int i = 0; i < N_INV; i++)
    if (invs[i].alive && invs[i].y >= SH - 20) gOver = true;

  bool any = false;
  for (int i = 0; i < N_INV; i++) if (invs[i].alive) { any = true; break; }
  if (!any) gWon = true;
}

void scrGameOver() {
  tft.fillScreen(ST77XX_BLACK);
  tft.setTextColor(ST77XX_RED);
  tft.setTextSize(2);
  tft.setCursor(20, 40);
  tft.print("GAME OVER");
  tft.setTextColor(ST77XX_WHITE);
  tft.setTextSize(1);
  tft.setCursor(30, 70); tft.print("SCORE: "); tft.print(score);
  tft.setCursor(10, 90); tft.print("hold all 3 to exit");
  delay(200);
}

void scrWin() {
  tft.fillScreen(ST77XX_BLACK);
  tft.setTextColor(ST77XX_GREEN);
  tft.setTextSize(2);
  tft.setCursor(30, 40);
  tft.print("YOU WIN!");
  tft.setTextColor(ST77XX_WHITE);
  tft.setTextSize(1);
  tft.setCursor(30, 70); tft.print("SCORE: "); tft.print(score);
  tft.setCursor(10, 90); tft.print("hold all 3 to exit");
  delay(200);
}
