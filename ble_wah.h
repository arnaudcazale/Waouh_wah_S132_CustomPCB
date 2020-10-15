#ifndef BLE_WAH_H__
#define BLE_WAH_H__

#include <stdint.h>
#include <stdbool.h>
#include "ble.h"
#include "ble_srv_common.h"
#include "utils.h"

#include "drv_AD5263.h"
#include "drv_DS1882.h"


#define DEBUG_PRESET_RUNTIME

#define WAH_SERVICE_UUID_BASE         {0xA9, 0xE9, 0x00, 0x00, 0x19, 0x4C, 0x45, 0x23, \
                                       0xA4, 0x73, 0x5F, 0xDF, 0x36, 0xAA, 0x4D, 0x21}
#define WAH_SERVICE_UUID               0x1400
#define PRESET_SELECTION_CHAR_UUID     0x1401
#define PRESET_1_CHAR_UUID             0x1402
#define PRESET_2_CHAR_UUID             0x1403
#define PRESET_3_CHAR_UUID             0x1404
#define PRESET_4_CHAR_UUID             0x1405
#define PEDAL_VALUE_CHAR_UUID          0x1406
#define CALIBRATION_CHAR_UUID          0x1407

#define NRF_BLE_MAX_MTU_SIZE 247

#define SAADC_RES 1024

#define SAMPLING_20MS    20


#ifdef __GNUC__
    #ifdef PACKED
        #undef PACKED
    #endif

    #define PACKED(TYPE) TYPE __attribute__ ((packed))
#endif

/**@brief   Macro for defining a ble_cus instance.
 *
 * @param   _name   Name of the instance.
 * @hideinitializer
 */
#define BLE_WAH_DEF(_name)                                                                          \
static ble_wah_t _name;                                                                             \
NRF_SDH_BLE_OBSERVER(_name ## _obs,                                                                 \
                     BLE_HRS_BLE_OBSERVER_PRIO,                                                     \
                     ble_wah_on_ble_evt, &_name)      

typedef enum
{
    BLE_WAH_EVT_PRESET_SELECTION_VALUE_RECEIVED,
    BLE_WAH_EVT_PRESET_1_RECEIVED,
    BLE_WAH_EVT_PRESET_2_RECEIVED,
    BLE_WAH_EVT_PRESET_3_RECEIVED,
    BLE_WAH_EVT_PRESET_4_RECEIVED,
    BLE_WAH_EVT_NOTIF_PRESET_SELECTION,
    BLE_WAH_EVT_NOTIF_PEDAL_VALUE,
    BLE_WAH_EVT_CONFIG_RECEIVED,
    BLE_WAH_EVT_DISCONNECTED,
    BLE_WAH_EVT_CONNECTED
} ble_wah_evt_type_t;


/**@brief Custom Service event. */
typedef struct
{
    ble_wah_evt_type_t evt_type;
    uint8_t          * p_data;
    uint16_t           length;
} ble_wah_evt_t; 

typedef PACKED( struct
{
    uint8_t    STATUS;
    uint16_t   MIN_DATA;
    uint16_t   MAX_DATA;

}) ble_wah_calib_config_t;      

         
// Forward declaration of the ble_cus_t type.
typedef struct ble_wah_s ble_wah_t;

/**@brief Custom Service event handler type. */
typedef void (*ble_wah_evt_handler_t) (ble_wah_t * p_wah, ble_wah_evt_t * p_evt);                                                        

/**@brief Custom Service init structure. This contains all options and data needed for
 *        initialization of the service.*/
typedef struct
{
    ble_wah_evt_handler_t         evt_handler;                /**< Event handler to be called for handling events in the Custom Service. */
    int16_t                       initial_pedal_value;        /**< Initial custom value */
    ble_wah_calib_config_t        initial_calibration;
    ble_srv_cccd_security_mode_t  wah_value_char_attr_md;     /**< Initial security level for Custom characteristics attribute */
} ble_wah_init_t;

/**@brief Custom Service structure. This contains various status information for the service. */
struct ble_wah_s
{
    ble_wah_evt_handler_t         evt_handler;                              /**< Event handler to be called for handling events in the Custom Service. */
    uint16_t                      service_handle;                           /**< Handle of Custom Service (as provided by the BLE stack). */
    ble_gatts_char_handles_t      preset_selection_value_handles;           /**< Handles related to the Custom Value characteristic. */
    ble_gatts_char_handles_t      preset_1_handles;                         /**< Handles related to the Custom Value characteristic. */
    ble_gatts_char_handles_t      preset_2_handles;
    ble_gatts_char_handles_t      preset_3_handles;
    ble_gatts_char_handles_t      preset_4_handles;
    ble_gatts_char_handles_t      pedal_value_handles;
    ble_gatts_char_handles_t      calibration_handles;
    bool                          is_preset_selection_notif_enabled;        /**< Variable to indicate if the peer has enabled notification of the characteristic.*/
    bool                          is_pedal_value_notif_enabled; 
    bool                          is_preset_1_notif_enabled; 
    bool                          is_preset_2_notif_enabled; 
    bool                          is_preset_3_notif_enabled; 
    bool                          is_preset_4_notif_enabled; 
    uint16_t                      conn_handle;                              /**< Handle of thunknow type current connection (as provided by the BLE stack, is BLE_CONN_HANDLE_INVALID if not in a connection). */
    uint8_t                       uuid_type; 
};

/**@brief Function for initializing the Custom Service.
 *
 * @param[out]  p_cus       Custom Service structure. This structure will have to be supplied by
 *                          the application. It will be initialized by this function, and will later
 *                          be used to identify this particular service instance.
 * @param[in]   p_cus_init  Information needed to initialize the service.
 *
 * @return      NRF_SUCCESS on successful initialization of service, otherwise an error code.
 */
uint32_t ble_wah_init(ble_wah_t *, const ble_wah_init_t *);
void ble_wah_on_ble_evt( ble_evt_t const * p_ble_evt, void * p_context);

static void on_connect(ble_wah_t *, ble_evt_t const *);
static void on_disconnect(ble_wah_t *, ble_evt_t const *);
static void on_write(ble_wah_t *, ble_evt_t const *);
static void on_autorize_req(ble_wah_t *, ble_evt_t const *);

static uint32_t preset_selection_value_char_add(ble_wah_t *, const ble_wah_init_t *);
static uint32_t pedal_value_char_add(ble_wah_t *, const ble_wah_init_t *);
static uint32_t preset_1_char_add(ble_wah_t *, const ble_wah_init_t *);
static uint32_t preset_2_char_add(ble_wah_t *, const ble_wah_init_t *);
static uint32_t preset_3_char_add(ble_wah_t *, const ble_wah_init_t *);
static uint32_t preset_4_char_add(ble_wah_t *, const ble_wah_init_t *);
static uint32_t calibration_char_add(ble_wah_t *, const ble_wah_init_t *);

uint32_t preset_selection_value_update(ble_wah_t *, uint8_t);
uint32_t pedal_data_value_update(ble_wah_t *, uint16_t);
uint32_t preset_1_update(ble_wah_t *);
uint32_t preset_2_update(ble_wah_t *);
uint32_t preset_3_update(ble_wah_t *);
uint32_t preset_4_update(ble_wah_t *);
void check_data_received(uint8_t, uint8_t *, uint16_t);
void update_preset(int);
uint32_t check_mode(uint8_t);
void config_preset();
void reset_config_preset();
static void check_and_save_same_preset_name(uint8_t);
static void debug_preset(uint8_t);
static void send_notif(uint8_t);
static void set_filter_type(uint8_t);
static void timer_start_auto_wah(uint16_t );
static void timer_start_auto_level(uint16_t );
static void app_timer_periodic_handler_auto_wah(void * p_context);
static void app_timer_periodic_handler_auto_level(void * p_context);




#endif // BLE_WAH_H__