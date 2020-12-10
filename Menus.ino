/*
 * gestione menus per programma CronoTermostato_FA
 * by Fabrizio Allevi
 * fabrizio . allevi @ tiscali . it
 */
 void changeText(int step, int max, int col, int row, int *idx, String *arr) {
	 if(step>0) {
		 if(*idx==max-1)
			 *idx=-1;
	 }
	 else {
		 if(*idx==0)
			 *idx=max;
	 }
	 *idx+=step;
	 lcd.setCursor(col,row);
	 lcd.print(arr[*idx]);
	 if(DEBUG) {
		 Serial.print("*idx=");
		 Serial.println(*idx);
	 }
 }

 void handleMenus() {
	 int dir=1;
   if(bIdleState) {
     showInfo();
     return;
   }
	 if(lastBtn==btnLEFT || lastBtn==btnRIGHT) {
		 if(lastBtn==btnLEFT)
			 dir=-1;
		 if(bProgram) {
			 if(mLevel==LEVEL_DATETIME) {
				 if(smLevel==4 && dir==1)
					 smLevel=-1;
				 else if(smLevel==0 && dir==-1)
					 smLevel=5;
				 smLevel+=dir;
				 if(smLevel<3)
					 lcd.setCursor(smLevel*3,1);
				 else
					 lcd.setCursor(2+smLevel*3,1);
			 }
			 else if(mLevel==LEVEL_PROGRAMS) {
				 if(smCursor==2 && dir==1) {
					 smCursor=-1;
					 if(smValue==5)
						 smValue=-1;
					 smValue+=dir;
					 printProgEvent();
				 }
				 else if(smCursor==0 && dir==-1) {
					 smCursor=3;
					 if(smValue==0)
						 smValue=MAX_SEQ;
					 smValue+=dir;
					 printProgEvent();
				 }
				 smCursor+=dir;
				 if(smCursor<2)
					 lcd.setCursor(smCursor*3,1);
				 else
					 lcd.setCursor(11,1);
			 }
		 }
		 else {
			 if(mLevel==LEVEL_PROGRAMS) {
					 changeText(dir,MAX_PROGS,0,1, &smLevel, sPrograms);
			 }
			 else if(mLevel==LEVEL_DAYS) {
				 changeText(dir,MAX_DAYS,12,0, &smLevel, sDays);
				 lcd.setCursor(0,1);
				 lcd.print(sPrograms[dayProg[smLevel]]);
			 }
		 }
	 }
	 else if(lastBtn==btnUP || lastBtn==btnDOWN) {
		 if(bProgram) {
      if(mLevel==LEVEL_OFF) {
        forceData.bOFF=!forceData.bOFF;
        updateOFF();
      }
      else if(mLevel==LEVEL_FORCE) {
				 if(lastBtn==btnDOWN)
					 dir=-1;
				 if((forceData.hForce==4 && dir==1) || (forceData.hForce==-4 && dir==-1))
					 forceData.hForce=dir*24;
				 else if((forceData.hForce==-24 && dir==1) || (forceData.hForce==24 && dir==-1))
					 forceData.hForce=dir*4*-1;
				 else if(forceData.hForce>4 || forceData.hForce<-4)
					 forceData.hForce+=dir*24;
				 else
					 forceData.hForce+=dir;
				 updateForce();
			 }
       else if(mLevel==LEVEL_OFSTEMP) {
				 if(lastBtn==btnDOWN)
					 dir=-1;
				 forceData.ofsTemp+=dir;
         updateOfs();
			 }
			 else if(mLevel==LEVEL_DATETIME) {
				 if(lastBtn==btnDOWN)
					 dir=-1;
				 arrayDT[smLevel]+=dir;
				 showDateTimePage();
			 }
			 else if(mLevel==LEVEL_DAYS) {
				 if(lastBtn==btnUP)
					 dir=-1;
				 changeText(dir,MAX_PROGS,0,1, &smValue, sPrograms);
			 }
			 else if(mLevel==LEVEL_PROGRAMS) {
				 if(lastBtn==btnDOWN)
					 dir=-1;
         int hour=progs[smLevel].HM[smValue]>>8;
         int min=progs[smLevel].HM[smValue]&0xFF;  
         if(smCursor==2) { //temp
           progs[smLevel].T[smValue]+=5*dir;
         }
         else {
           if(smCursor==0) {  //hour
             hour+=dir;
             if(hour<0)
              hour=23;
             hour%=24;
           }
           else if(smCursor==1) { //min
             min+=dir*15;
             if(min<0)
              min=45;
             min%=60;
           }
           progs[smLevel].HM[smValue]=(hour<<8)|(min&0xFF);
         }
				 printProgEvent();
			 }
		 }
		 else {
			 lcd.clear();
       if(lastBtn==btnUP)
				 dir=-1;
			 changeText(dir,MAX_MENU,0,0, &mLevel, sMenu);
			 lcd.setCursor(0,1);
       if(mLevel==LEVEL_OFF)
         updateOFF();
       else if(mLevel==LEVEL_FORCE)
         updateForce();
       else if(mLevel==LEVEL_OFSTEMP)
				 updateOfs();
			 else if(mLevel==LEVEL_DATETIME)
				 showDateTimePage(true);
			 else if(mLevel==LEVEL_DAYS) {
				 lcd.print(sPrograms[dayProg[0]]);
				 lcd.setCursor(12,0);
				 lcd.print(sDays[0]);
			 }
			 else if(mLevel==LEVEL_PROGRAMS)
				 lcd.print(sPrograms[0]);
			 else if(mLevel==LEVEL_ZERO)
				 showInfo();
			 smLevel=0;
		 }
	 }
	 else if(lastBtn==btnSELECT) {
		 if(mLevel!=LEVEL_ZERO) {
       if(!bProgram) {
  			 if(mLevel==LEVEL_PROGRAMS) {
  				 smValue=0;
  				 smCursor=0;
  				 printProgEvent();
  			 }
  			 else if(mLevel==LEVEL_DAYS) {
  				 //...
  			 }
  			 else if(mLevel==LEVEL_DATETIME) {
  				 smLevel=0;
  			 }
         else if(mLevel==LEVEL_OFSTEMP) {
  				 //...
  			 }
         else if(mLevel==LEVEL_FORCE) {
           //...
         }
         else if(mLevel==LEVEL_OFF) {
           //...
         }
  			 lcd.setCursor(0,1);
  			 lcd.blink();
  			 bProgram=true;
  		 }
  		 else {
  			 if(mLevel==LEVEL_PROGRAMS) {
  				 smValue=smCursor=0;
  				 mLevel=LEVEL_ZERO;
  				 changeText(1,MAX_MENU,0,0, &mLevel, sMenu);
  				 lcd.setCursor(0,1);
  				 lcd.print(sPrograms[smLevel]);
           saveData((byte*)progs, sizeof(progs), EEPROM_OFSP);
  			 }
  			 else if(mLevel==LEVEL_DAYS) {
  				 dayProg[smLevel]=smValue;
           saveData((byte*)dayProg, sizeof(dayProg), EEPROM_OFSD);
         }
         else if(mLevel==LEVEL_DATETIME)
  				 adjustDateTime();
         else if(mLevel==LEVEL_OFSTEMP) {
           saveData((byte*)&forceData, sizeof(forceData), EEPROM_OFSF);
         }
  			 else if(mLevel==LEVEL_FORCE) {
  				 // calcolo dell'endPoint
  				 if(DEBUG)
  					 Serial.println("setForce");
           int hf=forceData.hForce;
           if(hf==0) {
             forceData.tFin=0;
           }
           else {
             if(hf<0)
               hf*=-1; // date always in the future :)
             DateTime now = rtc.now(); //get the current date-time
    				 unsigned long nt=now.get();
    				 nt+=((unsigned long)hf*3600);
    				 if(DEBUG)
    					 Serial.println(nt);
    				 DateTime tFin(nt);
    				 if(DEBUG) {
    					 char txt[17];
    					 sprintf(txt, "%02d/%02d/%4d %02d:%02d", tFin.date(), tFin.month(), tFin.year(), tFin.hour(), tFin.minute());
    					 Serial.println(txt);
    					 Serial.println("endSetForce");
    				 }
    				 forceData.tFin=tFin.get();
           }
  				 saveData((byte*)&forceData, sizeof(forceData), EEPROM_OFSF);
  			 }
         else if(mLevel==LEVEL_OFF) {
           saveData((byte*)&forceData, sizeof(forceData), EEPROM_OFSF);
         }
  			 bProgram=false;
         bForce=true;
  			 lcd.noBlink();
  		 }
     }
	 }
 }

 void printProgEvent() {
	 lcd.setCursor(0,0);
	 lcd.print(sPrograms[smLevel]);
	 lcd.setCursor(13,0);
	 lcd.print("["+String(smValue)+"]");
	 lcd.setCursor(0,1);
	 char sTemp[5];
   float fTemp=progs[smLevel].T[smValue];
   fTemp/=10.0;
	 dtostrf(fTemp, 2, 1, sTemp);
	 char txt[17];
	 sprintf(txt, "%02d:%02d  ->  %s\xdf", progs[smLevel].HM[smValue]>>8, progs[smLevel].HM[smValue]&0xFF, sTemp);
	 lcd.print(txt);
 }

 void updateForce() {
   lcd.setCursor(0,1);
   if(forceData.hForce>23 || forceData.hForce<-23) {
     lcd.print(forceData.hForce/24);
     lcd.print(" day  ");
   }
   else {
     lcd.print(forceData.hForce);
     lcd.print(" hour ");
   }
   lcd.setCursor(0,1);
 }

 void updateOFF() {
   lcd.setCursor(0,1);
   if(forceData.bOFF) {
     lcd.print(" OFF  ");
   }
   else {
     lcd.print(" ON ");
   }
   lcd.setCursor(0,1);
 }

 void updateOfs() {
	 lcd.setCursor(0,1);
   char sTemp[5];
   float fTemp=forceData.ofsTemp;
   fTemp/=10.0;
	 dtostrf(fTemp, 2, 1, sTemp);
	 lcd.print(sTemp);
	 lcd.print("\xdf    ");
	 lcd.setCursor(0,1);
 }

 void showOnlyTemp(float t) {
	 lcd.clear();
	 char sTemp[6];
	 dtostrf(t, 2, 1, sTemp);
	 lcd.setCursor(6, 1);
	 sTemp[4]='\xdf';
	 sTemp[5]='\x0';
	 lcd.print(sTemp);
   lcd.setCursor(14, 1);
	 if(bFire)
		 lcd.write(byte(cFire));
   else
     lcd.write(byte(cNoFire));
 }

 void showInfo() {
	 char txt[17];
	 DateTime now = rtc.now(); //get the current date-time
	 lcd.clear();
	 lcd.setCursor(0, 0);
	 sprintf(txt, "%s %02d %s %02d:%02d", sDays[now.dayOfWeek()].c_str(), now.date(), sMonth[now.month()-1].c_str(), now.hour(), now.minute());
	 lcd.print(txt);
	 lcd.setCursor(0, 1);
	 char sTemp[6];
	 dtostrf(readTemp(), 2, 1, sTemp);
   sTemp[5]='\x0';
   char c=cNoFire; // no flame
   if(bFire)
     c=cFire;
	 sprintf(txt, "[%c]   %s\xdf  %c%c%c", cProgram[dayProg[now.dayOfWeek()]], sTemp, c,c,c);
   lcd.print(txt);
 }

 void showDateTimePage(bool bGet/*=false*/) {
	 char txt[17];
	 if(bGet) {
		 DateTime now=rtc.now(); //get the current date-time
		 arrayDT[0]=now.date();
		 arrayDT[1]=now.month();
		 arrayDT[2]=now.year();
		 arrayDT[3]=now.hour();
		 arrayDT[4]=now.minute();
	 }
	 lcd.setCursor(0, 1);
	 sprintf(txt, "%02d/%02d/%4d %02d:%02d", arrayDT[0], arrayDT[1], arrayDT[2], arrayDT[3], arrayDT[4]);
	 lcd.print(txt);
	 if(DEBUG)
		 Serial.println(txt);
 }

 void adjustDateTime() {
	 if(DEBUG) {
		 Serial.print(arrayDT[0]);
		 Serial.print(arrayDT[1]);
		 Serial.print(arrayDT[2]);
		 Serial.print(arrayDT[3]);
		 Serial.println(arrayDT[4]);
	 }
	 DateTime dtAdjust(arrayDT[2], arrayDT[1], arrayDT[0], arrayDT[3], arrayDT[4], 0);
	 rtc.setDateTime(dtAdjust);
 }

int pointLCD=0;
int lineLCD=0;
int ds=1;
 void idlePage(float t, bool bReset) {
   if(pointLCD==12 && lineLCD==0 && ds==1) {
     pointLCD=11;
     lineLCD=1;
     ds=-1;
     lcd.clear();
   }
   else if(bReset || (pointLCD==-1 && lineLCD==1 && ds==-1)) {
     pointLCD=0;
     lineLCD=0;
     ds=1;
     lcd.clear();
   }
   else if(ds==1) {
     lcd.scrollDisplayRight();
     pointLCD+=ds;
     return;
   }
   else if(ds==-1) {
     lcd.scrollDisplayLeft();
     pointLCD+=ds;
     return;
   }
   // temp
   char sTemp[6];
   dtostrf(t, 2, 1, sTemp);
   sTemp[4]='\xdf';
   sTemp[5]='\x0';
   lcd.setCursor(pointLCD, lineLCD);
   lcd.print(sTemp);
   // flame
   char sFlame[5];
   if(bFire) {
     sFlame[0]=sFlame[1]=sFlame[2]=sFlame[3]=cFire;
     sFlame[4]='\x0';
   }
   else {
     sFlame[0]=sFlame[1]=sFlame[2]=sFlame[3]=cNoFire;
     sFlame[4]='\x0';
   }
   lcd.setCursor(pointLCD, (lineLCD+1)%2);
   lcd.print(sFlame);
   //lcd.write(byte(cNoFire));
   pointLCD+=ds;
 }
