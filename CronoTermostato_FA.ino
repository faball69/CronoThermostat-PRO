/*
 * programma CronoTermostato_FA
 * by Fabrizio Allevi
 * fabrizio . allevi @ tiscali . it
 */
#include <LiquidCrystal.h>
#include <Wire.h>
#include "Adafruit_MCP9808.h"
#include "Sodaq_DS3231.h"

//#define DEBUG true
#define DEBUG false

// initialize the library with the numbers of the interface pins (4-5-6-7-8-9-10dimmer)
LiquidCrystal lcd(8, 9, 4, 5, 6, 7);
Adafruit_MCP9808 tempsensor = Adafruit_MCP9808();

// menu principali
#define MAX_MENU 7
String sMenu[MAX_MENU] = {"                ",
                          "Programs        ",
                          "Days            ",
                          "Date   &   Time ",
                          "OfsTemp         ",
                          "Force           ",
                          "SwitchPlant     "};
#define LEVEL_ZERO 0
#define LEVEL_PROGRAMS 1
#define LEVEL_DAYS 2
#define LEVEL_DATETIME 3
#define LEVEL_OFSTEMP 4
#define LEVEL_FORCE 5
#define LEVEL_OFF 6
int mLevel = LEVEL_ZERO;
// sottomenu programmi
#define MAX_PROGS 4
String sPrograms[MAX_PROGS] = {"AllDay          ",
              							   "Morning+Evening ",
              							   "Early+Evening   ",
              							   "maiNtenance     " };
char cProgram[MAX_PROGS] = { 'A', 'M', 'E', 'N' };

int smLevel = 0;
// giorni settimana
#define MAX_DAYS 7
String sDays[MAX_DAYS] = { "Dom", "Lun", "Mar", "Mer", "Gio", "Ven", "Sab" };
String sMonth[12] = { "Gen", "Feb", "Mar", "Apr", "Mag", "Giu", "Lug", "Ago", "Set", "Ott", "Nov", "Dic" };

#define MAX_SEQ 6
////////////////////////////////////////////////////////////////////////////////
#define EEPROM_OFSP 0
// crono programmi
struct stProgram {
	int T[MAX_SEQ];  // in decimi di grado
	int HM[MAX_SEQ]; // hhmm nei 16bit hh=partealta mm=partebassa
} progs[MAX_PROGS] = {{{ 210, 195, 210, 195, 210, 195 }, { 0x071E, 0x091E, 0x0B00, 0x0D00, 0x0F00, 0x1700 }},
                      {{ 210, 195, 210, 195, 195, 195 }, { 0x071E, 0x091E, 0x0F00, 0x1700, 0x0000, 0x0000 }},
                      {{ 210, 195, 210, 195, 195, 195 }, { 0x051E, 0x0800, 0x0F00, 0x1700, 0x0000, 0x0000 }},
                      {{ 170, 170, 170, 170, 170, 170 }, { 0x0000, 0x173B, 0x0000, 0x0000, 0x0000, 0x0000 }}};
////////////////////////////////////////////////////////////////////////////////
#define EEPROM_OFSD (sizeof(progs))
// assegnazione programmi/giorni
int dayProg[MAX_DAYS] = { 0, 2, 2, 2, 2, 2, 1 };
////////////////////////////////////////////////////////////////////////////////
#define EEPROM_OFSF (sizeof(progs)+sizeof(dayProg))
// forzatura programmi a tempo e offset temperatura
bool bForce=false;
struct stForce {
	int hForce;
	unsigned long tFin;
	int ofsTemp;
  bool bOFF;
} forceData = { 0, 0, 0, false};
////////////////////////////////////////////////////////////////////////////////
int smValue = 0;
bool bProgram = false;
bool bFire = false;
int nProg = 0;
int arrayDT[5];
void showDateTimePage(bool bGet = false);
int smCursor = 0;

// define the button
#define btnRIGHT  0
#define btnUP     1
#define btnDOWN   2
#define btnLEFT   3
#define btnSELECT 4
#define btnNONE   5
int lastBtn = btnNONE;

//read the button value
int read_buttons() {
	int adc_key_in = analogRead(0);          // read analog A0 value
	if (adc_key_in > 1000)
		return btnNONE;
	if (adc_key_in < 50)
		return btnRIGHT;
	if (adc_key_in < 250)
		return btnUP;
	if (adc_key_in < 450)
		return btnDOWN;
	if (adc_key_in < 650)
		return btnLEFT;
	if (adc_key_in < 850)
		return btnSELECT;
	return btnNONE;
}

byte cFire = 1;
byte fire[8] = { B00100, B00100, B01000, B10010, B10101, B10001, B01110, };
byte cNoFire = 2;
byte noFire[8] = { B00000, B00000, B00000, B00000, B00000, B10101, B11111, };

#define CALDAIA 11 // digital pin out 11
#define DIMMER 10 // pin 10 dimmer lcd
#define D_LOW 50 //50
#define D_MIDDLE 150  //100
#define D_HIGH 250  // 200

void setup() {
	if (DEBUG) {
		Serial.begin(57600);
		Serial.println("Cronotermostato Debug:");
	}
	// wire
	Wire.begin();
	pinMode(CALDAIA, OUTPUT);
	digitalWrite(CALDAIA, HIGH);  // spengo rele
	// RTC DS3231
	rtc.begin();
	// MCP9808
	if (!tempsensor.begin(0x18) && DEBUG)
		Serial.println("Couldn't find MCP9808! Check your connections and verify the address is correct.");
	tempsensor.setResolution(3); // sets the resolution 0-0.5°C-30ms | 1-0.25°C-65ms | 2-0.125°C-130ms | 3-0.0625°C-250ms
	// LCD
	lcd.begin(16, 2); // set up the LCD's number of columns and rows:
	lcd.createChar(cFire, fire);
	lcd.createChar(cNoFire, noFire);
  // force data for update init progs in flash
  //saveData((byte*) progs, sizeof(progs), EEPROM_OFSP);
  //saveData((byte*) dayProg, sizeof(dayProg), EEPROM_OFSD);
  //saveData((byte*) &forceData, sizeof(forceData), EEPROM_OFSF);
	// load from eeprom data
	loadData((byte*) progs, sizeof(progs), EEPROM_OFSP);
	loadData((byte*) dayProg, sizeof(dayProg), EEPROM_OFSD);
	loadData((byte*) &forceData, sizeof(forceData), EEPROM_OFSF);
	//showMainPage(readTemp());
	analogWrite(DIMMER, D_LOW);
	idlePage(readTemp(), true);
}

unsigned long msIdle = millis();
unsigned long msTemp = millis();
unsigned long msScheduler = millis();
unsigned long msLast = millis();
unsigned long msDimmer = millis();
bool bIdleState = true;
float lastTemp;

/**************************************************************/
void loop() {
	unsigned long msNow = millis();
	if (msNow < msLast) {  // gestione overflow
		msLast = msNow;
		if (msDimmer)
			msDimmer = msNow;
		if (msScheduler > msNow)
			msScheduler = msNow;
		if (msIdle > msNow)
			msIdle = msNow;
		if (msTemp > msNow)
			msTemp = msNow;
	}
	int newBtn = read_buttons();
	if (newBtn != btnNONE) {
		if (lastBtn == btnNONE) {
			if (DEBUG)
				Serial.println(newBtn);
			lastBtn = newBtn;
			//lcd.display();
			analogWrite(DIMMER, D_HIGH);
			handleMenus();  // gestione menus
			msDimmer = millis() + 5000;
			bIdleState = false;
		}
	} else {
		lastBtn = btnNONE;
		if (msDimmer) {
			if (msNow >= msDimmer + 5000) {
				analogWrite(DIMMER, D_LOW);
				if (!bProgram) {
					//showMainPage(readTemp());
					mLevel = LEVEL_ZERO;
					smLevel = 0;
					smValue = 0;
					msDimmer = 0;
					bIdleState = true;
					idlePage(readTemp(), true);
				}
			} else if (msNow >= msDimmer)
				analogWrite(DIMMER, D_MIDDLE);
		}
	}
	if (bForce || (msNow > msScheduler + 60000)) { // ogni 60sec
		float t = readTemp();
		scheduler(t);
		handleCaldaia();
		msScheduler = msNow;
    bForce=false;
	}
	if (msNow > msTemp + 12000) { // ogni 12sec
		lastTemp = readTemp();
		msTemp = msNow;
	}
	if (bIdleState) {
		if (msNow > msIdle + 1000) { // ogni 1sec
			idlePage(lastTemp, false);
			msIdle = msNow;
		}
	}
	msLast = msNow;
}
/*
 * Buttons:
 *    #up/page
 * #sx/- #dx/+      #enter
 *    #down/page
 */
