#include <stdint.h>

char digitToAscii[16] = { 0x30, 0X31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0X38,
		0x39, 0x61, 0X62, 0x63, 0x64, 0x65, 0x66 };

void getDecimalFromShort(char* buffer, unsigned short value) {
	unsigned short temp = value;
	uint8_t tenThousands;
	uint8_t thousands;
	uint8_t hundreds;
	uint8_t tens;
	uint8_t noTrailingZero = 0;

	tenThousands = value / 10000;
	temp -= tenThousands * 10000;
	thousands = temp / 1000;
	temp -= thousands * 1000;
	hundreds = temp / 100;
	temp -= hundreds * 100;
	tens = temp / 10;
	temp -= tens * 10;

	if (tenThousands > 0) {
		buffer[0] = digitToAscii[tenThousands];
		noTrailingZero = 1;
	} else {
		buffer[0] = 0x20;
	}
	if (thousands > 0 || noTrailingZero) {
		buffer[1] = digitToAscii[thousands];
		if (thousands > 0)
			noTrailingZero = 1;
	} else {
		buffer[1] = 0x20;
	}

	if (hundreds > 0 || noTrailingZero) {
		buffer[2] = digitToAscii[hundreds];
		if (hundreds > 0)
			noTrailingZero = 1;
	} else {
		buffer[2] = 0x20;
	}
	if (tens > 0 || noTrailingZero) {
		buffer[3] = digitToAscii[tens];
		if (tens > 0)
			noTrailingZero = 1;
	} else {
		buffer[3] = 0x20;
	}

	buffer[4] = digitToAscii[(uint8_t) temp];
}
