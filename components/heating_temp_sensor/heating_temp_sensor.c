#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "freertos/queue.h"
#include <math.h>
#include "esp_log.h"


#include "soc/soc_caps.h"
#include "heating_temp_sensor.h"

#include "mqtt_client.h"
extern esp_mqtt_client_handle_t event_client;

static const char *TAG = "TEMP_SENSOR";

#define TEMP_SENSOR          ADC_CHANNEL_3
#define ADC_ATTEN           ADC_ATTEN_DB_11

#define beta 3950                   // The beta coefficient or the B value of the thermistor
#define Rref 10000                  //Value of resistor used for the voltage divider
#define nominal_resistance 2200     //Nominal resistance at 25⁰C
#define nominal_temeprature 25      // temperature for nominal resistance (almost always 25⁰ C)

static int adc_raw[10];
static int voltage[10];

int hvac_mode = 1;          // 0 - Off | 1 - Heat | 2 - Cool
float hvac_temp = 21.0;
float current_temp = 20.0;
float current_ha_temp = 20.0;

static bool adc_calibration_init(adc_unit_t unit, adc_channel_t channel, adc_atten_t atten, adc_cali_handle_t *out_handle);

void publish_hvac_temp(){
    char status[4];
    sprintf(status,"%.1f", hvac_temp);
    esp_mqtt_client_publish(event_client, "hvac/temp/status", status, 0, 0, 0);
}

void publish_current_temp(){
    current_ha_temp = current_temp;
    char status[4];
    sprintf(status,"%.1f", current_temp);
    esp_mqtt_client_publish(event_client, "hvac/temp/current", status, 0, 0, 0);

    if (hvac_mode != 0)
    {
        if (hvac_temp >= current_temp)
        {
            hvac_mode = 1;
            esp_mqtt_client_publish(event_client, "hvac/mode/status", "heat", 0, 0, 0);
        }
        else if (hvac_temp < current_temp)
        {
            hvac_mode = 2;
            esp_mqtt_client_publish(event_client, "hvac/mode/status", "cool", 0, 0, 0);
        }
    }
    
}

void set_hvac_mode(bool off){

    if(off)
    {
        hvac_mode = 0;
        esp_mqtt_client_publish(event_client, "hvac/mode/status", "off", 0, 0, 0);
    }
    else if (hvac_temp >= current_ha_temp)
    {
        hvac_mode = 1;
        esp_mqtt_client_publish(event_client, "hvac/mode/status", "heat", 0, 0, 0);
    }
    else if (hvac_temp < current_ha_temp)
    {
        hvac_mode = 2;
        esp_mqtt_client_publish(event_client, "hvac/mode/status", "cool", 0, 0, 0);
    }
}

void set_hvac_temp(float temp){

    hvac_temp = temp;

    if (hvac_mode != 0)
    {
        if (hvac_temp >= current_ha_temp)
        {
            hvac_mode = 1;
            esp_mqtt_client_publish(event_client, "hvac/mode/status", "heat", 0, 0, 0);
        }
        else if (hvac_temp < current_ha_temp)
        {
            hvac_mode = 2;
            esp_mqtt_client_publish(event_client, "hvac/mode/status", "cool", 0, 0, 0);
        }
    }

    publish_hvac_temp();
    
}

void start_heating_temp_sensor(){ 

    //-------------ADC1 Init---------------//
    adc_oneshot_unit_handle_t adc1_handle;
    adc_oneshot_unit_init_cfg_t init_config1 = {
        .unit_id = ADC_UNIT_1,
    };
    ESP_ERROR_CHECK(adc_oneshot_new_unit(&init_config1, &adc1_handle));

    //-------------ADC1 Config---------------//
    adc_oneshot_chan_cfg_t config = {
        .bitwidth = ADC_BITWIDTH_DEFAULT,
        .atten = ADC_ATTEN,
    };
    ESP_ERROR_CHECK(adc_oneshot_config_channel(adc1_handle, TEMP_SENSOR, &config));

    //-------------ADC1 Calibration Init---------------//
    adc_cali_handle_t adc1_cali_chan0_handle = NULL;
    bool do_calibration1_chan0 = adc_calibration_init(ADC_UNIT_1, TEMP_SENSOR, ADC_ATTEN, &adc1_cali_chan0_handle);

    
    while(1) {

        ESP_ERROR_CHECK(adc_oneshot_read(adc1_handle, TEMP_SENSOR, &adc_raw[0]));

        int voltage_sum = 0; 
        if (do_calibration1_chan0) {
            for (int i = 0; i < 100; i++)
            {
                ESP_ERROR_CHECK(adc_cali_raw_to_voltage(adc1_cali_chan0_handle, adc_raw[0], &voltage[0]));
                voltage_sum += voltage[0];
                vTaskDelay(10 / portTICK_PERIOD_MS); // Delay for 10 second
            }
            //printf("%d\n", voltage_sum);
            voltage_sum = voltage_sum/100;
        }


        float temperature = 0;
        float resistance = 0;

        resistance = (voltage_sum*10000)/(3300-voltage_sum);
        //printf("Voltage = %d\n", voltage_sum);
        temperature = resistance / nominal_resistance;          // (R/Ro)
        temperature = log(temperature);                         // ln(R/Ro)
        temperature /= beta;                                    // 1/B * ln(R/Ro)
        temperature += 1.0 / (nominal_temeprature + 273.15);    // + (1/To)
        temperature = 1.0 / temperature;                        // Invert
        temperature = temperature -273.15;
        current_temp = temperature;                      // convert absolute temp to C
        
        //printf("Temp = %f\n", temperature);
        current_temp = roundf(current_temp * 2) / 2;
        printf("Temp = %f\n", current_temp);

        static int update_ha = 1;
        if (update_ha == 30)
        {
            publish_current_temp();

            update_ha = 0;
        }
        update_ha += 1;

    }
}



/*---------------------------------------------------------------
        ADC Calibration
---------------------------------------------------------------*/
static bool adc_calibration_init(adc_unit_t unit, adc_channel_t channel, adc_atten_t atten, adc_cali_handle_t *out_handle)
{
    adc_cali_handle_t handle = NULL;
    esp_err_t ret = ESP_FAIL;
    bool calibrated = false;

#if ADC_CALI_SCHEME_CURVE_FITTING_SUPPORTED
    if (!calibrated) {
        ESP_LOGI(TAG, "calibration scheme version is %s", "Curve Fitting");
        adc_cali_curve_fitting_config_t cali_config = {
            .unit_id = unit,
            .chan = channel,
            .atten = atten,
            .bitwidth = ADC_BITWIDTH_DEFAULT,
        };
        ret = adc_cali_create_scheme_curve_fitting(&cali_config, &handle);
        if (ret == ESP_OK) {
            calibrated = true;
        }
    }
#endif

#if ADC_CALI_SCHEME_LINE_FITTING_SUPPORTED
    if (!calibrated) {
        ESP_LOGI(TAG, "calibration scheme version is %s", "Line Fitting");
        adc_cali_line_fitting_config_t cali_config = {
            .unit_id = unit,
            .atten = atten,
            .bitwidth = ADC_BITWIDTH_DEFAULT,
        };
        ret = adc_cali_create_scheme_line_fitting(&cali_config, &handle);
        if (ret == ESP_OK) {
            calibrated = true;
        }
    }
#endif

    *out_handle = handle;
    if (ret == ESP_OK) {
        ESP_LOGI(TAG, "Calibration Success");
    } else if (ret == ESP_ERR_NOT_SUPPORTED || !calibrated) {
        ESP_LOGW(TAG, "eFuse not burnt, skip software calibration");
    } else {
        ESP_LOGE(TAG, "Invalid arg or no memory");
    }

    return calibrated;
}