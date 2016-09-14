/**
 *
 */

#include <xdc/std.h>
#include <xdc/runtime/System.h>
#include <stdint.h>
#include "hdc1080.h"

typedef struct {
    uint16_t operand;
    int32_t value;
} LogLUT_t;

static const LogLUT_t LUT_ln_Q16[] = {
    {0, -2147483648},  // This should be an invalid number as it's an asymptote
    {655, -301804}, // 0.01, ln(0.01)
    {1310, -256378}, // 0.02, ln(0.02)
    {1966, -229805}, // etc...
    {2621, -210952},
    {3276, -196328},
    {3932, -184379},
    {4587, -174277},
    {5242, -165526},
    {5898, -157807},
    {6553, -150902},
    {7208, -144655},
    {7864, -138953},
    {8519, -133707},
    {9175, -128851},
    {9830, -124329},
    {10485, -120100},
    {11141, -116126},
    {11796, -112381},
    {12451, -108837},
    {13107, -105476},
    {13762, -102278},
    {14417, -99229},
    {15073, -96316},
    {15728, -93527},
    {16384, -90852},
    {17039, -88281},
    {17694, -85808},
    {18350, -83425},
    {19005, -81125},
    {19660, -78903},
    {20316, -76754},
    {20971, -74673},
    {21626, -72657},
    {22282, -70700},
    {22937, -68801},
    {23592, -66954},
    {24248, -65159},
    {24903, -63411},
    {25559, -61709},
    {26214, -60050},
    {26869, -58431},
    {27525, -56852},
    {28180, -55310},
    {28835, -53803},
    {29491, -52331},
    {30146, -50890},
    {30801, -49481},
    {31457, -48101},
    {32112, -46750},
    {32768, -45426},
    {33423, -44128},
    {34078, -42855},
    {34734, -41607},
    {35389, -40382},
    {36044, -39179},
    {36700, -37998},
    {37355, -36839},
    {38010, -35699},
    {38666, -34578},
    {39321, -33477},
    {39976, -32394},
    {40632, -31328},
    {41287, -30279},
    {41943, -29247},
    {42598, -28231},
    {43253, -27231},
    {43909, -26245},
    {44564, -25274},
    {45219, -24318},
    {45875, -23375},
    {46530, -22445},
    {47185, -21528},
    {47841, -20624},
    {48496, -19733},
    {49152, -18853},
    {49807, -17985},
    {50462, -17128},
    {51118, -16283},
    {51773, -15448},
    {52428, -14623},
    {53084, -13809},
    {53739, -13005},
    {54394, -12211},
    {55050, -11426},
    {55705, -10650},
    {56360, -9884},
    {57016, -9126},
    {57671, -8377},
    {58327, -7637},
    {58982, -6904},
    {59637, -6180},
    {60293, -5464},
    {60948, -4755},
    {61603, -4055},
    {62259, -3361},
    {62914, -2675},
    {63569, -1996},
    {64225, -1324},
    {64880, -658}
};

int32_t HDC1080_temp_C(HDC1080_Handle h)
{
	// temp is the original value from the chip, unmodified

	int32_t wTemp = (int32_t)h->temp;
	// temp is a fractional unsigned value in Q0.16, a number between 0.0 and 165.0, equalling measured degrees Celsius plus 40.
	wTemp *= 165;
	wTemp -= 40 * 65536;  // Convert to signed degrees C in Q15.16 format
	wTemp /= 65536; // Convert to whole degrees C

	return wTemp;
}

int32_t HDC1080_temp_F(HDC1080_Handle h)
{
	// temp is the original value from the chip, unmodified
	int32_t wTemp = (int32_t)h->temp;
	wTemp *= 165;
	wTemp -= 40 * 65536;  // Convert to signed degrees C in Q15.16 format
	wTemp *= 9;
	wTemp /= 5;
	wTemp += 32 * 65536; // Convert to degrees F in Q15.16 format
	wTemp /= 65536; // Convert to whole degrees F

	return wTemp;
}

int32_t HDC1080_relative_humidity(HDC1080_Handle h)
{
	return ((int32_t)h->humidity * 100) / 65536;
}

int32_t HDC1080_dewpoint_C(HDC1080_Handle h)
{
	// Temp, relative humidity assumed to be in chip-unmodified format

	// Find natural logarithm of relative humidity
	int i;
	int32_t lnRh;

	for (i=1; i <= 100; i++) {
		if (h->humidity > LUT_ln_Q16[i-1].operand && h->humidity <= LUT_ln_Q16[i].operand) {
			lnRh = LUT_ln_Q16[i].value;
			break;
		}
	}
	if (i > 100) {
		return 0;  // Error
	}

	// Convert temperature into Q47.16 degrees Celsius first
	int64_t wTemp = (int64_t)h->temp;
	wTemp *= 165;
	wTemp -= 40 * 65536;

	// Dewpoint formula derived from http://andrew.rsmas.miami.edu/bmcnoldy/Humidity.html with floating point values converted to fixed-point Q47.16
	// 64-bit precision is required for some of the multiplications here in order to retain decent precision...
	int64_t dewpt = 15927869 * (lnRh + ((1155072 * wTemp) / (15927869 + wTemp))) / (1155072 - lnRh - ((1155072 * wTemp) / (15927869 + wTemp)));
	dewpt /= 65536;

	return (int32_t)dewpt;
}

int32_t HDC1080_dewpoint_F(HDC1080_Handle h)
{
	// Temp, relative humidity assumed to be in chip-unmodified format

	// Find natural logarithm of relative humidity
	int i;
	int32_t lnRh;

	for (i=1; i <= 100; i++) {
		if (h->humidity > LUT_ln_Q16[i-1].operand && h->humidity <= LUT_ln_Q16[i].operand) {
			lnRh = LUT_ln_Q16[i].value;
			break;
		}
	}
	if (i > 100) {
		return 0;  // Error
	}

	// Convert temperature into Q47.16 degrees Celsius
	int64_t wTemp = (int64_t)h->temp;
	wTemp *= 165;
	wTemp -= 40 * 65536;

	// Dewpoint formula derived from http://andrew.rsmas.miami.edu/bmcnoldy/Humidity.html with floating point values
	// 64-bit precision is required for some of the multiplications here in order to retain decent precision...
	int64_t dewpt = 15927869 * (lnRh + ((1155072 * wTemp) / (15927869 + wTemp))) / (1155072 - lnRh - ((1155072 * wTemp) / (15927869 + wTemp)));

	// Dewpoint is in C, convert to F
	dewpt *= 9;
	dewpt /= 5;
	dewpt += 32 * 65536;
	dewpt /= 65536;

	return (int32_t)dewpt;
}
