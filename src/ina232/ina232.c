
#include "ina232.h"
#include "smbus.h"

#define INA232_ADDR 0x40

#define CONFIGURATION_REGISTER   0x00
#define SHUNT_VOLTAGE_REGISTER   0x01
#define BUS_VOLTAGE_REGISTER     0x02
#define POWER_REGISTER           0x03
#define CURRENT_REGISTER         0x04
#define CALIBRTION_REGISTER      0x05
#define MANUFACTURER_ID_REGISTER 0x3E

/**
 * V_bus_max = 16V
 * V_shunt_max = 81.92mV
 * R_shunt = 0.01R
 * I_max = V_shunt_max / R_shunt = 0.08192 / 0.01 = 8.192A
 * I_expected = 8A
 * LSB_min = I_expected / (2^15-1) = 8 / 32767 = 0.000244148
 * LSB_current = 0.00025
 * calibration = 0.00512 / (LSB_current * R_shunt) = 0.00512 / (0.0025 * 0.01) = 2048
 *
 * Power LSB = LSB_current * 32 = 0.00025 * 32 = 0.008mW/LSB
 */

uint16_t read_manufacturer_id(void) {
    return smbus_read_word(INA232_ADDR, MANUFACTURER_ID_REGISTER);
}

double read_bus_voltage(void) {
    return (smbus_read_word(INA232_ADDR, BUS_VOLTAGE_REGISTER) * 0.0016);
}

double read_shunt_voltage(void) {
    return ((int16_t)smbus_read_word(INA232_ADDR, SHUNT_VOLTAGE_REGISTER) * 0.0000025);
}

double read_current(void) {
    return ((int16_t)smbus_read_word(INA232_ADDR, CURRENT_REGISTER) * 0.00025);
}

double read_power(void) {
    return (smbus_read_word(INA232_ADDR, POWER_REGISTER) * 0.008);
}

void set_calibration_register(uint16_t value) {
    smbus_write_word(INA232_ADDR, CALIBRTION_REGISTER, value);
}

uint16_t read_bus_voltage_raw(void) {
    return (smbus_read_word(INA232_ADDR, BUS_VOLTAGE_REGISTER));
}

int16_t read_shunt_voltage_raw(void) {
    return (int16_t)(smbus_read_word(INA232_ADDR, SHUNT_VOLTAGE_REGISTER));
}

int16_t read_current_raw(void) {
    return (int16_t)(smbus_read_word(INA232_ADDR, CURRENT_REGISTER));
}

uint16_t read_power_raw(void) {
    return (smbus_read_word(INA232_ADDR, POWER_REGISTER));
}

void set_configuration_register(uint16_t value) {
    smbus_write_word(INA232_ADDR, CONFIGURATION_REGISTER, value);
}
