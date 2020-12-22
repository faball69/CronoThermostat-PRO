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
  // check sequences
	for(int seq=0; seq<MAX_SEQ-1; seq++) {
		int minI=(progs[idProg].HM[seq]>>8)*60+(progs[idProg].HM[seq]&0xFF);
		int minF=(progs[idProg].HM[seq+1]>>8)*60+(progs[idProg].HM[seq+1]&0xFF);
		if(minI<=minuteNow && (minuteNow<minF || minF==0)) { // trovata la seq in corso
      float setPtemp=progs[idProg].T[seq];  // setpoint di temperatura da tenere in questa sequenza
      setPtemp/=10.0;
      if(DEBUG) {
  			Serial.print("seq=");
  			Serial.print(seq);
  			Serial.print(" minI=");
  			Serial.print(minI);
        Serial.print(" minF=");
  			Serial.print(minF);
        Serial.print(" temp=");
  			Serial.println(setPtemp);
  		}
			if((!bFire && tempNow<setPtemp) || (bFire && tempNow<(setPtemp+fht)))
				bFire=true;
			else
				bFire=false;
      break;
		}
	}
}
