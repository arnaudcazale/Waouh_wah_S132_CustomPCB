#ifndef SAADC_H__
#define SAADC_H__

#include "nrf_log.h"
#include "nrf_log_internal.h"

#include "nrf_drv_saadc.h"
#include "nrf_drv_ppi.h"
#include "nrf_drv_timer.h"

#include "ble_wah.h"
#include "stroke.h"

enum
{
    M_SAADC_STATUS_CODE_SUCCESS,                     ///< Successfull
    M_SAADC_STATUS_CODE_INVALID_PARAM,               ///< Invalid parameters
};

#define MARGE_DATA_IN 0

#define SAMPLES_IN_BUFFER 1
#define ADC_GAIN                    NRF_SAADC_GAIN1_6     // ADC gain.
#define ADC_REFERENCE_VOLTAGE       (0.6f)              // The standard internal ADC reference voltage.
#define ADC_RESOLUTION_BITS         (8 + (SAADC_CONFIG_RESOLUTION * 2)) //ADC resolution [bits].

void saadc_init_one_shot();
void saadc_init();
void saadc_uninit();
void saadc_sampling_event_init(void);
void saadc_callback(nrf_drv_saadc_evt_t const *);
void timer_handler(nrf_timer_event_t event_type, void *);
void saadc_sampling_event_enable(void);
void saadc_start(ble_wah_t*, uint8_t, uint8_t);
uint16_t get_saadc_data(void);












#endif // SAADC_H__