/**
 *
 */

#ifndef HDC1080_HDC1080_H_
#define HDC1080_HDC1080_H_

#include <xdc/std.h>
#include <stdint.h>

#define HDC1080_SLAVE_ADDR 0x40

/* Data storage */
typedef struct {
	uint16_t temp;
	uint16_t humidity;
	Bool heaterOn;
} HDC1080_Struct;
typedef HDC1080_Struct * HDC1080_Handle;

/* User API */

Void HDC1080_init();  //! @brief Does nothing.
Bool HDC1080_open(UInt i2cInstance);  //! @brief Open the I2C bus and store a handle, returns true if a valid HDC1080 shows up at 0x40
Bool HDC1080_quickopen(I2C_Handle);   //! @brief Initialize library without performing I2C_open, returns true if valid HDC1080 shows up at 0x40
Void HDC1080_close();  //! @brief Closes the stored I2C_Handle if present.

HDC1080_Handle HDC1080_read();  //! @brief Perform a conversion, pausing 15ms using Task_sleep()
Void HDC1080_setConfiguration(uint16_t);  //! @brief Reconfigure the configuration register
Void HDC1080_heaterOn();  //! @brief Set configuration with HEAT=1, future conversions will reflect this
Void HDC1080_heaterOff(); //! @brief Set configuration with HEAT=0, the default config

int32_t HDC1080_temp_C_Q16(HDC1080_Handle);  //! @brief Convert temperature intermediate data into degrees Celsius (* 65536)
int32_t HDC1080_temp_C(HDC1080_Handle);  //! @brief Convert temperature intermediate data into degrees Celsius
int32_t HDC1080_temp_F_Q16(HDC1080_Handle);  //! @brief Convert temperature intermediate data into degrees Fahrenheit (* 65536)
int32_t HDC1080_temp_F(HDC1080_Handle);  //! @brief Convert temperature intermediate data into degrees Fahrenheit
int32_t HDC1080_relative_humidity_Q16(HDC1080_Handle); //! @brief Return relative humidity in integer value (e.g. 50*65536 for 50%, 1*65536 for 1%)
int32_t HDC1080_relative_humidity(HDC1080_Handle); //! @brief Return relative humidity in integer value (e.g. 50 for 50%, 1 for 1%)
int32_t HDC1080_dewpoint_C_Q16(HDC1080_Handle); //! @brief Convert temperature, humidity intermediate data into Dewpoint in Celsius (* 65536)
int32_t HDC1080_dewpoint_C(HDC1080_Handle); //! @brief Convert temperature, humidity intermediate data into Dewpoint in Celsius
int32_t HDC1080_dewpoint_F_Q16(HDC1080_Handle); //! @brief Convert temperature, humidity intermediate data into Dewpoint in Fahrenheit (* 65536)
int32_t HDC1080_dewpoint_F(HDC1080_Handle); //! @brief Convert temperature, humidity intermediate data into Dewpoint in Fahrenheit


#endif /* HDC1080_HDC1080_H_ */
