/** @file
 *  @brief Main file.
 *  @description Just contains some example code. Adapt it in the way you like.
 */

#include <math.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>

#include "drivers/adc_driver/adc_driver.h"
#include "drivers/error_led/error_led.h"
#include "drivers/speed_sensor/speed_sensor.h"

#define MIN_PEDAL_ANGLE     0
#define MAX_PEDAL_ANGLE     30
#define TOLERANCE_DEVIATION 0.05

static int   get_torque(float veh_speed, float pedal_angle);
static float calculate_angle(adc_channel_id_t id, adc_value_t value);
static float determine_final_angle(float adc0_angle, float adc1_angle);

int main(int argc, char *argv[]) {
  adc_return_t        adc_res0    = ADC_RET_OK;
  adc_return_t        adc_res1    = ADC_RET_OK;
  spd_sensor_return_t spd_res     = SPD_SENSOR_RET_OK;
  adc_value_t         value       = 0;
  int                 torque      = 0;
  float               adc0_angle  = 0.0;
  float               adc1_angle  = 0.0;
  float               pedal_angle = 0.0;
  float               veh_speed   = 0.0;

  // Initialize all the drivers
  error_led_init();
  adc_res0 = adc_init(ADC_CHANNEL0);
  adc_res1 = adc_init(ADC_CHANNEL1);
  spd_res  = speed_sensor_init();

  if (ADC_RET_NOK == adc_res0 || ADC_RET_NOK == adc_res1 || SPD_SENSOR_RET_NOK == spd_res)
  {
    printf("Initialization fails: ADC channel and/or speed sensor cannot be initialized.");
    error_led_set(true);

    // If any of the init is failed, the rest of the code should not continue
    return 0;
  }

  // Simulate ADC channel 0 and channel 1 output values
  // Change the values here to test
  adc_read_set_output(ADC_CHANNEL0, 19712, ADC_RET_OK);
  adc_read_set_output(ADC_CHANNEL1, 23552, ADC_RET_OK);

  // Calculate the pedal angle from ADC0
  if (ADC_RET_NOK == adc_read(ADC_CHANNEL0, &value))
  {
    printf("ERROR: Failed to read ADC_CHANNEL0 value\n");
    error_led_set(true);
  }
  else
  {
    adc0_angle = calculate_angle(ADC_CHANNEL0, value);
  }

  // Calculate the pedal angle from ADC1
  if (ADC_RET_NOK == adc_read(ADC_CHANNEL1, &value))
  {
    printf("ERROR: Failed to read ADC_CHANNEL1 value\n");
    error_led_set(true);
  }
  else
  {
    adc1_angle = calculate_angle(ADC_CHANNEL1, value);
  }

  // Determine the final pedal angle which will be used for calculating the torque
  pedal_angle = determine_final_angle(adc0_angle, adc1_angle);

  // Here I use the binary linear regression mathematic model for the convience. 
  // Other models such as Bivariate Polynomial Regression can be better.
  // The matlab code is in /'git root'/src/matlab/BinLinearRegression.m
  // The binary linear regression function calculated by Matlab is: 
  // torque = 2.4437 + 1.3249 * pedal_angle - 0.345 * veh_speed
  veh_speed = read_veh_speed();
  torque    = get_torque(veh_speed, pedal_angle);

  printf("INFO: Required torque is %d\n", torque);
  return torque;
}

/** @brief Calculate the pedal angle with ADC channel ID and corresponding ADC channel read value.
 *  @param[in] id The ID of the ADC channel.
 *  @param[in] value The ADC channel read value.
 *  @returns Calculated pedal angle (degrees).
 */
static float calculate_angle(adc_channel_id_t id, adc_value_t value)
{
  float pedal_angle = 0.0;
  float adc_voltage = 0.0;

  if (ADC_CHANNEL0 == id)
  {
    // adc0 = 0.5 + 0.1 * angle  ==>  angle = 10 * adc0 - 5
    adc_voltage = ((float) (value / pow(2, 16))) * 5;
    pedal_angle = 10 * adc_voltage - 5;
  }
  else if (ADC_CHANNEL1 == id)
  {
    // adc1 = 1 + 0.08 * angle  ==>  angle = 12.5 * adc1 - 12.5
    adc_voltage = ((float) (value / pow(2, 16))) * 5;
    pedal_angle = 12.5 * adc_voltage - 12.5;
  }
  else
  {
    pedal_angle = -1.0;
  }

  return pedal_angle;
}

/** @brief Validate the two calculated pedal angle (from two ADC channels) and determine the final angle which will be used for calculating the torque.
 *  @param[in] adc0_angle ADC channel0 calculated pedal angle.
 *  @param[in] adc1_angle ADC channel1 calculated pedal angle.
 *  @returns The final angle which will be used for calculating the torque
 */
static float determine_final_angle(float adc0_angle, float adc1_angle)
{
  float pedal_angle = 0.0;

  // The angles must be within the range [MIN_PEDAL_ANGLE, MAX_PEDAL_ANGLE]
  if ( (adc0_angle < MIN_PEDAL_ANGLE) || (adc0_angle > MAX_PEDAL_ANGLE) )
  {
    printf("ERROR: Calculated ADC channel0 returned pedal angle %f is out of range [%d, %d]\n", adc0_angle, MIN_PEDAL_ANGLE, MAX_PEDAL_ANGLE);
    error_led_set(true);
    if ( (adc1_angle >= MIN_PEDAL_ANGLE) && (adc1_angle <= MAX_PEDAL_ANGLE) )
    {
      pedal_angle = adc1_angle;
    }
    else
    {
      // If both ADCs are broken, return pedal angle 0 
      // (this should be defined in a requirement)
      pedal_angle = 0.0;
    }
  }
  else if ( (adc1_angle < MIN_PEDAL_ANGLE) || (adc1_angle > MAX_PEDAL_ANGLE) )
  {
    printf("ERROR: Calculated ADC channel1 returned pedal angle %f is out of range [%d, %d]\n", adc0_angle, MIN_PEDAL_ANGLE, MAX_PEDAL_ANGLE);
    error_led_set(true);
    pedal_angle = adc0_angle;
  }
  else
  {
    // If the two calculated angle values differ for more than 5% (assumed tolerance deviation), raise the error light
    if ((adc1_angle < ((1 - TOLERANCE_DEVIATION) * adc0_angle)) || (adc1_angle > ((1 + TOLERANCE_DEVIATION) * adc0_angle)))
    {
      printf("ERROR: Calculated pedal angles from ADC0 and ADC1 differ for more than %f percent\n", TOLERANCE_DEVIATION);
      error_led_set(true);
    }

    // If both ADCs can return a valid value, use ADC CH0 as default,
    // even if their values differ a lot (this should be defined in the requirement)
    pedal_angle = adc0_angle;
  }

  return pedal_angle;
}

/** @brief Calculate the required torque together with vehicle speed and pedal angle
 *  @param[in] veh_speed Current vehicle speed.
 *  @param[in] pedal_angle Current pedal angle.
 *  @returns Return the required torque.
 */
static int get_torque(float veh_speed, float pedal_angle)
{
  int torque = 0;

  // torque = 2.4437 + 1.3249 * pedal_angle - 0.345 * veh_speed
  // Round down
  torque = (int) (2.4437 + 1.3249 * pedal_angle - 0.345 * veh_speed);

  return torque;
}
