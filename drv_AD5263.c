#include "drv_AD5263.h"

static struct
{
    drv_AD5263_init_t        init;             ///< TWI configuration.
//    void                      (*cb)(void);      ///< Callback. Invoked when a pin interrupt is caught by GPIOTE.
    bool                      initialized;      ///< Module initialized.
    bool                      int_registered;   ///<
    bool                      enabled;          ///< Driver enabled.
//    uint32_t                  evt_sheduled;     ///< Number of scheduled events.
} m_AD5263 = {.initialized = false, .int_registered = false};

/**@brief Function to init / allocate the TWI module
 */
uint32_t drv_AD5263_init(drv_AD5263_init_t * p_params)
{
    if(p_params != NULL)
    {
        m_AD5263.init.p_twi_cfg      = p_params->p_twi_cfg;
        m_AD5263.init.p_twi_instance = p_params->p_twi_instance;
        m_AD5263.initialized         = true;    
    }
   
    return NRF_SUCCESS;
}

int drv_AD5263_write(unsigned char slave_addr, unsigned char channel_select, unsigned char const * p_data)
{
    uint32_t err_code;
    uint8_t buffer[2];
    buffer[0] = channel_select << 5;
    memcpy(&buffer[1], p_data, 1);

    err_code = nrf_drv_twi_tx( m_AD5263.init.p_twi_instance,
                               slave_addr,
                               buffer,
                               2,
                               false);
    if (err_code != NRF_SUCCESS)
    {
        NRF_LOG_ERROR("drv_AD5263_write Failed!\r\n");
    }

    return 0;
}

int drv_AD5263_read(unsigned char slave_addr, unsigned char * data)
{
    uint32_t err_code;

    err_code = nrf_drv_twi_rx( m_AD5263.init.p_twi_instance,
                               slave_addr,
                               data,
                               1 );
    if (err_code != NRF_SUCCESS)
    {
        NRF_LOG_ERROR("drv_AD5263_read Failed!\r\n");
    }

    return 0;
}