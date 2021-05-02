/** @file
 *  @brief Source file for the speed sensor driver.
 */

#include "speed_sensor.h"

#include <stdio.h>

spd_sensor_return_t speed_sensor_init() {
  printf("[SPEED_SENSOR] Initializing\n");
  return SPD_SENSOR_RET_OK;
}

float read_veh_speed() {
  // Assume the current vehicle speed is 10km/h
  // Change the value for the test purpose
  return 10;
}
