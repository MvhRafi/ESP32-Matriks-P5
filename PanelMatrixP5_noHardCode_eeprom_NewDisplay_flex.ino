 #include <ESP32-HUB75-MatrixPanel-I2S-DMA.h>
#include <Adafruit_GFX.h>
#include <WiFi.h>
// #include <LittleFS.h>
#include <EEPROM.h>
#include <time.h>
#include "PrayerTimes.h"
// #include <Fonts/FreeMonoBold12pt7b.h>
#include <Fonts/FreeSansBold9pt7b.h>
// #include <Fonts/FreeSerif12pt7b.h>
#include <Fonts/FreeSans9pt7b.h>
// #include <Fonts/FreeSerifItalic9pt7b.h>
// #include <Fonts/FreeSansBold12pt7b.h>
#include <Fonts/FreeMono9pt7b.h>
// #include <Fonts/Org_01.h>
// #include <Fonts/TomThumb.h>
// #include <Fonts/Picopixel.h>
// #include <PressStart2P_Regular6pt7b.h>
#include <DS_Digital14x19.h>
#include <FreeSansBold5pt7b.h>
#include <FreeSans6pt7b.h>
#include <RobotoSlab_Bold7pt7b.h>
#include <RobotoSlab_Light7pt7b.h>
#include <DS_DIGI4pt7b.h>
#include <FreeSerifBold6pt7b.h>

#include "colorarray1.h"
#include "img_1.h"
#include "img_2.h"
#include "img_3.h"
#include "img_4.h"

#define EEPROM_SIZE 4096

#define ADDR_TEXTS_COUNT 0
#define ADDR_TEXTS_START 10
#define ADDR_TEXTS_END 700

#define ADDR_VERTS_COUNT 700
#define ADDR_VERTS_START 710
#define ADDR_VERTS_END 1400

#define ADDR_QUOTES_COUNT 1400
#define ADDR_QUOTES_START 1410
#define ADDR_QUOTES_END 2000

#define ADDR_QUOTEVERTS_COUNT 2000
#define ADDR_QUOTEVERTS_START 2010
#define ADDR_QUOTEVERTS_END 2600

#define ADDR_INFO_COUNT 2600
#define ADDR_INFO_START 2610
#define ADDR_INFO_END 3200

#define ADDR_IMAM_COUNT 3200
#define ADDR_IMAM_START 3210
#define ADDR_IMAM_END 4000

#define EEPROM_INITIALIZED_FLAG 3990
#define EEPROM_MAGIC_NUMBER 0xAB

#define PANEL_RES_X 64  
#define PANEL_RES_Y 32
#define PANEL_CHAIN 2  //ganti ukuran tinggal rubah ini

#define R1 19
#define G1 13
#define BL1 18
#define R2 5
#define G2 12
#define BL2 17
#define CH_A 16
#define CH_B 14
#define CH_C 4
#define CH_D 27
#define CH_E -1 
#define CLK 2
#define LAT 26
#define OE 15

MatrixPanel_I2S_DMA *dma_display = nullptr;

GFXcanvas16 *layoutJam = nullptr;
GFXcanvas16 *layoutDate = nullptr;
GFXcanvas16 *layoutText = nullptr;
GFXcanvas16 *midText = nullptr;
GFXcanvas16 *QuoteLayout = nullptr;
GFXcanvas16 *QuoteLayout2 = nullptr;
// GFXcanvas16 *fullPanel = nullptr;
GFXcanvas16 *runningText8pt = nullptr;

char ssid[] = "Hidysm";
char pass[] = "12334567890";

#define TZ 7
#define TZ_S (TZ * 3600)

bool flasher = false;
int h, m, s, d, yr, dow;
uint8_t month_index;
String text;
unsigned long lastTimeUpdate = 0;
unsigned long NewRTCh = 24;
unsigned long NewRTCm = 60;
unsigned long NewRTCs = 10;
time_t t;

const char* wd[7] =  {"MINGGU", "SENIN", "SELASA", "RABU", "KAMIS", "JUMAT", "SABTU"};
const char* monthName[12] = {"Jan", "Feb", "Maret", "April", "Mei", "Juni", "Juli", "Aug", "Sep", "Okt", "Nov", "Dec"};

enum StatusAnimasi { dayDown, dayStuck, dayUp, dateDown, dateStuck, dateUp};
StatusAnimasi animState = dayDown;
int animPos;
unsigned long lastAnimUpdate = 0;
unsigned long animDelay = 50;
unsigned long animDuration = 5000;
unsigned long diam = 0;

int textPos;
int lebarText = 0;
unsigned long lastScroll = 0;
unsigned long scrollSpeed = 30;
int ScrolPlus;

unsigned long lastPrayerScroll = 0;
int prayerScrollSpeed = 50;

int textPos96;
unsigned long lastScroll96Pixel = 0;

//diedit  27 Januri 2026
#define MAX_TEXT 10
#define MAX_TEXT_LENGTH 120

int currentTextIndex = 0;
String texts[MAX_TEXT];
byte Color[MAX_TEXT];
int textCount = 0;

int currentVertIndex = 0;
String vertText[MAX_TEXT]; 
byte vertColor[MAX_TEXT]; 
int vertTextCount = 0;

int currentQuoteIndex = 0;
String QuoteText[MAX_TEXT];
byte QuoteColor[MAX_TEXT];
int QuoteCount = 0;

int currentQuoteVertIndex = 0;
String QuoteVertText[MAX_TEXT];
byte QuoteVertColor[MAX_TEXT];
int QuoteVertCount = 0;

//informasi 1
String textInfo[MAX_TEXT];
byte ColorInfo[MAX_TEXT];
int textInfoCount = 0;
int currentInfoIndex = 0;
int infoPos = PANEL_RES_X * PANEL_CHAIN;
unsigned long lastInfoScroll = 0;

//informasi 2
String textInfo1[MAX_TEXT];
byte colorInfo1[MAX_TEXT];
int textInfoCount1 = 0;
int currentInfoIndex1 = 0;
int infoPos1  = PANEL_RES_X * PANEL_CHAIN;
unsigned long lastInfoScroll1 = 0;

//variabel kontrol animasi
int prayerIdx = 0;
unsigned long lastMove = 0;
unsigned long pauseTime = 0;
bool  isPausing = false;

//konfigurasi
int pauseDuration = 3000;

int scrollOffset = 0;
unsigned long lastScrollTime = 0;
int Speed = 50;

double times[7];
double lat = -7.7956;
double lon = 110.3695;
int gtmOfset = 7;
int last_calculated_day = -1;
bool prayer_times_valid = false;

const char* waktuShalat[] = {"Subuh", "terbit", "Dzuhur", "Ashar","Sunset", "Maghrib", "Isya"};

int shift = 0;
unsigned long pergantianShift = 0;
unsigned long shiftDuration = 20000;

struct ShiftLoops {
  int currentLoop;
  int maxLoop;
  bool completed;
};

ShiftLoops shiftLoop[5] = {
  {0, 1, false},
  {0, 1, false},
  {0, vertTextCount, false},
  {0, 1, false},
  {0, 1, false}
};

int runningTextLoopCount = 0;
int runningTextFullCycle = 0;
int prayerTimeScrool = 0;
int runningQuotefullCycle = 0;
int runningQuotefullCycle2 = 0;

int col;
int row;
int led_wiring = 1;

int halfCol;
int halfRow;
int thirdCol;
int quarterRow;
int width;
int height;
int contentArea;
int contentStartX;
int runTeks96;
int jamXStart;


int posImgKanan;

int currentFrame = 0;
const int totalFrames = 162;
const int frameSize = 32 * 32 * 2;
int textPoskecil = posImgKanan;
int scrollx = col;

unsigned char saveImage1[128];
unsigned char saveImage2[128];
unsigned char saveImage3[128];
unsigned char saveImage4[128];
unsigned char saveImage5[128];
unsigned char saveImage6[128];

String currentOpenFile = "";

struct pixelColor {
  uint8_t r;
  uint8_t g;
  uint8_t b;
};

byte colorDate = 0;

String serialInput = "";

struct CategoryInfo {
  const char* name;
  String* textArray;
  byte* colorArray;
  int* countPtr;
  int countAddr;
  int startAddr;
  int endAddr;
  byte defaultColor;
};

CategoryInfo categories[6] = {
  {"texts", texts, Color, &textCount, ADDR_TEXTS_COUNT, ADDR_TEXTS_START, ADDR_TEXTS_END, 18},
  {"verts", vertText, vertColor, &vertTextCount, ADDR_VERTS_COUNT, ADDR_VERTS_START, ADDR_VERTS_END, 15},
  {"quotes", QuoteText, QuoteColor, &QuoteCount, ADDR_QUOTES_COUNT, ADDR_QUOTES_START, ADDR_QUOTES_END, 10},
  {"quoteverts", QuoteVertText, QuoteVertColor, &QuoteVertCount, ADDR_QUOTEVERTS_COUNT, ADDR_QUOTEVERTS_START, ADDR_QUOTEVERTS_END, 18},
  {"info", textInfo, ColorInfo, &textInfoCount, ADDR_INFO_COUNT, ADDR_INFO_START, ADDR_INFO_END, 2},
  {"imam", textInfo1, colorInfo1, &textInfoCount1, ADDR_IMAM_COUNT, ADDR_IMAM_START, ADDR_IMAM_END, 2}
};

//manaement file from FS
void saveText(int countAddr, int startAddr, int endAddr, String* textArray, int count){
  if(count > MAX_TEXT) count = MAX_TEXT;

  EEPROM.begin(EEPROM_SIZE);
  EEPROM.write(countAddr, count);

  int addr = startAddr;
  for(int i = 0; i < count; i++){
    int len = textArray[i].length();
    if(len > MAX_TEXT_LENGTH) len = MAX_TEXT_LENGTH;

    if(addr + len + 1 > endAddr){
      EEPROM.write(countAddr, i);
      break;
    }

    for(int j  =0;  j < len; j++){
      EEPROM.write(addr++, textArray[i][j]);
    }
    EEPROM.write(addr++,'\0');
  }

  EEPROM.commit();
  EEPROM.end();
  Serial.printf("Save %d text  to EEPROMM (addr %d-%d)\n", count , countAddr);
}

void loadTextsFromEEPROM(int countAddr, int startAddr, int endAddr, String* textArray, byte* colorArray, int &count, byte  defaultColor){
  EEPROM.begin(EEPROM_SIZE);
  count = EEPROM.read(countAddr);

  if(countAddr == 255 || count == 0 || count > MAX_TEXT){
    count = 0;
    EEPROM.end();
    Serial.printf("no data di alamat %d (count = %d)\n", countAddr, count);
    return;
  }

  Serial.printf("\n loading from EEPROM (addr %d)\n", countAddr);

  int addr = startAddr;
  for(int i = 0; i < count && addr  < endAddr; i++){
    String text = "";
    char c;
    while (addr < endAddr && (c = EEPROM.read(addr++)) != '\0'){
      text += c;
      if(text.length() >= MAX_TEXT_LENGTH) break;
    }

    if(text.length() > 0){
      textArray[i] = text;
      colorArray[i] = i * 3;
    }
  }
  EEPROM.end();
  Serial.printf("Load %d text\n", count); 
}

//load file.txt
void loadAllText(){
  loadTextsFromEEPROM(ADDR_TEXTS_COUNT, ADDR_TEXTS_START, ADDR_TEXTS_END, texts, Color, textCount, 18);
  loadTextsFromEEPROM(ADDR_VERTS_COUNT, ADDR_VERTS_START, ADDR_VERTS_END, vertText, vertColor, vertTextCount, 15);
  loadTextsFromEEPROM(ADDR_QUOTES_COUNT, ADDR_QUOTES_START, ADDR_QUOTES_END, QuoteText,QuoteColor, QuoteCount, 10);
  loadTextsFromEEPROM(ADDR_QUOTEVERTS_COUNT, ADDR_QUOTEVERTS_START, ADDR_QUOTEVERTS_END,QuoteVertText, QuoteVertColor, QuoteVertCount, 18);
  loadTextsFromEEPROM(ADDR_INFO_COUNT, ADDR_INFO_START, ADDR_INFO_END,textInfo, ColorInfo, textInfoCount, 2);
  loadTextsFromEEPROM(ADDR_IMAM_COUNT, ADDR_IMAM_START, ADDR_IMAM_END,textInfo1, colorInfo1, textInfoCount1, 2);
}

void createDefaultFiles() {
  Serial.println("\nCheck dan buat default files...\n");
  
  EEPROM.begin(EEPROM_SIZE);
  byte flag = EEPROM.read(EEPROM_INITIALIZED_FLAG);
  if(flag == EEPROM_MAGIC_NUMBER){
    EEPROM.end();
    Serial.println("Eeprom Ready!");
    return;
  }
  EEPROM.end();
}

void showEEPROMInfo(){
  Serial.println("\n EEPRM Storage Info : ");;
  Serial.printf("Total Size : %d bytes (4 KB)\n", EEPROM_SIZE);
  Serial.println("Address Map :");
  Serial.printf("Texts : %d - %d\n", ADDR_TEXTS_START, ADDR_TEXTS_END);
  Serial.printf("verts : %d - %d\n", ADDR_VERTS_START, ADDR_VERTS_END);
  Serial.printf("quotes : %d - %d\n", ADDR_QUOTES_START, ADDR_QUOTES_END);
  Serial.printf("quotverts : %d - %d\n", ADDR_QUOTEVERTS_START, ADDR_QUOTEVERTS_END);
  Serial.printf("info : %d - %d\n", ADDR_INFO_START, ADDR_INFO_END);
  Serial.printf("imam : %d - %d\n", ADDR_IMAM_START, ADDR_IMAM_END);
}

void printMenu(){
  Serial.println("LED PANEL P5 EDITOR");
  Serial.println("\n AVAILABLE COMANDS :");
  Serial.println(" 1. LIST <category> - Lihat isi texts");
  Serial.println(" 2. ADD <category> - Tambah texts baru");
  Serial.println(" 3. DELETE <category> - Hapus text baru");
  Serial.println(" 4. CLEAR <category> - Hapus semua text");
  Serial.println(" 5. RELOAD - Reload semua texts");
  Serial.println(" 6. INFO - Lihat info EEPROM");
  Serial.println(" 7. RESET - Reset ke default");
  Serial.println(" 8. MENU - Tampilkan menu Baru");

  Serial.println("\n AVAILABLE  CATEGORY");
  Serial.println(" -texts (Running text horizontal)");
  Serial.println(" -verts (Running text Vertikal)");
  Serial.println(" -quotes (Qoutes texts horizontal)");
  Serial.println(" -quoteverts (Quotes texts vertikal)");
  Serial.println(" -info (Running text informasi)");
  Serial.println(" -Imam (Jadwal imam Jum'at)");
}

CategoryInfo* findCategory(String name){
  for(int i = 0; i < 6; i++){
    if(name == categories[i].name){
      return &categories[i];
    }
  }
  return nullptr;
}

void listCategory(String category){
  CategoryInfo* cat = findCategory(category);
  if(!cat){
    Serial.println("Kategory tidak dikenal");
      return;
    }
    Serial.println("\n isi category" + category);
    
    if(*cat->countPtr == 0){
      Serial.println(" (Kosong)");
    } else {
      for(int i  = 0;  i < *cat->countPtr; i++){
        Serial.printf(" [%d] %s\n", i + 1, cat->textArray[i].c_str());
      }
    }
  Serial.printf("Total: %d baris\n\n", *cat->countPtr);
}

void addTextToCategory(String category){
  CategoryInfo* cat = findCategory(category);
  if(!cat){
    Serial.println("Kategory tidak dikenal");
    return;
  }

  Serial.println("\n MODE: Tambah texts!");
  Serial.println("Ketik DONE, Jika selesai!");
  Serial.println("Ketik CANCEL, jika batal");

  int addedCount = 0;
  while(true){
    while(!Serial.available()){
      yield();
    }

    String newText = Serial.readStringUntil('\n');
    newText.trim();

    if(newText.equalsIgnoreCase("DONE")){
      break;
    }

    if(newText.equalsIgnoreCase("CANCEL")){
      return;
    }

    if(newText.length() > 0  && newText.length() <= MAX_TEXT_LENGTH){
      if(*cat->countPtr < MAX_TEXT){
        cat->textArray[*cat->countPtr] = newText;
        (*cat -> countPtr)++;
        Serial.printf("  [%d] ditambahkan : %s\n", addedCount + 1, newText.c_str());
        addedCount++;
      } else {
        Serial.println("Maksimal" + String(MAX_TEXT) + "Teks tercapai");
        break;
      }
    } else if(newText.length() > MAX_TEXT_LENGTH){
      Serial.println(" teks terlalu panjang (max 120 Karakter!)");
    }
  }

  if(addedCount > 0){
    Serial.println("\n menyimpan ke eeprom!");
    saveText(cat->countAddr, cat->startAddr, cat->endAddr, cat->textArray, *cat->countPtr);
  }
}

void deleteFromCategory(String category){
  CategoryInfo* cat = findCategory(category);
  if(!cat){
    Serial.println("Kategory tidak dikenal!");
    return;
  }

  if(*cat->countPtr == 0){
    Serial.println("Kategory kosong!\n");
    return;
  }

  Serial.println("\n Isi saat ini:");
  for(int i = 0; i < *cat->countPtr; i++) {
    Serial.printf("  [%d] %s\n", i + 1, cat->textArray[i].c_str());
  }
  
  Serial.printf("\nMasukkan nomor yang ingin dihapus (1-%d): ", *cat->countPtr);

  delay(10);
  while(Serial.available() > 0) Serial.read();
  
  while(!Serial.available()) {
    yield();
  }
  
  String input = Serial.readStringUntil('\n');
  input.trim();
  int deleteIndex = input.toInt() - 1;
  
  if(deleteIndex < 0 || deleteIndex >= *cat->countPtr) {
    Serial.println(" Nomor tidak valid!\n");
    return;
  }
  
  Serial.printf(" Hapus: \"%s\" ? (Y/N): ", cat->textArray[deleteIndex].c_str());
  
  while(!Serial.available()) {
    yield();
  }
  
  String confirm = Serial.readStringUntil('\n');
  confirm.trim();
  
  if(!confirm.equalsIgnoreCase("Y")) {
    Serial.println(" Dibatalkan.\n");
    return;
  }
  
  // Shift array
  for(int i = deleteIndex; i < *cat->countPtr - 1; i++) {
    cat->textArray[i] = cat->textArray[i + 1];
  }
  (*cat->countPtr)--;
  
  Serial.println("\n Menyimpan ke EEPROM...");
  saveText(cat->countAddr, cat->startAddr, cat->endAddr, cat->textArray, *cat->countPtr);
  
  Serial.println("Teks berhasil dihapus!");
  Serial.println("Ketik 'RELOAD' untuk menerapkan perubahan di LED.\n");
}

void clearCategory(String category) {
  CategoryInfo* cat = findCategory(category);
  if(!cat) {
    Serial.println(" Kategory tidak dikenal!");
    return;
  }
  
  Serial.printf("hapus SEMUA teks di '%s'? (Y/N): ", category.c_str());
  
  delay(10);
  while(Serial.available() > 0)Serial.read();
  while(!Serial.available()) {
    yield();
  }
  
  String confirm = Serial.readStringUntil('\n');
  confirm.trim();
  
  if(!confirm.equalsIgnoreCase("Y")) {
    Serial.println(" Dibatalkan.\n");
    return;
  }
  
  *cat->countPtr = 0;
  
  Serial.println("\n Menyimpan ke EEPROM...");
  saveText(cat->countAddr, cat->startAddr, cat->endAddr, cat->textArray, 0);
  
  Serial.println(" Category berhasil dikosongkan!");
  Serial.println(" Ketik 'RELOAD' untuk menerapkan perubahan di PANEL.\n");
}

void resetToDefault() {
  Serial.println("\n RESET semua teks ke default? (Y/N): ");
  
  delay(10);
  while(Serial.available() > 0)Serial.read();

  while(!Serial.available()) {
    yield();
  }
  
  String confirm = Serial.readStringUntil('\n');
  confirm.trim();
  
  if(!confirm.equalsIgnoreCase("Y")) {
    Serial.println("Dibatalkan.\n");
    return;
  }
  
  // Clear EEPROM
  EEPROM.begin(EEPROM_SIZE);
  for(int i = 0; i < EEPROM_SIZE; i++) {
    EEPROM.write(i, 0xFF);
  }
  EEPROM.commit();
  EEPROM.end();
  
  Serial.println("EEPROM cleared, creating defaults...\n");
  
  createDefaultFiles();
  loadAllText();
  
  Serial.println(" Reset complete!\n");
}

void reloadAllTextsCommand() {
  Serial.println("\n Reloading all texts...\n");

  resetAllAnimationStates();
  // // Reset indices
  // currentTextIndex = 0;
  // currentVertIndex = 0;
  // currentQuoteIndex = 0;
  // currentQuoteVertIndex = 0;
  // currentInfoIndex = 0;
  // currentInfoIndex1 = 0;
  
  // // Reset positions
  // textPos = col;
  // ScrolPlus = col;
  // // textPos96 = thirdCol * 3;
  // // scrollOffset = 0;
  
  loadAllText();

  shiftLoop[0].maxLoop = 1;
  shiftLoop[1].maxLoop = 1;
  shiftLoop[2].maxLoop = vertTextCount;
  shiftLoop[3].maxLoop = 1;
  shiftLoop[4].maxLoop = 1;
  
  Serial.println("RELOAD COMPLETE!");
}

void handleSerialCommand() {
  if(Serial.available()) {
    char c = Serial.read();
    
    if(c == '\n' || c == '\r') {
      if(serialInput.length() > 0) {
        serialInput.trim();
        serialInput.toUpperCase();
        
        int spaceIndex = serialInput.indexOf(' ');
        String command = (spaceIndex > 0) ? serialInput.substring(0, spaceIndex) : serialInput;
        String param = (spaceIndex > 0) ? serialInput.substring(spaceIndex + 1) : "";
        param.toLowerCase();
        
        if(command == "MENU") {
          printMenu();
        }
        else if(command == "LIST") {
          if(param.length() > 0) listCategory(param);
          else Serial.println("Format: LIST <category>\n   Contoh: LIST texts\n");
        }
        else if(command == "ADD") {
          if(param.length() > 0) addTextToCategory(param);
          else Serial.println("Format: ADD <category>\n   Contoh: ADD texts\n");
        }
        else if(command == "DELETE") {
          if(param.length() > 0) deleteFromCategory(param);
          else Serial.println("Format: DELETE <category>\n   Contoh: DELETE verts\n");
        }
        else if(command == "CLEAR") {
          if(param.length() > 0) clearCategory(param);
          else Serial.println("Format: CLEAR <category>\n   Contoh: CLEAR quotes\n");
        }
        else if(command == "RELOAD") {
          reloadAllTextsCommand();
        }
        else if(command == "INFO") {
          showEEPROMInfo();
        }
        else if(command == "RESET") {
          resetToDefault();
        }
        else {
          Serial.println("Command tidak dikenal!");
        }
        serialInput = "";
      }
    } else {
      serialInput += c;
    }
  }
}

//tambahan an fungsi karena bug animasi ke skip karena adanya variabel yang nggak ke reset
void resetAllAnimationStates() {
  scrollOffset = 0;
  textPos96 = col;
  textPos = col;
  textPoskecil = col;
  ScrolPlus = col;
  scrollx = col;
 
  prayerIdx = 0;
  isPausing = false;
  lastMove = 0;
  pauseTime = 0;
 
  currentTextIndex = 0;
  currentVertIndex = 0;
  currentQuoteIndex = 0;
  currentQuoteVertIndex = 0;
  currentInfoIndex = 0;
  currentInfoIndex1 = 0;
  
  animPos = -quarterRow;
  animState = dayDown;
 
  shift = 0;
  
  runningTextLoopCount = 0;
  runningTextFullCycle = 0;
  prayerTimeScrool = 0;
  runningQuotefullCycle = 0;
  runningQuotefullCycle2 = 0;

  lastScroll96Pixel = 0;
  lastScrollTime = 0;
  lastScroll = 0;
  lastInfoScroll = 0;
  lastInfoScroll1 =0;
  
  for(int i = 0; i < 5; i++) {
    shiftLoop[i].currentLoop = 0;
    shiftLoop[i].completed = false;
  }
  
  Serial.println("Riset semua variabel Animasi!");
}

void setup() {
  Serial.begin(115200);
  
  col = PANEL_RES_X * PANEL_CHAIN; //192
  row = PANEL_RES_Y; // 32
  halfCol = col / 2; //64 //96
  halfRow = row / 2; //16 
  thirdCol = col / 4; // 32 //
  quarterRow = row / 4; // 8
  contentArea = col - 64; //misal 128 - 64 = 64 // 192 - 64 = 128 dst kenapa 64 akrena 2 gambar 32x32px
  contentStartX  =32;
  runTeks96 = col - 32;
  jamXStart = (col - 64)/2;
  /// col 

  posImgKanan = col - 32; // khususs untuk img 32 x 32;

  setupDisplay();
  
  layoutJam = new GFXcanvas16(64, halfRow);
  layoutDate = new GFXcanvas16(halfCol, quarterRow);
  layoutText = new GFXcanvas16(col, halfRow);
  midText = new GFXcanvas16(contentArea, row);
  QuoteLayout = new GFXcanvas16(runTeks96, row);
  QuoteLayout2 = new GFXcanvas16(runTeks96, row);
  // fullPanel = new GFXcanvas16(col, row);
  runningText8pt = new GFXcanvas16(contentArea, quarterRow);
  
  // animPos = -halfRow; //-16
  // textPos = col; //128
  // ScrolPlus = col; //128
  // textPos96 = thirdCol * 3; //96
  resetAllAnimationStates();

  wifiBegin();

  createDefaultFiles();
  loadAllText();

  shiftLoop[0].maxLoop = 1;
  shiftLoop[1].maxLoop = 1;
  shiftLoop[2].maxLoop = vertTextCount;
  shiftLoop[3].maxLoop = 1;
  shiftLoop[4].maxLoop = 1;

  ntpSetup();

  Serial.println("LED Panel P5 128x32. Started!");
  Serial.println("Menunggu waktu valid dari NTP...");

  delay(2000);
  printMenu();
}

void loop() {
  unsigned long currentMillis = millis();

  handleSerialCommand();

  if(currentMillis - lastTimeUpdate >= 1000){
    lastTimeUpdate = currentMillis;
    updateTime();
    flasher = !flasher;
    // checkUpdatePrayerTimes();
  }

  if(shift == 0){
    diplayAwalCoba();
    timeDisplay3(1,10);
    dateDisplay(15);
    bool end = runningTextkecil(texts[currentTextIndex], 3, textPoskecil, lastScroll, scrollSpeed);
    if(end){
      currentTextIndex++;
      if(currentTextIndex >= textCount){
        Serial.println("SHift 0 Sompleted");
        currentTextIndex = 0;
        textPoskecil = col;
        shift = 1;

        prayerIdx = 0;       // Reset index jadwal
        scrollx = col; // Reset posisi awal scroll
        isPausing = false;
        lastMove = 0;
        led_clear();
      }
    }
  }else if(shift == 1){
    diplayAwalCoba();
    bool end = displayPrayerTimeStatic();
    if(end){
      Serial.println("SHift 1 Sompleted");
      shift = 2;
      scrollOffset = 0;
      lastScrollTime - 0;
      currentVertIndex = 0;
      shiftLoop[2].currentLoop = 0;
      led_clear();
      // scrollx = thirdCol * 3;  
     
    }
  } else if(shift == 2){
    static unsigned long lastUpdate = 0;
    if(millis() - lastUpdate >= 20){
      lastUpdate = millis();
      if (animated1(vertText[currentVertIndex],18)){
        shiftLoop[2].currentLoop++;
      }
      if(shiftLoop[2].currentLoop >= shiftLoop[2].maxLoop){
        Serial.println("SHift 2 Sompleted");
        shiftLoop[2].currentLoop = 0;
        shiftLoop[2].completed = true;

        currentVertIndex = 0;
        scrollOffset = 0;
        currentQuoteIndex = 0;
        textPos96 = col;
        lastScroll96Pixel = 0; 

        shift = 3;
        Serial.println(textPos96);
        led_clear();
      }
    }
  }else if(shift == 3){
    if(runningQuotefullCycle >= shiftLoop[3].maxLoop){
      Serial.println("SHift 3 Sompleted");
      runningQuotefullCycle = 0;
      shiftLoop[3].completed = true;

      currentQuoteVertIndex = 0;
      scrollOffset = 0;
      lastScrollTime = 0;

      shift = 4;
      pergantianShift = currentMillis;
      led_clear();
      return;
    }
    if(currentQuoteIndex < QuoteCount){
      animated2(QuoteText[currentQuoteIndex], QuoteColor[currentQuoteIndex]);
    }  else {
      currentQuoteIndex = 0;
      animated2(QuoteText[currentQuoteIndex], QuoteColor[currentQuoteIndex]);
    }
    
  } else if(shift == 4){
    if(runningQuotefullCycle2 >= shiftLoop[4].maxLoop){
      Serial.println("SHift 4 Sompleted");
      runningQuotefullCycle2 = 0;

      currentInfoIndex1 = 0;
      infoPos1 = col;
      lastInfoScroll = 0;

      shift = 5;
      pergantianShift = currentMillis;
      led_clear();
      return;
    }
    animated3(QuoteVertText[currentQuoteVertIndex], 18);
  } else if(shift == 5){
    title("Imam Jum'at", 10);
    bool end = runningTexts(textInfo1[currentInfoIndex1], 18, infoPos1, lastInfoScroll1, scrollSpeed);
    if(end){
      currentInfoIndex1++;
      if(currentInfoIndex1 >= textInfoCount1){
        Serial.println("SHift 5 Sompleted");
        currentInfoIndex1 = 0;
        infoPos1 = col;

        currentInfoIndex = 0;
        infoPos = col;
        lastInfoScroll = 0;

        shift = 6;
        led_clear();
      }
    }
  } else if(shift == 6){
    title("Informasi!", 10);
    bool end = runningTexts(textInfo[currentInfoIndex], 18, infoPos, lastInfoScroll, scrollSpeed);
    if(end){
      currentInfoIndex++;
      if(currentInfoIndex >= textInfoCount){
        Serial.println("SHift 6 Sompleted");
        currentInfoIndex = 0;
        infoPos = col;

        currentTextIndex = 0;
        textPoskecil = col;
        scrollOffset = 0;
        animPos = -quarterRow;
        animState = dayDown;

        shift = 0;
        infoPos = col;
        led_clear();
      }
    }
  }
}

// void setupLittlesFS(){
//   if(!LittleFS.begin(true)){
//     Serial.println("LittleFS Mount Failed!");
//     Serial.println("Teks yang ditampilkan adalah default");
//     return;
//   }

//   Serial.println("LittleFS Mount Succesfully!");
//   Serial.printf("Total space : %d bytes (%.2f KB)n", LittleFS.totalBytes(), LittleFS.totalBytes()/1024.0);
//   Serial.printf("Total used : %d bytes (%.2f KB)n", LittleFS.usedBytes(), LittleFS.usedBytes()/1024.0);
//   Serial.printf("Total free : %d bytes (%.2f KB)n", LittleFS.totalBytes() - LittleFS.usedBytes(), LittleFS.totalBytes() - LittleFS.usedBytes()/1024.0);

//   listAllFiles();
//   createDefaultFiles();
// }

void ntpSetup(){
  configTime(TZ_S, 0, "pool.ntp.org", "time.google.com", "time.windows.com");
}

void waitForValidTime() {
  Serial.println(" Menunggu sinkronisasi waktu dari NTP...");
  
  int attempt = 0;
  bool timeValid = false;
  
  while(!timeValid && attempt < 30) {
    delay(1000);
    updateTime();
    
    time_t now = time(NULL);
    timeValid = (now > 1000000000) && (yr >= 2020) && (yr <= 2100);
    
    attempt++;
    Serial.printf(" Attempt %d/30 - Unix: %ld, Date: %04d-%02d-%02d\n", attempt, now, yr, month_index + 1, d);
  }
  Serial.println();
  
  if(timeValid) {
    Serial.println(" Waktu NTP valid!");
    Serial.printf(" Waktu sekarang: %s, %04d-%02d-%02d %02d:%02d:%02d\n", wd[dow], yr, month_index + 1, d, h, m, s);
  } else {
    Serial.println(" GAGAL mendapatkan waktu dari NTP!");
    Serial.println(" Perhitungan jadwal shalat tidak akan akurat!");
  }
  Serial.println();
}

void updateTime(){
  t = time(NULL);

  struct tm*tm;
  tm = localtime(&t);

  int old_day = d;
  int old_month = month_index;
  int old_yr = yr;

  h = tm->tm_hour;
  m = tm->tm_min;
  s = tm->tm_sec;
  d = tm->tm_mday;
  dow = tm->tm_wday;
  month_index = tm->tm_mon;
  yr  = tm->tm_year + 1900;
}

// bool isTimeValid(){
//   bool yrValid = (yr >= 2020 && yr <= 2100);
//   bool mothValid = (month_index >= 0 && month_index < 12);
//   bool dayValid = (d >= 0 && d <= 31);
//   bool hValid = (h >= 0 && h <= 23);
//   bool mValid = (m >= 0 &&  m <= 59);

//   bool allValid  = yrValid && mothValid && dayValid && hValid && mValid;

//   if(!allValid){
//     Serial.printf("Invalid time components: yr=%d m=%d d=%d h=%d min=%d\n", yr, month_index+1, d, h, m);
//   } else {
//     Serial.printf("Time valid: %04d-%02d-%02d %02d:%02d\n",yr, month_index+1, d, h, m);
//   }

//   return allValid;
// }

// void jadwalShalatSetup(){

//   Serial.println("Hitung jadwal Shalat dengan Library Prayer Times");
//   if(!isTimeValid()){
//     Serial.println("Waktu dari NTP  belum Valid");
//     prayer_times_valid = false;
//     return;
//   }

//   if(yr  < 2020 || yr > 2100){
//     Serial.println("Tahun tidak Valid");
//     prayer_times_valid = false;
//     return;
//   }

//   set_calc_method(Makkah);
//   set_asr_method(Shafii);
//   set_high_lats_adjust_method(AngleBased);
  
//   for(int i = 0; i < 7; i++){
//     times[i] = 0.0;
//   }

//   get_prayer_times(yr, month_index + 1, d, lat, lon, gtmOfset, times);

//   bool hasValidData = false;
//   for(int i = 0; i < 7; i++){
//     if(times[i] <= 0 || times[i] >= 24) {
//       Serial.println("Waktu tidak valid");
//     } else {
//       Serial.println("Hasil Oke");
//       hasValidData = true;
//     }
//   }
//   Serial.println();

//   if(!hasValidData){
//     Serial.println("Perhitungan jadwal shalat gagal!");
//     prayer_times_valid = false;
//     return;
//   }

//   bool valid = true;
//   Serial.println("Konversi ke JAM : MENIT");
    
//   for(int i = 0; i < 7; i++){
//     if(times[i] <= 0.0 || times[i] >= 24.0){
//       continue;
//     } 
//     int ho, mi;
//     get_float_time_parts(times[i], ho, mi);

//     if(times[i] == 0.0) {
//       Serial.println("Nilai 0");
//     } else if (ho < 0 || ho > 23 || mi < 0 || mi > 59){
//       Serial.println("Nilai gagal konversi!!");
//       valid = false;
//     } else {
//       Serial.println("OKKE!");
//     }
//   }  

//   if(!valid){
//     Serial.println("perhitungan JWS gagal! periksa Programer pusing apa tidak!");
//     prayer_times_valid = false;
//     return;
//   }

//   byte prayerIndex[] = {0,2,3,5,6};
//   int validCount = 0;

//   for(int i  = 0; i < 5; i++){
//     int index = prayerIndex[i];
//     if(times[index] > 0.0 &&  times[index] < 24.0){
//       validCount++;
//     }
//   }

//   if(validCount < 5){
//     prayer_times_valid = false;
//     return;
//   }

//   last_calculated_day = d;
//   prayer_times_valid = true;

//   for(int i  = 0; i < 5; i++){
//     byte index  = prayerIndex[i];
//     String timeStr = getTimePray(index);
//   }
// }

// void checkUpdatePrayerTimes(){
//   if(d != last_calculated_day || !prayer_times_valid){
//     jadwalShalatSetup();
//   }
// }

String getTimePray(byte Index){
  if(Index >= 7) {
    // Serial.printf("Index  invalid %d\n", Index);
    return "00:00";
  }

  if(times[Index] <= 0.0 || times[Index] >= 24.0) {
    // Serial.printf("times[%d] diluar range : %.6f\n", Index, times[Index]);
    return "00:00";
  }

  int ho, mi;
  get_float_time_parts(times[Index], ho, mi);

  if(ho < 0 || ho > 23 || mi  < 0 ||  mi > 59) {
    // Serial.printf("Konversi gagal ho = %d mi = %d\n", ho, mi);
    return "00:00";
  }

  String hS = (ho < 10) ? "0" + String(ho) : String(ho);
  String mS = (mi < 10) ? "0" + String(mi) : String(mi);

  return hS + ":" + mS;
}

void wifiBegin(){
  Serial.println("Menyambung ke internet !");
  Serial.println(ssid);

  WiFi.begin(ssid, pass);
  while (WiFi.status() != WL_CONNECTED){
    Serial.print(".");
    if(delayx(1000) == 1)return;
  }

  Serial.println(" ");
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address : ");
  Serial.println(ip);
}

void setupDisplay(){
  HUB75_I2S_CFG mxconfig(
    PANEL_RES_X,  
    PANEL_RES_Y,  
    PANEL_CHAIN    
  );

  mxconfig.i2sspeed = HUB75_I2S_CFG :: HZ_10M;
  mxconfig.double_buff = false;
  mxconfig.min_refresh_rate = 120;
  mxconfig.clkphase = false;

  mxconfig.gpio.r1 = R1; mxconfig.gpio.g1 = G1; mxconfig.gpio.b1 = BL1;
  mxconfig.gpio.r2 = R2; mxconfig.gpio.g2 = G2; mxconfig.gpio.b2 = BL2;
  mxconfig.gpio.a = CH_A; mxconfig.gpio.b = CH_B; mxconfig.gpio.c = CH_C; 
  mxconfig.gpio.d = CH_D; mxconfig.gpio.e = CH_E; 
  mxconfig.gpio.clk = CLK; 
  mxconfig.gpio.lat = LAT; 
  mxconfig.gpio.oe = OE;

  dma_display = new MatrixPanel_I2S_DMA(mxconfig);
  dma_display->begin();
  dma_display->setBrightness8(128);
  dma_display->clearScreen();
  dma_display->setTextWrap(false);
}

void led_clear(){
  dma_display->clearScreen();
}

void clearArea(int x, int y, int w, int h){
  dma_display->fillRect(x, y, w, h, dma_display->color565(0, 0, 0));
}

void printText(String text){
  dma_display->print(text);
}

void Basic_color(String Warna){
  if( Warna == "Black") dma_display->color565(0,0,0);
  if( Warna == "White") dma_display->color565(255,255,255);
  if( Warna == "Red") dma_display->color565(255,0,0);
  if( Warna == "Green") dma_display->color565(0,255,0);
  if( Warna == "Blue") dma_display->color565(0,0,255);
}

pixelColor color_select(byte index){
  pixelColor c = {0,0,0};
  if(index >= 19 ) return c;

  int base  = index*3;
  c.r = pgm_read_byte(&colorarray1[base]);
  c.g = pgm_read_byte(&colorarray1[base + 1]);
  c.b  = pgm_read_byte(&colorarray1[base + 2]);
  return  c;
}

uint16_t getColor(byte Index){
  pixelColor c = color_select(Index);
  return dma_display->color565(c.r,c.g,c.b);
} 

//yang ini fix x nya 
void timeDisplay3(byte size, byte color){ // jam di harcode di ukuran 64 * 32!
  String jam = (h < 10 ? "0" : "")+ String(h);
  String menit = (m < 10 ? "0" : "")+String(m);
  // String titikDua = (flasher ? ":" : " "); 

  layoutJam->fillScreen(0);
  layoutJam->setFont(&DS_Digital14x19);
  layoutJam->setTextSize(size);

  int16_t x1, y1;
  uint16_t w, h, wJ, hJ;

//   int16_t x2, y2;
//   uint16_t w2, h2;

  layoutJam->getTextBounds(":", 0, 0, &x1, &y1, &w, &h);
  layoutJam->getTextBounds(jam, 0, 0, &x1, &y1, &wJ, &hJ);
  // layoutJam->getTextBounds(menit, 0, 0, &x2, &y2, &w2, &h2);

  int x_titik = (64 - w)/2;
  int y_pos = (halfRow - h)/2 - 1 - y1;

  int xJam  = 8 -x1;
  int xMenit = 64 - wJ - 8 - x1;
  
  layoutJam->setTextColor(getColor(color));
  layoutJam->setCursor(xJam, y_pos);
  layoutJam->print(jam);

  layoutJam->setCursor(xMenit, y_pos);
  layoutJam->print(menit);

  layoutJam->setCursor(x_titik, y_pos);
  if(flasher){
    layoutJam->setTextColor(getColor(color));
  } else {
    layoutJam->setTextColor(0);
  }
  layoutJam->print(":");

  dma_display->drawRGBBitmap(jamXStart, 0, layoutJam->getBuffer(), 64, halfRow);
}

//display time 
// void timeDisplay3(int size, byte color){
//   String time = (h < 10 ? "0" : "")+ String(h) + (flasher ? ":" : "") + (m < 10 ? "0" : "")+String(m);
  
//   layoutJam->fillScreen(0);
//   layoutJam->setFont(&DS_Digital14x19);
//   layoutJam->setTextSize(size);

//   int16_t x1, y1;
//   uint16_t w, h;

//   layoutJam->getTextBounds(time, 0, 0, &x1, &y1, &w, &h);
  
//   int x_pos = (halfCol - w)/2;
//   int y_pos = (halfRow - h)/2 - y1;

//   layoutJam->setTextColor(getColor(color));
//   layoutJam->setCursor(x_pos, y_pos);
//   layoutJam->print(time);
//   dma_display->drawRGBBitmap(thirdCol, 0, layoutJam->getBuffer(), halfCol, halfRow);
// }

void diplayAwalCoba(){
  dma_display->drawBitmap(0, 0, epd_bitmap_1_1, 32, 32, getColor(18));
  dma_display->drawBitmap(posImgKanan, 0, epd_bitmap_2_1, 32, 32, getColor(18));  
}

// void displayPrayerTime(byte color, String judul){
//   if(currentMillis - lastPrayerScroll >= prayerScrollSpeed){
//     lastPrayerScroll = currentMillis;

//     static String CachedTitle = "";
//     static int16_t x1, y1;
//     static uint16_t w, h;
//     static int xTitle = -1;

//     if(CachedTitle != judul){
//       dma_display->setFont(&PressStart2P_Regular6pt7b);
//       dma_display->setTextSize(1);
//       dma_display->getTextBounds(judul, 0, 0, &x1, &y1, &w, &h);
//       xTitle = (col - w )/2;
//       CachedTitle = judul;
//     }
    
//     dma_display->fillRect(0, 0, col, halfRow, 0);
//     dma_display->setTextColor(getColor(10));
//     dma_display->setCursor(xTitle, 2 - y1);
//     dma_display->print(judul);

//     static String allPrayer = "";
//     static int16_t prayer_x1, prayer_y1;
//     static uint16_t prayer_w, prayer_h;
//     static unsigned long lastPrayerUpdate = 0;

//     if(currentMillis - lastPrayerScroll >= 6000 || allPrayer ==""){
//       lastPrayerUpdate = currentMillis;

//       byte PrayerIndex[] = {0, 2, 3, 5, 6};
//       const char* jadwalImam[] = {"Jum'at Legi", "Jum'at Pahing", "Jum'at Pon", "Jum'at Wage", "Jum'at Kliwon"};
//       const char* imam[] = {"Bp Prabowo", "Bp teddy", "Bp Gibran", "Bp Bahlil", "bp Yaqut"};

//       allPrayer = "";
//       for(int i  = 0; i < 5; i++){
//         allPrayer += String(jadwalImam[i]) + " " + String(imam[i]);
//         if (i < 4) allPrayer += "|";
//       }
//       dma_display->setTextSize(1);
//       dma_display->setFont(&FreeMono9pt7b);
//       dma_display->getTextBounds(allPrayer, 0, 0, &prayer_x1, &prayer_y1, &prayer_w, &prayer_h);
//     }
  
//     dma_display->fillRect(0, halfRow, col, halfRow, 0);
//     dma_display->setTextColor(getColor(color));
//     dma_display->setTextSize(1);
//     dma_display->setFont(&FreeMono9pt7b);
//     dma_display->setCursor(ScrolPlus, halfRow - prayer_y1);
//     dma_display->print(allPrayer);

//     ScrolPlus--;

//     if(ScrolPlus < -prayer_w){
//       ScrolPlus = col;
//       prayerTimeScrool++;

//       Serial.printf("prayer scroll loop : %d/%d\n", prayerTimeScrool, shiftLoop[1].maxLoop);
//     }
//   }
// }

//tampilin waktu shalat 
bool displayPrayerTimeStatic(){
  
  const char* time[] = {"4:00", "12:01", "15:23", "18:10", "19:20"};
  const char* label[] = {"SUBUH", "DZUHUR", "ASAR", "MGHRB", "ISYA"};

  if(prayerIdx < 0 || prayerIdx >= 5){
    prayerIdx = 0;
  }

  memset(midText->getBuffer(), 0, contentArea * row * 2);
  midText->setTextWrap(false);

  midText->setFont(&RobotoSlab_Bold7pt7b);
  midText->setTextSize(1);

  int16_t x1, y1;
  uint16_t wL, hL, wT, hT;

  midText->getTextBounds(label[prayerIdx], 0, 0, &x1, &y1, &wL, &hL);

  midText->setFont(&RobotoSlab_Light7pt7b);
  midText->getTextBounds(time[prayerIdx], 0, 0, &x1, &y1, &wT, &hT);

  int space = 2;
  int totalTinggi = hL + space + hT;
  int startY = (row - totalTinggi)/2;

  // int maxW = (wL > wT) ? wL : wT;
  int y_label = startY - y1;
  int y_time = startY + hL + space - y1;

  //Logika Animasi
  unsigned long currentMillis = millis();

  if(!isPausing){
    if(currentMillis - lastMove >= scrollSpeed){
      lastMove = currentMillis;
      scrollx--;

      int targetx = contentStartX + 1 + (contentArea - wL) / 2; // gunakan ini jika masih salah rumusnya 
      // int targetx = 1 + (contentArea - wL) / 2;
      if(scrollx <= targetx) {
        scrollx = targetx;
        isPausing = true;
        pauseTime = currentMillis;
      }
    }
  } else {
    if(currentMillis - pauseTime >= pauseDuration){
      if(currentMillis - lastMove >= scrollSpeed){
        lastMove = currentMillis;
        scrollx--;

        int maxW = (wL > wT) ? wL : wT;
        if(scrollx < contentStartX - maxW){
          scrollx = col;
          isPausing = false;
          prayerIdx++;

          if(prayerIdx >= 5) {
            prayerIdx = 0;
            scrollx = col;
            isPausing = false;
            return true;
          }
        }
      }
    }
  }

  int canvasX = scrollx - contentStartX - x1;

  midText->setFont(&RobotoSlab_Bold7pt7b);
  midText->setTextColor(getColor(18));
  midText->setCursor(canvasX, y_label);
  midText->print(label[prayerIdx]);

  midText->setFont(&RobotoSlab_Light7pt7b);
  midText->setTextColor(getColor(3));

  int offsetTime = (wL - wT)/2;
  int x_time = canvasX + offsetTime;

  midText->setCursor(x_time, y_time);
  midText->print(time[prayerIdx]);

  dma_display->drawRGBBitmap(contentStartX, 0, midText->getBuffer(), contentArea, row);
  return false;
}

// void displayPrayerTimeStatic(){
//   memset(fullPanel->getBuffer(), 0, col * row * 2);

//   fullPanel->setTextWrap(false);

//   byte prayerIndex[] = {0,2,3,5,6};
//   const char* label[] = {"SUBUH", "DZUHUR", "ASAR", "MAGHRIB", "ISYA"};

//   fullPanel->setFont(&Org_01);
//   fullPanel->setTextSize(1);

//   int numCol = 5;
//   int colWidth = col / 5;

//   String joiningPrayer = "";
//   String joiningTimePrayer = "";

//   for(int i = 0; i < 3; i++){
//     joiningPrayer += String(label[i]) + (i < 2 ? "  " : "");
//     joiningTimePrayer += getTimePray(prayerIndex[i]) + (i < 2 ? "  " : "");
//   }

//   String joiningPrayer1 = "";
//   String joiningTimePrayer1 = "";

//   for(int i = 3; i < 5; i++){
//     joiningPrayer1 += String(label[i]) + (i < 4 ? "  " : "");
//     joiningTimePrayer1 += getTimePray(prayerIndex[i]) + (i < 4 ? "  " : "");
//   }

//   int16_t xLabel, yLabel;
//   uint16_t wLabel, hLabel;
//   fullPanel->getTextBounds(joiningPrayer, 0, 0, &xLabel, &yLabel, &wLabel, &hLabel);

//   int16_t xTnggi, yTnggi;
//   uint16_t wTnggi, hTnggi;
//   fullPanel->getTextBounds(joiningTimePrayer, 0, 0, &xTnggi, &yTnggi, &wTnggi, &hTnggi);

//   int16_t xLabel1, yLabel1;
//   uint16_t wLabel1, hLabel1;
//   fullPanel->getTextBounds(joiningPrayer1, 0, 0, &xLabel1, &yLabel1, &wLabel1, &hLabel1);

//   int16_t xTnggi1, yTnggi1;
//   uint16_t wTnggi1, hTnggi1;
//   fullPanel->getTextBounds(joiningTimePrayer1, 0, 0, &xTnggi1, &yTnggi1, &wTnggi1, &hTnggi1);

//   int x_set_waktu = (col - wTnggi)/2 - xTnggi;
//   int x_set_label = (col - wLabel)/2 - xLabel;

//   int x_set_waktu1 = (col - wTnggi1)/2 - xTnggi1;
//   int x_set_label1 = (col - wLabel1)/2 - xLabel1;

//   int midY = quarterRow;
//   int y_set_waktu = midY + (midY - hTnggi)/2 - yTnggi;
//   int y_set_label = (midY - hLabel)/2 - yLabel;

//   int midY1 = halfRow;
//   int halfMidY1 = midY1 / 2;

//   int y_set_waktu1 = midY1 + halfMidY1 + (halfMidY1 - hTnggi1)/2 - yTnggi1;
//   int y_set_label1 = midY1 + (halfMidY1 - hLabel1)/2 - yLabel1;

//   fullPanel->setTextColor(getColor(3));
//   fullPanel->setCursor(x_set_label, y_set_label);
//   fullPanel->print(joiningPrayer);

//   fullPanel->setTextColor(getColor(3));
//   fullPanel->setCursor(x_set_label1, y_set_label1);
//   fullPanel->print(joiningPrayer1);

//   fullPanel->setTextColor(getColor(18));
//   fullPanel->setCursor(x_set_waktu, y_set_waktu);
//   fullPanel->print(joiningTimePrayer);

//   fullPanel->setTextColor(getColor(18));
//   fullPanel->setCursor(x_set_waktu1, y_set_waktu1);
//   fullPanel->print(joiningTimePrayer1);

//   dma_display->drawRGBBitmap(0, 0, fullPanel->getBuffer(), col, row);
// }

// void dateDisplay(int size, byte color){
//   layoutDate->fillScreen(0);
//   layoutDate->setFont(&Picopixel);
//   layoutDate->setTextSize(size);

//   layoutDate->setTextColor(getColor(color));
//   int16_t x1, y1;
//   uint16_t w, h;

//   unsigned long currentMillis = millis();

//   switch(animState){
//     case dayDown:
//     {
//       text = wd[dow];
//       layoutDate->getTextBounds(text, 0, 0, &x1, &y1, &w, &h);
//       int x_pos = (halfCol - w)/2;
//       int y_pos = (halfRow - h)/2 - y1;

//       if(animPos < y_pos){
//         animPos++;
//       } else {
//         animState = dayStuck;
//         diam = currentMillis;
//       }
//       layoutDate->setCursor(x_pos, animPos);
//       layoutDate->print(text);
//     }
//     break;

//     case dayStuck:
//     {
//       text = wd[dow];
//       layoutDate->getTextBounds(text, 0, 0, &x1, &y1, &w, &h);
//       int x_pos = (halfCol - w)/2;
//       int y_pos = (halfRow - h)/2 - y1;

//       layoutDate->setCursor(x_pos, y_pos);
//       layoutDate->print(text);

//       if(currentMillis - diam >= animDuration){
//         animState = dayUp;
//       }
//     }
//     break;

//     case dayUp:
//     {
//       text = wd[dow];
//       layoutDate->getTextBounds(text, 0, 0, &x1, &y1, &w, &h);
//       int x_pos = (halfCol - w)/2;

//       if(animPos > -h){
//         animPos--;
//         layoutDate->setCursor(x_pos, animPos);
//         layoutDate->print(text);
//       } else {
//         colorDate += 2;
//         if(colorDate > 18){
//           colorDate = 1;
//         }
//         animState = dateDown;
//         animPos = -halfRow;
//       }
//     }
//     break;

//     case dateDown:
//     {
//       String fullDate = String(d) + "/" + String(monthName[month_index]) + "/" + String(yr%100);

//       layoutDate->getTextBounds(fullDate, 0, 0, &x1, &y1, &w, &h);
//       int x_pos = (halfCol - w)/2;
//       int y_pos = (halfRow - h)/2 - y1;

//       if(animPos < y_pos){
//         animPos++;
//       }else {
//         animState = dateStuck;
//         diam = currentMillis;
//       }

//       layoutDate->setCursor(x_pos, animPos);
//       layoutDate->print(fullDate);
//     }
//     break;

//     case dateStuck:
//     {
//       String fullDate = String(d) + "/" + String(monthName[month_index]) + "/" + String(yr%100);
//       layoutDate->getTextBounds(fullDate, 0, 0, &x1, &y1, &w, &h);
//       int x_pos = (halfCol - w)/2;
//       int y_pos = (halfRow - h)/2 - y1;  

//       layoutDate->setCursor(x_pos, y_pos);
//       layoutDate->print(fullDate);

//       if(currentMillis - diam >= animDuration){
//         animState = dateUp;
//       }
//     }
//     break;

//     case dateUp:
//     {
//       String fullDate = String(d) + "/" + String(monthName[month_index]) + "/" + String(yr%100);
//       layoutDate->getTextBounds(fullDate, 0, 0, &x1, &y1, &w, &h);
//       int x_pos = (halfCol - w)/2;

//       if(animPos > -h){
//         animPos--;
//         layoutDate->setCursor(x_pos, animPos);
//         layoutDate->print(fullDate);
//       }else{
//         animState = dayDown;
//         colorDate++;
//         if(colorDate > 18){
//           colorDate = 1;
//         }
//         animPos = -halfRow;
//       }
//     }
//     break;
//   }
//   dma_display->drawRGBBitmap(halfCol, 0, layoutDate->getBuffer(), halfCol, halfRow);
// }

//Digunakan untuk nampilin tanggalan 
void dateDisplay(byte color){
  memset(layoutDate->getBuffer(), 0, halfCol * quarterRow * 2);
  layoutDate->setFont(&FreeSansBold5pt7b); // perlu ganti font 
  layoutDate->setTextSize(1);

  layoutDate->setTextColor(getColor(color));
  int16_t x1, y1;
  uint16_t w, h;

  unsigned long currentMillis = millis();

  switch(animState){
    case dayDown:
    {
      text = wd[dow];
      layoutDate->getTextBounds(text, 0, 0, &x1, &y1, &w, &h);
      int x_pos = (halfCol - w)/2;
      int y_pos = (quarterRow - h)/2 - y1;

      if(animPos < y_pos){
        animPos++;
      } else {
        animState = dayStuck;
        diam = currentMillis;
      }
      layoutDate->setCursor(x_pos, animPos);
      layoutDate->print(text);
    }
    break;

    case dayStuck:
    {
      text = wd[dow];
      layoutDate->getTextBounds(text, 0, 0, &x1, &y1, &w, &h);
      int x_pos = (halfCol - w)/2;
      int y_pos = (quarterRow - h)/2 - y1;

      layoutDate->setCursor(x_pos, y_pos); 
      layoutDate->print(text);

      if(currentMillis - diam >= animDuration){
        animState = dayUp;
      }
    }
    break;

    case dayUp:
    {
      text = wd[dow];
      layoutDate->getTextBounds(text, 0, 0, &x1, &y1, &w, &h);
      int x_pos = (halfCol - w)/2;

      if(animPos > -h){
        animPos--;
        layoutDate->setCursor(x_pos, animPos);
        layoutDate->print(text);
      } else {
        colorDate += 2;
        if(colorDate > 18){
          colorDate = 1;
        }
        animState = dateDown;
        animPos = -quarterRow;
      }
    }
    break;

    case dateDown:
    {
      String fullDate = String(d) + " / " + String(monthName[month_index]) + " / " + String(yr);

      layoutDate->getTextBounds(fullDate, 0, 0, &x1, &y1, &w, &h);
      int x_pos = (halfCol - w)/2;
      int y_pos = (quarterRow - h)/2 - y1;

      if(animPos < y_pos){
        animPos++;
      }else {
        animState = dateStuck;
        diam = currentMillis;
      }
      //layoutDate->setFont(&DS_DIGI4pt7b);
      layoutDate->setCursor(x_pos, animPos);
      layoutDate->print(fullDate);
    }
    break;

    case dateStuck:
    {
      String fullDate = String(d) + " / " + String(monthName[month_index]) + " / " + String(yr);
      layoutDate->getTextBounds(fullDate, 0, 0, &x1, &y1, &w, &h);
      int x_pos = (halfCol - w)/2;
      int y_pos = (quarterRow - h)/2 - y1;  

      //layoutDate->setFont(&DS_DIGI4pt7b);
      layoutDate->setCursor(x_pos, y_pos);
      layoutDate->print(fullDate);

      if(currentMillis - diam >= animDuration){
        animState = dateUp;
      }
    }
    break;

    case dateUp:
    {
      String fullDate = String(d) + " / " + String(monthName[month_index]) + " / " + String(yr);
      layoutDate->getTextBounds(fullDate, 0, 0, &x1, &y1, &w, &h);
      int x_pos = (halfCol - w)/2;

      if(animPos > -h){
        animPos--;
        //layoutDate->setFont(&DS_DIGI4pt7b);
        layoutDate->setCursor(x_pos, animPos);
        layoutDate->print(fullDate);
      }else{
        animState = dayDown;
        colorDate++;
        if(colorDate > 18){
          colorDate = 1;
        }
        animPos = -quarterRow;
      }
    }
    break;
  }
  dma_display->drawRGBBitmap(thirdCol, halfRow, layoutDate->getBuffer(), halfCol, quarterRow);
}

bool runningTextkecil(String text, byte Color, int &pos ,unsigned long &lastUpdate, int Speed){
  static String Text = "";
  static int16_t x1, y1;
  static uint16_t w, h;

  runningText8pt->setFont(&FreeSansBold5pt7b);
  runningText8pt->setTextSize(1);
  runningText8pt->setTextWrap(false);

  if(Text != text){
    runningText8pt->getTextBounds(text,  0, 0, &x1, &y1, &w, &h);
    Text = text;
  }

  unsigned long currentMillis = millis();
  if(currentMillis - lastUpdate >= Speed){
    lastUpdate = currentMillis;

    pos--;
    if(pos < -w){
      pos = col;
      return true;
    }
    int y_pos = (quarterRow - h)/2 - y1;
    int canvasX = pos - contentStartX - x1;

    memset(runningText8pt->getBuffer(), 0, contentArea * quarterRow * 2);
    runningText8pt->setTextColor(getColor(Color));
    runningText8pt->setCursor(canvasX, y_pos);
    runningText8pt->print(text);
  }
  dma_display->drawRGBBitmap(contentStartX, halfRow + quarterRow, runningText8pt->getBuffer(), contentArea, quarterRow);
  return false;
}

// void runnigText(String text, byte color){
//   static String cachedText = "";
//   static int16_t x1, y1;
//   static uint16_t w, h;

//   layoutText->setFont(&FreeMono9pt7b);
//   layoutText->setTextSize(1);
//   layoutText->setTextWrap(false);
    
//   if(cachedText != text){
//     layoutText->getTextBounds(text, 0, 0, &x1, &y1, &w, &h);
//     cachedText = text;
//   }
    
//   unsigned long currentMillis = millis();
//   if(currentMillis - lastScroll >= scrollSpeed){
//     lastScroll = currentMillis;

//     textPos--;

//     if(textPos < -w ){
//       textPos = col;
//       currentTextIndex++;
//       if(currentTextIndex >= textCount){
//         currentTextIndex = 0;
//         runningTextFullCycle++;
//       }
//     }
  
//     memset(layoutText->getBuffer(), 0, col * halfRow * 2);

//     int y_pos = (halfRow - h)/2 - y1;
//     layoutText->setTextColor(getColor(color));
//     layoutText->setCursor(textPos - x1, y_pos);
//     layoutText->print(text);
//   }
//   dma_display->drawRGBBitmap(0, halfRow, layoutText->getBuffer(), col, halfRow);
// }

bool runningTexts(String text, byte color, int &pos ,unsigned long &lastUpdate, int Speed){
  static String cachedText = "";
  static int16_t x1, y1;
  static uint16_t w, h;

  layoutText->setFont(&FreeMono9pt7b); 
  layoutText->setTextSize(1);
  layoutText->setTextWrap(false);

  if(cachedText != text){
    layoutText->getTextBounds(text, 0, 0, &x1, &y1, &w, &h);
    cachedText = text;
  }

  if(millis() - lastUpdate >= Speed){
    lastUpdate = millis();
    pos--;

    if(pos < -w){
      pos = col;
      return true;
    }
    memset(layoutText->getBuffer(), 0, col * halfRow * 2);
    int y_pos = (halfRow - h)/2 - y1;
    layoutText->setTextColor(getColor(color));
    layoutText->setCursor(pos - x1, y_pos);
    layoutText->print(text);
  }
  dma_display->drawRGBBitmap(0, halfRow, layoutText->getBuffer(), col, halfRow);
  return false;
}

void title(String title, byte color){
  static String CachedTitle = "";
  static int16_t x1, y1;
  static uint16_t w, h;
  static int xTitle = -1;

  if(CachedTitle != title){
    dma_display->setFont(&FreeSans6pt7b);
    dma_display->setTextSize(1);
    dma_display->getTextBounds(title, 0, 0, &x1, &y1, &w, &h);
    xTitle = (col - w )/2;
    CachedTitle = title;
  }
  
  dma_display->setTextColor(getColor(color));
  dma_display->setCursor(xTitle, 2 - y1);
  dma_display->print(title);
}

// void startWithLittlesFS(String yourFile, int gifHigh, int gifWidth, int totalFramesOnFile, int d){ 
//   if(currentOpenFile != yourFile){
//     if(file) file.close();
//     file = LittleFS.open( yourFile, "r");
//     if(!file) {
//       Serial.println("Gagal buka filenya!");
//       return;
//     }
//     currentOpenFile = yourFile;
//     currentFrame = 0;
//   }

//   const int high = gifHigh;
//   const int width = gifWidth;
//   const int bytePerFrame = high * width * 2;
//   unsigned long offset = (unsigned long)currentFrame * bytePerFrame;

//   if(!file.seek(offset)){
//     file.seek(0);
//     currentFrame = 0;
//   }

//   uint16_t rowBuffer[width];
  
//   for(int y = 0; y < high; y++){
//     if(file.read((uint8_t*)rowBuffer, gifWidth * 2)){
//       for(int x = 0; x < width; x++){
//         uint16_t pixel = rowBuffer[x];
//         pixel = (pixel >> 8) | (pixel << 8);

//         for(int x_offset = 0; x_offset < col; x_offset += width){
//           int target_x = x + x_offset;
//           if(target_x < col){
//             dma_display->drawPixel(target_x, y, pixel);
//           }
//         }
//       }
//     }
//   }

//   currentFrame++;
//   if(currentFrame >= totalFramesOnFile){
//     currentFrame = 0;
//   }
//   if(delayx(d) == 1)return;
// }

void animated3(String text, byte color){
  static bool started = false;
  static bool completed = false;

  dma_display->drawBitmap(0, 0, epd_bitmap_3_1, 32, 32, getColor(18));
  kata_kata_gufron(text, color);

  if(scrollOffset == 0){
    started = true;
  }
  
  if(started && scrollOffset == 0){
    completed = true;
  }

  if(completed){
    started = false;
    completed = false;
  }
}

void kata_kata_gufron(String text, byte color){

  memset(QuoteLayout2->getBuffer(), 0, runTeks96 * row * 2);

  QuoteLayout2->setFont(&FreeSans9pt7b);
  QuoteLayout2->setTextSize(1);
  QuoteLayout2->setTextWrap(false);

  String line[10];
  int lineCount = 0;
  int maxWidth = runTeks96 - 4;
  int startPos = 0;
  int textLen = text.length();
  String currentLine = "";

  for(int i = 0; i <= textLen; i++){
    if(i == textLen || text.charAt(i) == ' '){
      String word = text.substring(startPos, i);
      String testLine = currentLine;
      if(testLine.length() > 0) testLine += " "; 
      testLine += word;

      int16_t x1, y1;
      uint16_t w, h;

      QuoteLayout2->getTextBounds(testLine, 0, 0, &x1, &y1, &w, &h);

      if(w > maxWidth && currentLine.length() > 0){
        line[lineCount++] = currentLine;
        currentLine = word;
      } else {
        currentLine = testLine;
      }
      startPos = 1 + i;
    }
  }
  if(currentLine.length() >  0 ){
    line[lineCount++] = currentLine;
  }

  int16_t x1, y1;
  uint16_t w, h;

  QuoteLayout2->getTextBounds("A", 0, 0, &x1, &y1, &w, &h);
  int tinggi = h;
  int space = 4;

  int totalTinggi = (lineCount * tinggi) + ((lineCount - 1) * space);

  unsigned long currentTime = millis();
  if(currentTime - lastScrollTime >= Speed){
    scrollOffset++;
    lastScrollTime = currentTime;

    if(scrollOffset > totalTinggi + (row + 12)){
      currentQuoteVertIndex++;
      if(currentQuoteVertIndex >= QuoteVertCount){
        currentQuoteVertIndex = 0;
        runningQuotefullCycle2++;
      }
      scrollOffset = 0;
    }
  }

  QuoteLayout2->setTextColor(getColor(color));
  for(int i = 0; i < lineCount; i++){
    QuoteLayout2->getTextBounds(line[i], 0, 0, &x1, &y1, &w, &h);
    int x_pos = (runTeks96 - w)/2 - x1;
    int y_pos = row - scrollOffset + (i * (tinggi + space)) + h - y1;

    if(y_pos > -tinggi && y_pos < row + tinggi){
      QuoteLayout2->setCursor(x_pos, y_pos);
      QuoteLayout2->print(line[i]);
    }
  }
  dma_display->drawRGBBitmap(contentStartX, 0, QuoteLayout2->getBuffer(), runTeks96, row); //runTeks96
}

void animated2(String text, byte color){
  dma_display->drawBitmap(0, 0, epd_bitmap_4_1, 32, 32, getColor(18));
  kata_kata_ustad(text, color);
}

void kata_kata_ustad(String text, byte color){
  static String cachedText = "";
  static int16_t x1, y1;
  static uint16_t w, h;
  // static bool firstRun = true;

  if(cachedText = "" || cachedText != text){
    QuoteLayout->setFont(&FreeSansBold9pt7b);
    QuoteLayout->setTextSize(2);
    QuoteLayout->getTextBounds(text, 0, 0, &x1, &y1, &w, &h);
    cachedText = text;

    // textPos96 = thirdCol * 3;
    // firstRun = false;
  }
  
  unsigned long currentMillis = millis();
  if(currentMillis - lastScroll96Pixel >= scrollSpeed){
    lastScroll96Pixel = currentMillis;

    textPos96--;
    Serial.println(textPos96);

    if(textPos96 < contentStartX - w ){
      textPos96 = col;
      Serial.println(textPos96);
      currentQuoteIndex++;

      if(currentQuoteIndex >= QuoteCount){
        currentQuoteIndex = 0;
        runningQuotefullCycle++;
      }
      cachedText = "";
    }
    memset(QuoteLayout->getBuffer(), 0, runTeks96 * row * 2);

    int y_pos = (row - h)/2 - y1;
    int canvasX = textPos96 - contentStartX - x1;

    QuoteLayout->setTextWrap(false);
    QuoteLayout->setTextColor(getColor(color));
    QuoteLayout->setCursor(canvasX, y_pos);
    QuoteLayout->print(text);
  }
  dma_display->drawRGBBitmap(contentStartX, 0, QuoteLayout->getBuffer(), runTeks96, row); // coba dulu kalau gagal balikin thridCol * 3
}

bool animated1(String text, byte Color){
  static bool started = false;
  static bool completed = false;

  dma_display->drawBitmap(0, 0, epd_bitmap_1_1, 32, 32, getColor(18));
  dma_display->drawBitmap(posImgKanan, 0, epd_bitmap_2_1, 32, 32, getColor(18));

  midtext(text, Color);
  
  if(scrollOffset > 0){
    started = true;
  }

  if(started && scrollOffset == 0){
    completed = true;
  }

  if(completed){
    bool result = true;
    started = false;
    completed = false;
    return result;
  }
  return false;
}

void midtext(String text, byte Color){
  memset(midText->getBuffer(), 0, contentArea * row * 2);

  midText->setTextSize(1);
  midText->setFont(&FreeSansBold5pt7b);
  midText->setTextWrap(false);

  String lines[10];
  int lineCount = 0;
  int maxWidth = contentArea - 4;

  int startPos = 0;
  int textLen = text.length();
  String currentLine = "";

  for(int i = 0; i <= textLen; i++){
    if(i == textLen || text.charAt(i) == ' '){
      String word = text.substring(startPos, i);
      String testLine = currentLine;
      if(testLine.length() > 0) testLine += " ";
      testLine += word;

      int16_t x1, y1;
      uint16_t w, h;
      midText->getTextBounds(testLine, 0, 0, &x1, &y1, &w, &h);

      if(w > maxWidth && currentLine.length() > 0){
        lines[lineCount++] = currentLine;
        currentLine = word;
      } else {
        currentLine = testLine;
      }

      startPos = i + 1;
    }
  }

  if(currentLine.length() > 0){
    lines[lineCount++] = currentLine;
  }

  int16_t x1, y1;
  uint16_t w, h;
  midText->getTextBounds("A", 0, 0, &x1, &y1, &w, &h);
  int lineHeight = h;

  int space = 4;
  int totalHeight = (lineCount * lineHeight) + ((lineCount - 1) * space);

  unsigned long currentTime = millis();
  if(currentTime - lastScrollTime >= Speed){
    scrollOffset++;
    lastScrollTime = currentTime;

    if(scrollOffset > totalHeight + (row + 12)){
      currentVertIndex++;
      if(currentVertIndex >= vertTextCount){
        currentVertIndex = 0;
      }
      scrollOffset = 0;
    }
  }

  midText->setTextColor(getColor(Color));

  for(int i = 0; i < lineCount; i++){
    int16_t x1, y1;
    uint16_t w, h;
    midText->getTextBounds(lines[i], 0, 0, &x1, &y1, &w, &h);

    int y_pos = row - scrollOffset + (i * (lineHeight + space)) + h - y1;
    int x_pos = (contentArea - w)/2 - x1;

    if(y_pos > -lineHeight && y_pos < row + lineHeight){
      midText->setCursor(x_pos, y_pos);
      midText->print(lines[i]);
    }
  }
  dma_display->drawRGBBitmap(contentStartX, 0, midText->getBuffer(), contentArea, row);
}

void invertBitMap(const unsigned char* input, unsigned char* output, int size){
  for(int i = 0; i  < size ; i++){
    output[i] = ~pgm_read_byte(&input[i]);
  }
}

int delayx(int d){
  unsigned long start = millis();
  while(millis() - start < d){
    if(serial_check() == 1)return 1;
    yield();
  }
  return 0;
}

int serial_check(){
  if(Serial.available() > 0){
    return 1;
  } else {
    return 0;
  }
}

void led_set_pixel(int x, int y, byte r, byte g, byte b){
  dma_display->drawPixel(x, y, dma_display->color565(r, g, b));
}

int pixel_mirror_x(int x){
  return (col - 1) - x;
}

int pixel_mirror_y(int y){
  return (row - 1) - y;
}

void led_set(int x, int y, byte cl, int br){
  if(x < 0 || x >= col || y  < 0 || y >= row) return;

  applyWiring(x, y);
  
  pixelColor c = color_select(cl);
  led_set_pixel(x, y, c.r, c.g, c.b);
}

void applyWiring(int &x, int &y){
  if(led_wiring == 1){
  }else if(led_wiring == 2){
    x = pixel_mirror_x(x);
  }else if(led_wiring == 3){
    y = pixel_mirror_y(y);
  }else if(led_wiring == 4){
    x = pixel_mirror_x(x);
    y = pixel_mirror_y(y);
  }
}

void color_brightness(pixelColor &c, byte br){
  c.r = (c.r * br)/255;
  c.g = (c.g * br)/255;
  c.b = (c.b * br)/255;
}