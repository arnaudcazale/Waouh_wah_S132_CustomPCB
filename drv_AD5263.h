#ifndef DRV_AD5263_H__
#define DRV_AD5263_H__

#include <stdint.h>
#include <stddef.h>
#include "nrf_drv_twi.h"
#include "nrf_log.h"

#define AD5263_ADDR                     0x2C
 
enum
{
    DRV_AD5263_STATUS_CODE_SUCCESS,            ///< Successful.
    DRV_AD5263_STATUS_CODE_INVALID_PARAM,      ///< Invalid parameters.
    DRV_AD5263_STATUS_WRONG_DEVICE,            ///< Wrong device at I2C (TWI) address.
    DRV_AD5263_STATUS_UNINITALIZED,            ///< The driver is unitialized, please initialize.
};

enum
{
    AD5263_CHANNEL_1,            
    AD5263_CHANNEL_2,
    AD5263_CHANNEL_3,
    AD5263_CHANNEL_4,
};

/**@brief TWI communication initialization struct.
 */
typedef struct
{
    nrf_drv_twi_t         const * p_twi_instance;
    nrf_drv_twi_config_t  const * p_twi_cfg;
}drv_AD5263_init_t;


/**@brief Function for initializing the MPU-9250 driver.
 *
 * @param[in] p_params      Pointer to the init paramter structure.
 *
 * @retval NRF_SUCCESS.
 */
uint32_t drv_AD5263_init(drv_AD5263_init_t * p_params);


int drv_AD5263_write(unsigned char slave_addr, unsigned char instruction_byte, unsigned char const * p_data);

/**@brief Function for reading a MPU-9250 register.
 *
 * @param[in]  slave_addr   Slave address on the TWI bus.
 * @param[in]  reg_addr     Register address to read.
 * @param[in]  length       Length of the data to read.
 * @param[out] p_data       Pointer to where the data should be stored.
 *
 * @retval 0 if success. Else -1.
 */
int drv_ADG5245_read(unsigned char slave_addr, unsigned char * p_data);


#endif // DRV_AD5263_H__

/** @} */

