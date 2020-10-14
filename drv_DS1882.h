#ifndef DRV_DS1882_H__
#define DRV_DS1882_H__

#include <stdint.h>
#include <stddef.h>
#include "nrf_drv_twi.h"
#include "nrf_log.h"

#define DS1882_ADDR                     0x28

 enum
{
    DRV_DS1882_STATUS_CODE_SUCCESS,            ///< Successful.
    DRV_DS1882_STATUS_CODE_INVALID_PARAM,      ///< Invalid parameters.
    DRV_DS1882_STATUS_WRONG_DEVICE,            ///< Wrong device at I2C (TWI) address.
    DRV_DS1882_STATUS_UNINITALIZED,            ///< The driver is unitialized, please initialize.
};

enum
{
    DS1882_CHANNEL_1,            
    DS1882_CHANNEL_2,
    DS1882_CONFIG,
};

/**@brief TWI communication initialization struct.
 */
typedef struct
{
    nrf_drv_twi_t         const * p_twi_instance;
    nrf_drv_twi_config_t  const * p_twi_cfg;
}drv_DS1882_init_t;


/**@brief Function for initializing the MPU-9250 driver.
 *
 * @param[in] p_params      Pointer to the init paramter structure.
 *
 * @retval NRF_SUCCESS.
 */
//uint32_t drv_ADG728_init(drv_ADG728_init_t * p_params);


//int drv_ADG728_write(unsigned char slave_addr, unsigned char const * p_data);

/**@brief Function for reading a MPU-9250 register.
 *
 * @param[in]  slave_addr   Slave address on the TWI bus.
 * @param[in]  reg_addr     Register address to read.
 * @param[in]  length       Length of the data to read.
 * @param[out] p_data       Pointer to where the data should be stored.
 *
 * @retval 0 if success. Else -1.
 */
//int drv_ADG728_read(unsigned char slave_addr, unsigned char * p_data);


#endif // DRV_DS1882_H__

/** @} */

