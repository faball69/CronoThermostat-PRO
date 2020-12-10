/*
 * gestione salvataggio parametri per programma CronoTermostato_FA
 * by Fabrizio Allevi
 * fabrizio . allevi @ tiscali . it
 */

// functions imported
void i2c_eeprom_write_byte( int deviceaddress, unsigned int eeaddress, byte data ) {
	int rdata = data;
	Wire.beginTransmission(deviceaddress);
	Wire.write((int)(eeaddress >> 8)); // MSB
	Wire.write((int)(eeaddress & 0xFF)); // LSB
	Wire.write(rdata);
	Wire.endTransmission();
}

// WARNING: address is a page address, 6-bit end will wrap around
// also, data can be maximum of about 30 bytes, because the Wire library has a buffer of 32 bytes
void i2c_eeprom_write_page( int deviceaddress, unsigned int eeaddresspage, byte* data, byte length ) {
	Wire.beginTransmission(deviceaddress);
	Wire.write((int)(eeaddresspage >> 8)); // MSB
	Wire.write((int)(eeaddresspage & 0xFF)); // LSB
	byte c;
	for ( c = 0; c < length; c++)
		Wire.write(data[c]);
	Wire.endTransmission();
}

byte i2c_eeprom_read_byte( int deviceaddress, unsigned int eeaddress ) {
	byte rdata = 0xFF;
	Wire.beginTransmission(deviceaddress);
	Wire.write((int)(eeaddress >> 8)); // MSB
	Wire.write((int)(eeaddress & 0xFF)); // LSB
	Wire.endTransmission();
	Wire.requestFrom(deviceaddress,1);
	if (Wire.available()) rdata = Wire.read();
	return rdata;
}

// maybe let's not read more than 30 or 32 bytes at a time!
void i2c_eeprom_read_buffer( int deviceaddress, unsigned int eeaddress, byte *buffer, int length ) {
	Wire.beginTransmission(deviceaddress);
	Wire.write((int)(eeaddress >> 8)); // MSB
	Wire.write((int)(eeaddress & 0xFF)); // LSB
	Wire.endTransmission();
	Wire.requestFrom(deviceaddress,length);
	int c = 0;
	for ( c = 0; c < length; c++ )
		if (Wire.available()) buffer[c] = Wire.read();
}

void saveData(byte *pb, int sizeTot, int addr) {
	// save data array ////////////////////////////////
	if(DEBUG) {
		Serial.print("sizeTot=");
		Serial.println(sizeTot);
	}
	int size=(sizeTot<16)?sizeTot:16;
	while(size) {
		i2c_eeprom_write_page(0x57, addr, pb, size);
		delay(100); //add a small delay
		if(DEBUG) {
			byte tmp[16];
			i2c_eeprom_read_buffer(0x57, addr, tmp, size);
			for(int c=0;c<size;c++) {
				Serial.print(tmp[c]);
				Serial.print('-');
			}
			Serial.print('\n');
		}
		pb+=size; addr+=size;
		sizeTot-=size;
		if(sizeTot<size)
			size=sizeTot;
	}
}

void loadData(byte *pb, int sizeTot, int addr) {
	// save data array ////////////////////////////////
	if(DEBUG) {
		Serial.print("sizeTot=");
		Serial.println(sizeTot);
	}
	int size=(sizeTot<16)?sizeTot:16;
	while(size) {
		i2c_eeprom_read_buffer(0x57, addr, pb, size);
		delay(100); //add a small delay
		if(DEBUG) {
			for(int c=0;c<size;c++) {
				Serial.print(pb[c]);
				Serial.print('-');
			}
			Serial.print('\n');
		}
		pb+=size; addr+=size;
		sizeTot-=size;
		if(sizeTot<size)
			size=sizeTot;
	}
}
