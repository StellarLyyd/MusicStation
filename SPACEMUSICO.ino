#include <Arduino.h>
#include <DFRobotDFPlayerMini.h>
#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(0x27, 16, 2);

constexpr uint8_t pauseB = 8;
constexpr uint8_t nextB = 5;
constexpr uint8_t lastB = 7;
constexpr uint8_t busy = 6;

constexpr uint8_t PIN_MP3_TX = 16;
constexpr uint8_t PIN_MP3_RX = 17;

constexpr uint8_t slider = A5;

int lastVolume = 0;
int lastPaused = 0;

int currentTrack = 1;
int numSongs = 9;

bool paused = false;

bool nextPressed = false;
bool lastPressed = false;

// BUSY is HIGH when idle/paused,
// LOW while actually playing
bool lastBusy = HIGH;

String songs[] = {"Timber Hearth", "Outer Wilds", "Flower Dance", "Green Sleeves", "Lullaby", "Nuvole di Luce", "Canon", "Waltz A Minor", "Oceano"};

HardwareSerial dfSerial(1);
DFRobotDFPlayerMini player;


void setup() {

  Serial.begin(115200);
  delay(1000);

  pinMode(pauseB, INPUT_PULLDOWN);
  pinMode(nextB, INPUT_PULLDOWN);
  pinMode(lastB, INPUT_PULLDOWN);

  // DFPlayer drives this pin itself
  pinMode(busy, INPUT);

  dfSerial.begin(
    9600,
    SERIAL_8N1,
    PIN_MP3_RX,
    PIN_MP3_TX
  );
  lcd.init();
  lcd.backlight();
  delay(1000);

  bool ok = player.begin(dfSerial);

  if (!ok) {
    Serial.println("DFPlayer failed");
    return;
  }

  Serial.println("DFPlayer ready");
  player.volume(10);
  delay(500);
  player.play(1);
  delay(200);
  lcd.setCursor(0, 0);
  lcd.print("Now Playing: ");
  lcd.setCursor(0, 1);
  lcd.print("                ");
  lcd.setCursor(0, 1);
  lcd.print(songs[currentTrack-1]);

  lastBusy = digitalRead(busy);
}


int readVolume() {

  int raw = analogRead(slider);

  int volume = map(
    raw,
    0,
    4095,
    5,
    25
  );

  return constrain(volume, 5, 25);
}


void pauseIt() {
  bool pauseNow = digitalRead(pauseB);

  if (pauseNow == HIGH &&
      paused == false &&
      lastPaused == LOW) {

    paused = true;

    player.pause();
    lcd.setCursor(0, 0);
    lcd.print("Paused       ");
  }

  else if (pauseNow == HIGH &&
           paused == true &&
           lastPaused == LOW) {

    paused = false;
    player.start();
    lcd.setCursor(0, 0);
    lcd.print("Now Playing: ");
  }
  lastPaused = pauseNow;
}


void switchIt() {

  bool nextNow = digitalRead(nextB);
  bool lastNow = digitalRead(lastB);
  bool busyNow = digitalRead(busy);


  // =============================
  // NEXT BUTTON
  // =============================

  if (nextNow == HIGH &&
      nextPressed == LOW) {

    player.stop();

    currentTrack++;

    if (currentTrack > numSongs) {
      currentTrack = 1;
    }

    paused = false;

    player.play(currentTrack);
  }


  // =============================
  // PREVIOUS BUTTON
  // =============================

  else if (lastNow == HIGH &&
           lastPressed == LOW) {

    player.stop();

    currentTrack--;

    if (currentTrack < 1) {
      currentTrack = numSongs;
    }

    paused = false;

    player.play(currentTrack);
  }

  else if (
    lastBusy == LOW &&
    busyNow == HIGH &&
    paused == false
  ) {

    currentTrack++;

    if (currentTrack > numSongs) {
      currentTrack = 1;
    }
    lcd.setCursor(0, 0);
    lcd.print("Now Playing: ");
    lcd.setCursor(0, 1);
    lcd.print("                ");
    lcd.setCursor(0, 1);
    lcd.print(songs[currentTrack-1]);

    player.play(currentTrack);
  }


  nextPressed = nextNow;
  lastPressed = lastNow;
  lastBusy = busyNow;
}


void loop() {
  int vol = readVolume();
  if (abs(vol - lastVolume) >= 1) {

    player.volume(vol);

    lastVolume = vol;
  }
  pauseIt();
  switchIt();
  delay(20);
}