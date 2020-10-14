#include "drv_DS1882.h"

static struct
{
    drv_DS1882_init_t         init;             ///< TWI configuration.
//    void                      (*cb)(void);      ///< Callback. Invoked when a pin interrupt is caught by GPIOTE.
    bool                      initialized;      ///< Module initialized.
    bool                      int_registered;   ///<
    bool                      enabled;          ///< Driver enabled.
//    uint32_t                  evt_sheduled;     ///< Number of scheduled events.
} m_DS1882 = {.initialized = false, .int_registered = false};

/**@brief Function to init / allocate the TWI module
 */
uint32_t drv_DS1882_init(drv_DS1882_init_t * p_params)
{
    if(p_params != NULL)
    {
        m_DS1882.init.p_twi_cfg      = p_params->p_twi_cfg;
        m_DS1882.init.p_twi_instance = p_params->p_twi_instance;
        m_DS1882.initialized         = true;
    }
   
    return NRF_SUCCESS;
}

int drv_DS1882_write(unsigned char slave_addr, unsigned char channel_select, unsigned char const * p_data)
{
    uint32_t err_code;
    uint8_t buffer[1];
    buffer[0] = (channel_select << 6 ) | *p_data;
    
    err_code = nrf_drv_twi_tx( m_DS1882.init.p_twi_instance,
                               slave_addr,
                               buffer,
                               1,
                               false);
    if (err_code != NRF_SUCCESS)
    {
        NRF_LOG_ERROR("drv_DS1882_write Failed!\r\n");
    }

    return 0;
}

int drv_DS1882_read(unsigned char slave_addr, unsigned char * data)
{
    uint32_t err_code;

    err_code = nrf_drv_twi_rx( m_DS1882.init.p_twi_instance,
                               slave_addr,
                               data,
                               3 );
    if (err_code != NRF_SUCCESS)
    {
        NRF_LOG_ERROR("drv_DS1882_read Failed!\r\n");
    }

    return 0;
}