/** @file
 *  @brief Header file for the speed sensor driver.
 */

#ifndef SPEED_SENSOR_H_
#define SPEED_SENSOR_H_

/** @brief Return type for speed sensor functions. */
typedef enum {
  SPD_SENSOR_RET_OK = 0,  //!< Operation successful
  SPD_SENSOR_RET_NOK,  //!< Operation not successful
} spd_sensor_return_t;

/** @brief Initializes the speed sensor.
 */
spd_sensor_return_t speed_sensor_init(void);

/** @brief Get the current vehicle speed.
 *  @returns Return the current vehicle speed.
 */
float read_veh_speed();

#endif  //  SPEED_SENSOR_H_
