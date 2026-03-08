#include <Arduino.h>
#include <ArduinoJson.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ST7735.h>
#include <WiFi.h>
#include <SpotifyEsp32.h>
#include <SPI.h>

// =====================
// YOUR CREDENTIALS HERE
// =====================
char* SSID           = "YOUR_WIFI_SSID";
char* PASSWORD       = "YOUR_WIFI_PASSWORD";
const char* CLIENT_ID     = "YOUR_SPOTIFY_CLIENT_ID";
const char* CLIENT_SECRET = "YOUR_SPOTIFY_CLIENT_SECRET";

// =====================
// PIN DEFINITIONS
// =====================
#define TFT_CS    3
#define TFT_RST   1
#define TFT_DC    2
#define TFT_SCLK  4
#define TFT_MOSI  5

#define BTN_LEFT   6   // SW1 - previous / move left in game
#define BTN_MID    7   // SW2 - play/pause / shoot in game
#define BTN_RIGHT  8   // SW3 - skip / move right in game

// =====================
// OBJECTS
// =====================
Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS, TFT_DC, TFT_MOSI, TFT_SCLK, TFT_RST);
Spotify sp(CLIENT_ID, CLIENT_SECRET);

// =====================
// DISPLAY DIMENSIONS
// =====================
#define SCREEN_W 160
#define SCREEN_H 128

// =====================
// MOOD COLOR THRESHOLDS
// These map Spotify energy/valence to background colors
// =====================
#define MOOD_HYPE   tft.color565(180, 0, 0)     // high energy - red
#define MOOD_HAPPY  tft.color565(180, 120, 0)   // happy - orange
#define MOOD_CHILL  tft.color565(0, 80, 160)    // chill - blue
#define MOOD_SAD    tft.color565(40, 0, 80)      // sad - dark purple
#define MOOD_NORMAL tft.color565(0, 0, 0)        // default - black

// =====================
// SPOTIFY STATE
// =====================
String lastTrack    = "";
String lastArtist   = "";
bool   lastPlaying  = false;
int    scrollOffset = 0;
unsigned long lastScrollTime  = 0;
unsigned long lastSpotifyPoll = 0;
unsigned long lastBarUpdate   = 0;
uint16_t currentMoodColor     = MOOD_NORMAL;

// bouncing bars state
int barHeights[3]     = {10, 20, 15};
int barTargets[3]     = {10, 20, 15};
unsigned long lastBarAnim = 0;

// progress bar
int trackDuration = 0;  // ms
int trackProgress = 0;  // ms

// =====================
// BUTTON STATE
// =====================
bool btn1Last = HIGH, btn2Last = HIGH, btn3Last = HIGH;
unsigned long btn1PressTime = 0, btn2PressTime = 0, btn3PressTime = 0;
bool allHeld = false;

// =====================
// GAME MODE
// =====================
bool gameMode = false;

// --- Space Invaders state ---
#define MAX_INVADERS  12
#define MAX_BULLETS    4

struct Invader {
  int x, y;
  bool alive;
};

struct Bullet {
  int x, y;
  bool active;
};

Invader invaders[MAX_INVADERS];
Bullet  playerBullets[MAX_BULLETS];
Bullet  invaderBullet;

int playerX        = SCREEN_W / 2;
int playerLives    = 3;
int score          = 0;
int invaderDir     = 1;  // 1 = right, -1 = left
unsigned long lastGameTick = 0;
bool gameOver      = false;
bool gameWon       = false;

// =====================
// FORWARD DECLARATIONS
// =====================
void drawSpotifyScreen();
void drawBars();
void scrollTrackName(String name);
uint16_t getMoodColor(String track);
void checkHoldForGameMode();
void runGame();
void initGame();
void drawGame();
void moveInvaders();
void movePlayerBullets();
void moveInvaderBullet();
void checkCollisions();
void drawPlayer();
void clearPlayer(int x);
void gameOverScreen();
void gameWonScreen();

// =====================
// SETUP
// =====================
void setup() {
  Serial.begin(115200);

  // Button pins
  pinMode(BTN_LEFT,  INPUT_PULLUP);
  pinMode(BTN_MID,   INPUT_PULLUP);
  pinMode(BTN_RIGHT, INPUT_PULLUP);

  // TFT init
  tft.initR(INITR_BLACKTAB);
  tft.setRotation(1);
  tft.fillScreen(ST77XX_BLACK);

  // Boot animation - display ECHO name
  tft.setTextSize(3);
  tft.setTextColor(ST77XX_WHITE);
  tft.setCursor(40, 50);
  tft.print("ECHO");
  delay(1000);
  tft.fillScreen(ST77XX_BLACK);

  // WiFi
  tft.setTextSize(1);
  tft.setTextColor(ST77XX_WHITE);
  tft.setCursor(0, 0);
  tft.print("Connecting to WiFi...");
  WiFi.begin(SSID, PASSWORD);
  int dots = 0;
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    tft.print(".");
    dots++;
    if (dots > 20) {
      tft.fillScreen(ST77XX_BLACK);
      tft.setCursor(0, 0);
      dots = 0;
    }
  }
  tft.fillScreen(ST77XX_BLACK);
  tft.setCursor(0, 0);
  tft.print("WiFi OK!");
  tft.setCursor(0, 12);
  tft.print(WiFi.localIP().toString().c_str());
  delay(2000);

  // Spotify auth
  tft.fillScreen(ST77XX_BLACK);
  tft.setCursor(0, 0);
  tft.print("Spotify auth...");
  tft.setCursor(0, 12);
  tft.print("Visit IP in browser");
  sp.begin();
  while (!sp.is_auth()) {
    sp.handle_client();
  }

  tft.fillScreen(ST77XX_BLACK);
  tft.setCursor(0, 0);
  tft.print("Authenticated!");
  delay(1000);
  tft.fillScreen(ST77XX_BLACK);
}

// =====================
// MAIN LOOP
// =====================
void loop() {
  checkHoldForGameMode();

  if (gameMode) {
    runGame();
  } else {
    // Poll Spotify every 2 seconds
    if (millis() - lastSpotifyPoll > 2000) {
      lastSpotifyPoll = millis();
      drawSpotifyScreen();
    }

    // Scroll long names
    if (millis() - lastScrollTime > 200) {
      lastScrollTime = millis();
      if (lastTrack.length() > 18) {
        scrollOffset++;
        if (scrollOffset > (int)lastTrack.length()) scrollOffset = 0;
        // redraw track name area only
        tft.fillRect(0, 40, SCREEN_W, 16, currentMoodColor);
        tft.setTextColor(ST77XX_WHITE);
        tft.setTextSize(2);
        tft.setCursor(0, 40);
        String scrolled = lastTrack.substring(scrollOffset) + "   " + lastTrack.substring(0, scrollOffset);
        tft.print(scrolled.substring(0, 13));
      }
    }

    // Animate bars
    if (millis() - lastBarAnim > 150 && lastPlaying) {
      lastBarAnim = millis();
      drawBars();
    }
  }
}

// =====================
// SPOTIFY DISPLAY
// =====================
void drawSpotifyScreen() {
  String currentTrack  = sp.current_track_name();
  String currentArtist = sp.current_artist_names();
  bool   isPlaying     = sp.is_playing();

  if (currentTrack == "Something went wrong" || currentTrack == "null" || currentTrack.isEmpty()) return;

  bool changed = (currentTrack != lastTrack || currentArtist != lastArtist || isPlaying != lastPlaying);

  if (changed) {
    lastTrack   = currentTrack;
    lastArtist  = currentArtist;
    lastPlaying = isPlaying;
    scrollOffset = 0;

    // Mood color
    currentMoodColor = getMoodColor(currentTrack);
    tft.fillScreen(currentMoodColor);

    // Playing indicator
    tft.setTextSize(1);
    tft.setTextColor(ST77XX_WHITE);
    tft.setCursor(0, 0);
    tft.print(isPlaying ? ">> NOW PLAYING" : "|| PAUSED");

    // Artist name (small)
    tft.setTextSize(1);
    tft.setCursor(0, 20);
    String artistShort = currentArtist.length() > 26 ? currentArtist.substring(0, 26) : currentArtist;
    tft.print(artistShort.c_str());

    // Track name (big, scrolls if long)
    tft.setTextSize(2);
    tft.setCursor(0, 40);
    if (currentTrack.length() <= 13) {
      tft.print(currentTrack.c_str());
    } else {
      tft.print(currentTrack.substring(0, 13).c_str());
    }

    // Bouncing bars
    drawBars();
  }

  // Progress bar at bottom
  tft.fillRect(0, SCREEN_H - 6, SCREEN_W, 6, tft.color565(50, 50, 50));
  // NOTE: SpotifyEsp32 doesn't expose progress directly
  // Replace progressPercent below with actual value if your lib supports it
  float progressPercent = 0.5; // PLACEHOLDER - replace with real progress if available
  int barW = (int)(progressPercent * SCREEN_W);
  tft.fillRect(0, SCREEN_H - 6, barW, 6, ST77XX_GREEN);
}

// =====================
// BOUNCING BARS ANIMATION
// =====================
void drawBars() {
  int barW  = 8;
  int barGap = 4;
  int startX = SCREEN_W - (3 * barW + 2 * barGap) - 4;
  int baseY  = SCREEN_H - 10;
  int maxH   = 30;

  for (int i = 0; i < 3; i++) {
    // Random target height
    if (random(10) > 6) {
      barTargets[i] = random(6, maxH);
    }
    // Ease toward target
    if (barHeights[i] < barTargets[i]) barHeights[i]++;
    else if (barHeights[i] > barTargets[i]) barHeights[i]--;

    int x = startX + i * (barW + barGap);
    // Clear old bar
    tft.fillRect(x, baseY - maxH, barW, maxH, currentMoodColor);
    // Draw new bar
    tft.fillRect(x, baseY - barHeights[i], barW, barHeights[i], ST77XX_GREEN);
  }
}

// =====================
// MOOD COLOR
// Based on track name length as a proxy (replace with Spotify audio features API if desired)
// =====================
uint16_t getMoodColor(String track) {
  // Placeholder mood logic - you can later hook into Spotify audio features
  // for real energy/valence values
  int len = track.length();
  if (len < 5)  return MOOD_SAD;
  if (len < 10) return MOOD_CHILL;
  if (len < 15) return MOOD_HAPPY;
  return MOOD_HYPE;
}

// =====================
// HOLD ALL 3 BUTTONS = TOGGLE GAME MODE
// =====================
void checkHoldForGameMode() {
  bool b1 = digitalRead(BTN_LEFT)  == LOW;
  bool b2 = digitalRead(BTN_MID)   == LOW;
  bool b3 = digitalRead(BTN_RIGHT) == LOW;

  if (b1 && b2 && b3) {
    if (!allHeld) {
      allHeld = true;
      btn1PressTime = millis();
    } else if (millis() - btn1PressTime > 2000) {
      // Held for 2 seconds - toggle game mode
      gameMode = !gameMode;
      allHeld = false;
      if (gameMode) {
        initGame();
      } else {
        tft.fillScreen(ST77XX_BLACK);
        lastTrack = ""; // force redraw
      }
      delay(500); // debounce
    }
  } else {
    allHeld = false;
  }
}

// =====================
// GAME - INIT
// =====================
void initGame() {
  tft.fillScreen(ST77XX_BLACK);
  playerX     = SCREEN_W / 2;
  playerLives = 3;
  score       = 0;
  gameOver    = false;
  gameWon     = false;
  invaderDir  = 1;

  // Clear bullets
  for (int i = 0; i < MAX_BULLETS; i++) playerBullets[i].active = false;
  invaderBullet.active = false;

  // Place invaders in a 4x3 grid
  int idx = 0;
  for (int row = 0; row < 3; row++) {
    for (int col = 0; col < 4; col++) {
      invaders[idx].x     = 10 + col * 35;
      invaders[idx].y     = 10 + row * 18;
      invaders[idx].alive = true;
      idx++;
    }
  }

  drawGame();
}

// =====================
// GAME - MAIN LOOP
// =====================
void runGame() {
  if (gameOver) { gameOverScreen(); return; }
  if (gameWon)  { gameWonScreen();  return; }

  bool b1 = digitalRead(BTN_LEFT)  == LOW;
  bool b2 = digitalRead(BTN_MID)   == LOW;
  bool b3 = digitalRead(BTN_RIGHT) == LOW;

  // Move player
  if (b1 && playerX > 4) {
    clearPlayer(playerX);
    playerX -= 3;
  }
  if (b3 && playerX < SCREEN_W - 12) {
    clearPlayer(playerX);
    playerX += 3;
  }

  // Shoot
  static unsigned long lastShot = 0;
  if (b2 && millis() - lastShot > 300) {
    lastShot = millis();
    for (int i = 0; i < MAX_BULLETS; i++) {
      if (!playerBullets[i].active) {
        playerBullets[i].x      = playerX + 4;
        playerBullets[i].y      = SCREEN_H - 20;
        playerBullets[i].active = true;
        break;
      }
    }
  }

  if (millis() - lastGameTick > 60) {
    lastGameTick = millis();
    moveInvaders();
    movePlayerBullets();
    moveInvaderBullet();
    checkCollisions();
    drawGame();
  }
}

// =====================
// GAME - DRAW
// =====================
void drawGame() {
  tft.fillScreen(ST77XX_BLACK);

  // Score & lives
  tft.setTextSize(1);
  tft.setTextColor(ST77XX_WHITE);
  tft.setCursor(0, 0);
  tft.print("SCORE:");
  tft.print(score);
  tft.setCursor(100, 0);
  tft.print("LVS:");
  tft.print(playerLives);

  // Invaders
  for (int i = 0; i < MAX_INVADERS; i++) {
    if (invaders[i].alive) {
      tft.fillRect(invaders[i].x, invaders[i].y, 10, 8, ST77XX_GREEN);
      // little antenna
      tft.drawPixel(invaders[i].x + 2, invaders[i].y - 2, ST77XX_GREEN);
      tft.drawPixel(invaders[i].x + 7, invaders[i].y - 2, ST77XX_GREEN);
    }
  }

  // Player bullets
  for (int i = 0; i < MAX_BULLETS; i++) {
    if (playerBullets[i].active) {
      tft.fillRect(playerBullets[i].x, playerBullets[i].y, 2, 6, ST77XX_YELLOW);
    }
  }

  // Invader bullet
  if (invaderBullet.active) {
    tft.fillRect(invaderBullet.x, invaderBullet.y, 2, 6, ST77XX_RED);
  }

  // Player ship
  drawPlayer();
}

void drawPlayer() {
  // Simple ship shape
  tft.fillTriangle(playerX, SCREEN_H - 10, playerX + 8, SCREEN_H - 10, playerX + 4, SCREEN_H - 18, ST77XX_CYAN);
}

void clearPlayer(int x) {
  tft.fillRect(x, SCREEN_H - 20, 10, 12, ST77XX_BLACK);
}

// =====================
// GAME - MOVE INVADERS
// =====================
void moveInvaders() {
  static int moveCounter = 0;
  moveCounter++;
  if (moveCounter < 3) return;
  moveCounter = 0;

  bool hitEdge = false;
  for (int i = 0; i < MAX_INVADERS; i++) {
    if (invaders[i].alive) {
      invaders[i].x += invaderDir * 2;
      if (invaders[i].x > SCREEN_W - 12 || invaders[i].x < 0) hitEdge = true;
    }
  }
  if (hitEdge) {
    invaderDir *= -1;
    for (int i = 0; i < MAX_INVADERS; i++) {
      if (invaders[i].alive) invaders[i].y += 6;
    }
  }

  // Random invader shoots
  if (!invaderBullet.active && random(20) == 0) {
    // Pick a random alive invader to shoot
    int shooter = random(MAX_INVADERS);
    for (int i = 0; i < MAX_INVADERS; i++) {
      int idx = (shooter + i) % MAX_INVADERS;
      if (invaders[idx].alive) {
        invaderBullet.x      = invaders[idx].x + 4;
        invaderBullet.y      = invaders[idx].y + 8;
        invaderBullet.active = true;
        break;
      }
    }
  }
}

// =====================
// GAME - MOVE BULLETS
// =====================
void movePlayerBullets() {
  for (int i = 0; i < MAX_BULLETS; i++) {
    if (playerBullets[i].active) {
      playerBullets[i].y -= 5;
      if (playerBullets[i].y < 0) playerBullets[i].active = false;
    }
  }
}

void moveInvaderBullet() {
  if (invaderBullet.active) {
    invaderBullet.y += 4;
    if (invaderBullet.y > SCREEN_H) invaderBullet.active = false;
  }
}

// =====================
// GAME - COLLISIONS
// =====================
void checkCollisions() {
  // Player bullets vs invaders
  for (int b = 0; b < MAX_BULLETS; b++) {
    if (!playerBullets[b].active) continue;
    for (int i = 0; i < MAX_INVADERS; i++) {
      if (!invaders[i].alive) continue;
      if (playerBullets[b].x >= invaders[i].x &&
          playerBullets[b].x <= invaders[i].x + 10 &&
          playerBullets[b].y >= invaders[i].y &&
          playerBullets[b].y <= invaders[i].y + 8) {
        invaders[i].alive         = false;
        playerBullets[b].active   = false;
        score                    += 10;
      }
    }
  }

  // Invader bullet vs player
  if (invaderBullet.active) {
    if (invaderBullet.x >= playerX &&
        invaderBullet.x <= playerX + 8 &&
        invaderBullet.y >= SCREEN_H - 18) {
      invaderBullet.active = false;
      playerLives--;
      if (playerLives <= 0) gameOver = true;
    }
  }

  // Invaders reach bottom
  for (int i = 0; i < MAX_INVADERS; i++) {
    if (invaders[i].alive && invaders[i].y >= SCREEN_H - 20) {
      gameOver = true;
    }
  }

  // All invaders dead
  bool anyAlive = false;
  for (int i = 0; i < MAX_INVADERS; i++) {
    if (invaders[i].alive) { anyAlive = true; break; }
  }
  if (!anyAlive) gameWon = true;
}

// =====================
// GAME OVER / WIN SCREENS
// =====================
void gameOverScreen() {
  tft.fillScreen(ST77XX_BLACK);
  tft.setTextColor(ST77XX_RED);
  tft.setTextSize(2);
  tft.setCursor(20, 40);
  tft.print("GAME OVER");
  tft.setTextColor(ST77XX_WHITE);
  tft.setTextSize(1);
  tft.setCursor(30, 70);
  tft.print("SCORE: ");
  tft.print(score);
  tft.setCursor(10, 90);
  tft.print("Hold all 3 to exit");
  delay(200);
}

void gameWonScreen() {
  tft.fillScreen(ST77XX_BLACK);
  tft.setTextColor(ST77XX_GREEN);
  tft.setTextSize(2);
  tft.setCursor(30, 40);
  tft.print("YOU WIN!");
  tft.setTextColor(ST77XX_WHITE);
  tft.setTextSize(1);
  tft.setCursor(30, 70);
  tft.print("SCORE: ");
  tft.print(score);
  tft.setCursor(10, 90);
  tft.print("Hold all 3 to exit");
  delay(200);
}
