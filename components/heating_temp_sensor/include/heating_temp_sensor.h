/**
* Author: Wout Lyen
* Team: HOME3
*/

#include "esp_adc/adc_oneshot.h"
#include "esp_adc/adc_cali.h"
#include "esp_adc/adc_cali_scheme.h"

void start_heating_temp_sensor();
void set_hvac_mode(bool off);
void set_hvac_temp(float temp);