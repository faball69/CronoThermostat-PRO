/*
 * scheduler per programma CronoTermostato_FA
 * by Fabrizio Allevi
 * fabrizio . allevi @ tiscali . it
 */

 void handleCaldaia() {
	 if(bFire && !forceData.bOFF)
		 digitalWrite(CALDAIA, LOW);
	 else
		 digitalWrite(CALDAIA, HIGH);
 }

 float readTemp() {
	 tempsensor.wake();   // wake up, ready to read!
	 float c=tempsensor.readTempC();
   float o=forceData.ofsTemp;
   o/=10;
   o+=c;
	 if(DEBUG) {
		 Serial.print("TempRead: ");
     Serial.print(c, 4);
		 Serial.print("°C - TempOfs: ");
     Serial.print(o, 4);
		 Serial.println("°C");
	 }
	 tempsensor.shutdown_wake(1); // shutdown MSP9808 - power consumption ~0.1 mikro Ampere, stops temperature sampling
	 return o;
 }

void scheduler(float tempNow) {
	DateTime now = rtc.now(); //get the current date-time
  int hh=now.hour();
	int minuteNow=hh*60+now.minute();
	int idProg=dayProg[now.dayOfWeek()];
	if(DEBUG) {
		Serial.print("minuteNow=");
		Serial.print(minuteNow);
		Serial.print(" idProg=");
		Serial.println(idProg);
	}
  // controllo force functionality
  float fmt=((float)(forceData.maxTemp))/10.0f;
  float fht=((float)(forceData.hysteresisTemp))/100.0f;
  if(forceData.hForce) {
      if(forceData.tFin>now.get()) {
        if(forceData.hForce>0 && hh>7 && hh<23 && ((tempNow<fmt && !bFire) || (tempNow<fmt+fht && bFire))) // forza ma non di notte e se supero maxtemp
          bFire=true;
        else
          bFire=false;
        if(DEBUG) {
      		Serial.print("forceFor=");
      		Serial.print(forceData.hForce);
      		Serial.println(" hour");
      	}
        return;
      }
      else {
        forceData.hForce=0;
        forceData.tFin=0;
        saveData((byte*)&forceData, sizeof(forceData), EEPROM_OFSF);
      }
  }
  else {
    int i1, i2;
    findNextCheckPoint(idProg, i1, i2);
    float setPtemp = progs[idProg].T[i1]; // setpoint di temperatura da tenere in questa sequenza
    setPtemp /= 10.0;
    if(setPtemp>fmt)
      setPtemp=fmt;
    if ((!bFire && tempNow < setPtemp) || (bFire && tempNow < (setPtemp + fht)))
      bFire = true;
    else
      bFire = false;

  }
}

void findNextCheckPoint(int prg, int &i1, int &i2) {
  i1=0; i2=1;
  DateTime now = rtc.now(); //get the current date-time
  int tp, tn=now.hour()*60+now.minute();
  for(i2=0; i2<MAX_SEQ; i2++) {
    tp=(progs[prg].HM[i2]>>8)*60+(progs[prg].HM[i2]&0xFF);
    if(tp>tn || progs[prg].T[i2]==0)
      break;
  }
  if(i2==0)
    i1=MAX_SEQ-1;
  else if(i2==MAX_SEQ) {
    i1=MAX_SEQ-1;
    i2=0;
  }
  else {
    i1=i2-1;
    if(progs[prg].T[i2]==0)
      i2=0;
  }
  if (DEBUG) {
    Serial.print("i1=");
    Serial.print(i1);
    Serial.print(" i2=");
    Serial.print(i2);
    Serial.print(" temp=");
    Serial.println(progs[prg].T[i1]);
  }
}
