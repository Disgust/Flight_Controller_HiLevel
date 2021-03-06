#include "BMP085.h"

void BMP085Calibration(void) {
	ac1 = BMP085ReadShort(0xAA);
	ac2 = BMP085ReadShort(0xAC);
	ac3 = BMP085ReadShort(0xAE);
	ac4 = BMP085ReadShort(0xB0);
	ac5 = BMP085ReadShort(0xB2);
	ac6 = BMP085ReadShort(0xB4);
	b1 = BMP085ReadShort(0xB6);
	b2 = BMP085ReadShort(0xB8);
	mb = BMP085ReadShort(0xBA);
	mc = BMP085ReadShort(0xBC);
	md = BMP085ReadShort(0xBE);
}

// bmp085ReadShort will read two sequential 8-bit registers, and return a 16-bit value
// the MSB register is read first
// Input: First register to read
// Output: 16-bit value of (first register value << 8) | (sequential register value)
uint16_t BMP085ReadShort(uint8_t address) {
	uint8_t msb, lsb;
	uint16_t data;
	
	TWIstart();
	TWIbyteWrite(BMP085_W);	// write SLA_W
	TWIbyteWrite(address);	// write register address
	TWIstart();
	TWIbyteWrite(BMP085_R);	// write SLA_R
	msb = TWIbyteRead();	// Get MSB result // TODO: Improve TWI Read for supporting ACKs and NACKs
	lsb = TWIbyteRead();	// Get LSB result
	TWIstop();
	data = msb << 8;
	data |= lsb;
	return data;
}

uint64_t BMP085ReadTemp(void) {
	TWIstart();
	
	TWIbyteWrite(BMP085_W);	// write SLA_W
	TWIbyteWrite(0xF4);	// write register address
	TWIbyteWrite(0x2E);	// write register data for temp

	//delay_ms(5);	// max time is 4.5ms
	
	return BMP085ReadShort(0xF6);
}

uint64_t BMP085ReadPressure(void) {
	uint16_t pressure = 0;
	
	TWIstart();
	
	TWIbyteWrite(BMP085_W);	// write SLA_W
	TWIbyteWrite(0xF4);	// write register address
	TWIbyteWrite(0x34);	// write register data for temp
	TWIstop();
	
	//delay_ms(5);	// max time is 4.5ms
	
	pressure = BMP085ReadShort(0xF6);
	pressure &= 0x0000FFFF;
	
	return pressure;
}

void BMP085Convert(Baro_t * buf) {
	int64_t ut;
	int64_t up;
	int64_t x1, x2, b5, b6, x3, b3, p;
	uint64_t b4, b7;
	
	ut = BMP085ReadTemp();
	ut = BMP085ReadTemp();	// some bug here, have to read twice to get good data
	up = BMP085ReadPressure();
	up = BMP085ReadPressure();
	
	x1 = ((int64_t)ut - ac6) * ac5 >> 15;
	x2 = ((int64_t) mc << 11) / (x1 + md);
	b5 = x1 + x2;
	buf->temperature = (b5 + 8) >> 4;
	
	b6= b5 - 4000;
	x1= (b2 * (b6 * b6 >> 12)) >> 11;
	x2= ac2 * b6 >> 11;
	x3= x1 + x2;
	b3= (((int32_t) ac1 * 4 + x3) + 2)/4;
	x1= ac3 * b6 >> 13;
	x2= (b1 * (b6 * b6 >> 12)) >> 16;
	x3= ((x1 + x2) + 2) >> 2;
	b4= (ac4 * (uint64_t) (x3 + 32768)) >> 15;
	b7= ((uint64_t) up - b3) * (50000 >> OSS);
	p= b7 < 0x80000000 ? (b7 * 2) / b4 : (b7 / b4) * 2;
	x1= (p >> 8) * (p >> 8);
	x1= (x1 * 3038) >> 16;
	x2= (-7357 * p) >> 16;
	buf->pressure = p + ((x1 + x2 + 3791) >> 4);
}

void BMP085CalculateAltitude(Baro_t * buf) {
	double temp= (double) buf->pressure/101325;
	temp= 1-pow(temp, 0.19029);
	buf->altitude= round(44330*temp);
}