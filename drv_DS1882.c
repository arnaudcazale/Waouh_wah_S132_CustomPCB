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