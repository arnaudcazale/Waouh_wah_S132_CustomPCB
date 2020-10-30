#include "saadc.h"

static uint8_t               m_input;
static nrf_saadc_value_t     m_buffer_pool[2][SAMPLES_IN_BUFFER];
static nrf_saadc_value_t     m_adc_buffer;
static nrf_ppi_channel_t     m_ppi_channel;
static uint32_t              m_adc_evt_counter;
static const nrf_drv_timer_t m_timer = NRF_DRV_TIMER_INSTANCE(1);
static ble_wah_t * m_wah_service;
static uint8_t m_sampling_time;
static uint16_t m_data = 0;
static uint8_t saadc_is_initialized = false;
static bool notif_authorize = false;

/**
 * @brief Function for confguring SAADC channel 0 for sampling AIN0 (P0.02).
 */
void saadc_init_one_shot(void)
{
    ret_code_t err_code;
    nrf_saadc_channel_config_t channel_config =
    NRF_DRV_SAADC_DEFAULT_CHANNEL_CONFIG_SE(NRF_SAADC_INPUT_AIN1);

    err_code = nrf_drv_saadc_init(NULL, saadc_callback);
    APP_ERROR_CHECK(err_code);

    err_code = nrf_drv_saadc_channel_init(0, &channel_config);
    APP_ERROR_CHECK(err_code);
}

void saadc_init()
{
    ret_code_t err_code;

    nrf_saadc_channel_config_t channel_config =
    NRF_DRV_SAADC_DEFAULT_CHANNEL_CONFIG_SE(m_input);
    NRF_LOG_INFO("SAADC_INPUT = %d", m_input);

    err_code = nrf_drv_saadc_init(NULL, saadc_callback);
    APP_ERROR_CHECK(err_code);

    err_code = nrf_drv_saadc_channel_init(0, &channel_config);
    APP_ERROR_CHECK(err_code);

    err_code = nrf_drv_saadc_buffer_convert(m_buffer_pool[0], SAMPLES_IN_BUFFER);
    APP_ERROR_CHECK(err_code);

    err_code = nrf_drv_saadc_buffer_convert(m_buffer_pool[1], SAMPLES_IN_BUFFER);
    APP_ERROR_CHECK(err_code);

    saadc_is_initialized = true;
}

void saadc_uninit()
{
    if(saadc_is_initialized)
    {
      nrf_drv_timer_disable(&m_timer);
      nrf_drv_timer_uninit(&m_timer);
      nrf_drv_ppi_channel_disable(m_ppi_channel);
      nrf_drv_ppi_uninit();
      nrf_drv_saadc_abort();
      nrf_drv_saadc_uninit();
      while(nrf_drv_saadc_is_busy());
      saadc_is_initialized = false;
    }
}

void saadc_sampling_event_init(void)
{
    ret_code_t err_code;

    err_code = nrf_drv_ppi_init();
    APP_ERROR_CHECK(err_code);

    nrf_drv_timer_config_t timer_cfg = NRF_DRV_TIMER_DEFAULT_CONFIG;
    timer_cfg.bit_width = NRF_TIMER_BIT_WIDTH_32;
    err_code = nrf_drv_timer_init(&m_timer, &timer_cfg, timer_handler);
    APP_ERROR_CHECK(err_code);

    /* setup m_timer for compare event every 400ms */
    uint32_t ticks = nrf_drv_timer_ms_to_ticks(&m_timer, m_sampling_time);
    nrf_drv_timer_extended_compare(&m_timer,
                                   NRF_TIMER_CC_CHANNEL0,
                                   ticks,
                                   NRF_TIMER_SHORT_COMPARE0_CLEAR_MASK,
                                   false);
    nrf_drv_timer_enable(&m_timer);

    uint32_t timer_compare_event_addr = nrf_drv_timer_compare_event_address_get(&m_timer,
                                                                                NRF_TIMER_CC_CHANNEL0);
    uint32_t saadc_sample_task_addr   = nrf_drv_saadc_sample_task_get();

    /* setup ppi channel so that timer compare event is triggering sample task in SAADC */
    err_code = nrf_drv_ppi_channel_alloc(&m_ppi_channel);
    APP_ERROR_CHECK(err_code);

    err_code = nrf_drv_ppi_channel_assign(m_ppi_channel,
                                          timer_compare_event_addr,
                                          saadc_sample_task_addr);
    APP_ERROR_CHECK(err_code);
}


void saadc_callback(nrf_drv_saadc_evt_t const * p_event)
{
    if (p_event->type == NRF_DRV_SAADC_EVT_DONE)
    {
        ret_code_t err_code;

        err_code = nrf_drv_saadc_buffer_convert(p_event->data.done.p_buffer, SAMPLES_IN_BUFFER);
        APP_ERROR_CHECK(err_code);

        // Avoid negative values due to noise (-1 lead to 65535 when cast to uint16_t)
        if(p_event->data.done.p_buffer[0] < 0)
        {
            m_adc_buffer = 0;            
        }else{
            m_adc_buffer = p_event->data.done.p_buffer[0];        
        }

        uint16_t new_data = (uint16_t)m_adc_buffer;

        if( ( new_data < (m_data - MARGE_DATA_IN) ) || ( new_data > (m_data + MARGE_DATA_IN) ))
        {
          NRF_LOG_INFO("saadc_data = %d", new_data);
          update_preset(new_data);
          if(m_wah_service->is_pedal_value_notif_enabled)
          {
            pedal_data_value_update(m_wah_service, new_data);
          }

        }

        m_data = new_data;
    }
}

void timer_handler(nrf_timer_event_t event_type, void * p_context)
{

}

void saadc_sampling_event_enable(void)
{
    ret_code_t err_code = nrf_drv_ppi_channel_enable(m_ppi_channel);

    APP_ERROR_CHECK(err_code);
}

void saadc_sampling_event_disable(void)
{
    ret_code_t err_code = nrf_drv_ppi_channel_disable(m_ppi_channel);

    APP_ERROR_CHECK(err_code);
}

void saadc_start(ble_wah_t * wah_service, uint8_t sampling_time, uint8_t input)
{
    m_wah_service =   wah_service; 
    m_sampling_time = sampling_time;
    m_input = input;
    saadc_init();
    saadc_sampling_event_init();
    saadc_sampling_event_enable();
}

void authorize_notifs(bool enable)
{
    notif_authorize = enable;
}