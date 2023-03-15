#ifndef INA232_H
#define INA232_H

#include <stdint.h>

void set_configuration_register(uint16_t value);
void set_calibration_register(uint16_t value);
uint16_t read_manufacturer_id(void);
double read_shunt_voltage(void);
double read_bus_voltage(void);
double read_current(void);
double read_power(void);

int16_t read_shunt_voltage_raw(void);
uint16_t read_bus_voltage_raw(void);
int16_t read_current_raw(void);
uint16_t read_power_raw(void);


#endif