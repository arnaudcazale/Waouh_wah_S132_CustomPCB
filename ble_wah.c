#include "sdk_common.h"
#include "ble_srv_common.h"
#include "ble_wah.h"
#include <string.h>
#include "nrf_gpio.h"
#include "boards.h"
#include "nrf_log.h"
//#include "app_timer.h"



extern volatile preset_config_8_t      preset[PRESET_NUMBER];
extern volatile calib_config_8_t       calibration;
extern volatile stroke_config_t        stroke;
extern volatile uint8_t                m_preset_selection_value;
static uint8_t calib_wah_heel = true;

static ble_wah_t *                     m_wah_service;
static bool                            timer_is_running;
static uint8_t                         cpt_timer;
static uint8_t                         auto_data_up;
static bool trigger_up, trigger_down = false;

//APP_TIMER_DEF(m_timer_auto_wah);
//APP_TIMER_DEF(m_timer_auto_level);

const nrf_drv_timer_t TIMER = NRF_DRV_TIMER_INSTANCE(2);

uint32_t ble_wah_init(ble_wah_t * p_wah, const ble_wah_init_t * p_wah_init)
{
    if (p_wah == NULL || p_wah_init == NULL)
    {
        return NRF_ERROR_NULL;
    }
    
    uint32_t   err_code;
    ble_uuid_t ble_uuid;

    m_wah_service = p_wah;
    p_wah->is_preset_selection_notif_enabled        = false;
    p_wah->is_pedal_value_notif_enabled             = false;
    p_wah->is_calibration_notif_enabled             = false;
    p_wah->is_stroke_notif_enabled                = false;
    p_wah->is_preset_1_notif_enabled                = false;
    p_wah->is_preset_2_notif_enabled                = false;
    p_wah->is_preset_3_notif_enabled                = false;
    p_wah->is_preset_4_notif_enabled                = false;

    // Initialize service structure
    p_wah->evt_handler               = p_wah_init->evt_handler;
    p_wah->conn_handle               = BLE_CONN_HANDLE_INVALID;

    // Add Custom Service UUID
    ble_uuid128_t base_uuid = {WAH_SERVICE_UUID_BASE};
    err_code =  sd_ble_uuid_vs_add(&base_uuid, &p_wah->uuid_type);
    VERIFY_SUCCESS(err_code);

    ble_uuid.type = p_wah->uuid_type;
    ble_uuid.uuid = WAH_SERVICE_UUID;

    // Add the Custom Service
    err_code = sd_ble_gatts_service_add(BLE_GATTS_SRVC_TYPE_PRIMARY, &ble_uuid, &p_wah->service_handle);
    if (err_code != NRF_SUCCESS)
    {
        return err_code;
    }

    // Add Custom Value characteristic
    err_code = preset_selection_value_char_add(p_wah, p_wah_init);
    if (err_code != NRF_SUCCESS)
    {
        return err_code;
    }

    // Add Custom Value characteristic
    err_code = pedal_value_char_add(p_wah, p_wah_init);
    if (err_code != NRF_SUCCESS)
    {
        return err_code;
    }

    // Add Custom Value characteristic
    err_code = preset_1_char_add(p_wah, p_wah_init);
    if (err_code != NRF_SUCCESS)
    {
        return err_code;
    }

    // Add Custom Value characteristic
    err_code = preset_2_char_add(p_wah, p_wah_init);
    if (err_code != NRF_SUCCESS)
    {
        return err_code;
    }

    // Add Custom Value characteristic
    err_code = preset_3_char_add(p_wah, p_wah_init);
    if (err_code != NRF_SUCCESS)
    {
        return err_code;
    }

    // Add Custom Value characteristic
    err_code = preset_4_char_add(p_wah, p_wah_init);
    if (err_code != NRF_SUCCESS)
    {
        return err_code;
    }

    // Add Custom Value characteristic
    err_code = calibration_char_add(p_wah, p_wah_init);
    if (err_code != NRF_SUCCESS)
    {
        return err_code;
    }

    // Add Custom Value characteristic
    err_code = stroke_char_add(p_wah, p_wah_init);
    if (err_code != NRF_SUCCESS)
    {
        return err_code;
    }

    //Configure TIMER_LED for generating simple light effect - leds on board will invert his state one after the other.
    nrf_drv_timer_config_t timer_cfg = NRF_DRV_TIMER_DEFAULT_CONFIG;
    timer_cfg.bit_width = NRF_TIMER_BIT_WIDTH_32;
    err_code = nrf_drv_timer_init(&TIMER, &timer_cfg, timer_event_handler);
    APP_ERROR_CHECK(err_code);

//    err_code = app_timer_create(&m_timer_auto_wah, APP_TIMER_MODE_REPEATED, app_timer_periodic_handler_auto_wah);
//    APP_ERROR_CHECK(err_code);
//
//    err_code = app_timer_create(&m_timer_auto_level, APP_TIMER_MODE_REPEATED, app_timer_periodic_handler_auto_level);
//    APP_ERROR_CHECK(err_code);

}

void ble_wah_on_ble_evt( ble_evt_t const * p_ble_evt, void * p_context)
{
    ret_code_t err_code;
    ble_wah_t * p_wah = (ble_wah_t *) p_context;

    //NRF_LOG_INFO("BLE event received. Event type = 0x%X\r\n", p_ble_evt->header.evt_id); 
    
    if (p_wah == NULL || p_ble_evt == NULL)
    {
        return;
    }
    
    switch (p_ble_evt->header.evt_id)
    {
        case BLE_GAP_EVT_CONNECTED:
            on_connect(p_wah, p_ble_evt);
            break;

        case BLE_GAP_EVT_DISCONNECTED:
            on_disconnect(p_wah, p_ble_evt);
            break;

        case BLE_GATTS_EVT_WRITE:
            //NRF_LOG_INFO("BLE_GATTS_EVT_WRITE"); 
            on_write(p_wah, p_ble_evt);
           break;

        case BLE_GATTS_EVT_RW_AUTHORIZE_REQUEST:
            NRF_LOG_INFO("BLE_GATTS_EVT_RW_AUTHORIZE_REQUEST"); 
            on_autorize_req(p_wah, p_ble_evt);
           break;

        default:
            // No implementation needed.
            break;
    }
}

/**@brief Function for handling the Connect event.
 *
 * @param[in]   p_cus       Custom Service structure.
 * @param[in]   p_ble_evt   Event received from the BLE stack.
 */
static void on_connect(ble_wah_t * p_wah, ble_evt_t const * p_ble_evt)
{
    p_wah->conn_handle = p_ble_evt->evt.gap_evt.conn_handle;
    
    ble_wah_evt_t evt;

    evt.evt_type = BLE_WAH_EVT_CONNECTED;

    p_wah->evt_handler(p_wah, &evt);
}

/**@brief Function for handling the Disconnect event.
 *
 * @param[in]   p_cus       Custom Service structure.
 * @param[in]   p_ble_evt   Event received from the BLE stack.
 */
static void on_disconnect(ble_wah_t * p_wah, ble_evt_t const * p_ble_evt)
{
    UNUSED_PARAMETER(p_ble_evt);
    p_wah->conn_handle = BLE_CONN_HANDLE_INVALID;
}

static void on_write(ble_wah_t * p_wah, ble_evt_t const * p_ble_evt)
{
    ble_gatts_evt_write_t const * p_evt_write = &p_ble_evt->evt.gatts_evt.params.write;
    ble_wah_evt_t evt;
    
    // If PRESET_SELECTION data written
    if (p_evt_write->handle == p_wah->preset_selection_value_handles.value_handle)
    {
        evt.evt_type = BLE_WAH_EVT_PRESET_SELECTION_VALUE_RECEIVED;
        evt.p_data = p_evt_write->data;
        evt.length = p_evt_write->len;
        // Call the application event handler.
        p_wah->evt_handler(p_wah, &evt);
    }

    // If PRESET_SELECTION notification enabled/disabled written
    if ((p_evt_write->handle == p_wah->preset_selection_value_handles.cccd_handle)
        && (p_evt_write->len == 2)
       )
    {
        bool notif_enabled;
        notif_enabled = ble_srv_is_notification_enabled(p_evt_write->data);

        if (p_wah->is_preset_selection_notif_enabled != notif_enabled)
        {
            p_wah->is_preset_selection_notif_enabled = notif_enabled;

            if (p_wah->evt_handler != NULL)
            {
                evt.evt_type = BLE_WAH_EVT_NOTIF_PRESET_SELECTION;
                p_wah->evt_handler(p_wah, &evt);
                
            }
        }
    }

    // If PEDAL_VALUE notification enabled/disabled written
    if ((p_evt_write->handle == p_wah->pedal_value_handles.cccd_handle)
        && (p_evt_write->len == 2)
       )
    {
        bool notif_enabled;
        notif_enabled = ble_srv_is_notification_enabled(p_evt_write->data);

        if (p_wah->is_pedal_value_notif_enabled != notif_enabled)
        {
            p_wah->is_pedal_value_notif_enabled = notif_enabled;

            if (p_wah->evt_handler != NULL)
            {
                evt.evt_type = BLE_WAH_EVT_NOTIF_PEDAL_VALUE;
                p_wah->evt_handler(p_wah, &evt);
                
            }
        }
    }

    // If CALIBRATION notification enabled/disabled written
    if ((p_evt_write->handle == p_wah->calibration_handles.cccd_handle)
        && (p_evt_write->len == 2)
       )
    {
        bool notif_enabled;
        notif_enabled = ble_srv_is_notification_enabled(p_evt_write->data);

        if (p_wah->is_calibration_notif_enabled != notif_enabled)
        {
            p_wah->is_calibration_notif_enabled = notif_enabled;

            if (p_wah->evt_handler != NULL)
            {
                evt.evt_type = BLE_WAH_EVT_NOTIF_CALIBRATION;
                p_wah->evt_handler(p_wah, &evt);
                
            }
        }
    }

    // If STROKE notification enabled/disabled written
    if ((p_evt_write->handle == p_wah->stroke_handles.cccd_handle)
        && (p_evt_write->len == 2)
       )
    {
        bool notif_enabled;
        notif_enabled = ble_srv_is_notification_enabled(p_evt_write->data);

        if (p_wah->is_stroke_notif_enabled != notif_enabled)
        {
            p_wah->is_stroke_notif_enabled = notif_enabled;

            if (p_wah->evt_handler != NULL)
            {
                evt.evt_type = BLE_WAH_EVT_NOTIF_STROKE;
                p_wah->evt_handler(p_wah, &evt);
                
            }
        }
    }

    // If PRESET 1 data written
    if (p_evt_write->handle == p_wah->preset_1_handles.value_handle)
    {
        evt.evt_type = BLE_WAH_EVT_PRESET_1_RECEIVED;
        evt.p_data = p_evt_write->data;
        evt.length = p_evt_write->len;
        // Call the application event handler.
        p_wah->evt_handler(p_wah, &evt);
    }

     // If PRESET 2 data written
    if (p_evt_write->handle == p_wah->preset_2_handles.value_handle)
    {
        evt.evt_type = BLE_WAH_EVT_PRESET_2_RECEIVED;
        evt.p_data = p_evt_write->data;
        evt.length = p_evt_write->len;
        // Call the application event handler.
        p_wah->evt_handler(p_wah, &evt);
    }

     // If PRESET 3 data written
    if (p_evt_write->handle == p_wah->preset_3_handles.value_handle)
    {
        evt.evt_type = BLE_WAH_EVT_PRESET_3_RECEIVED;
        evt.p_data = p_evt_write->data;
        evt.length = p_evt_write->len;
        // Call the application event handler.
        p_wah->evt_handler(p_wah, &evt);
    }

     // If PRESET 4 data written
    if (p_evt_write->handle == p_wah->preset_4_handles.value_handle)
    {
        evt.evt_type = BLE_WAH_EVT_PRESET_4_RECEIVED;
        evt.p_data = p_evt_write->data;
        evt.length = p_evt_write->len;
        // Call the application event handler.
        p_wah->evt_handler(p_wah, &evt);
    }

    // If CALIBRATION data written
    if (p_evt_write->handle == p_wah->calibration_handles.value_handle)
    {
        evt.evt_type = BLE_WAH_EVT_CALIBRATION_RECEIVED;
        evt.p_data = p_evt_write->data;
        evt.length = p_evt_write->len;
        // Call the application event handler.
        p_wah->evt_handler(p_wah, &evt);
    }

    // If STROKE data written
    if (p_evt_write->handle == p_wah->stroke_handles.value_handle)
    {
        evt.evt_type = BLE_WAH_EVT_STROKE_RECEIVED;
        evt.p_data = p_evt_write->data;
        evt.length = p_evt_write->len;
        // Call the application event handler.
        p_wah->evt_handler(p_wah, &evt);
    }

}

static void on_autorize_req(ble_wah_t * p_wah, ble_evt_t const * p_ble_evt)
{

//    ble_gatts_evt_rw_authorize_request_t * p_evt_rw_authorize_request = &p_ble_evt->evt.gatts_evt.params.authorize_request;
//    uint32_t err_code;
//
//    if (p_evt_rw_authorize_request->type  == BLE_GATTS_AUTHORIZE_TYPE_WRITE)
//    {
//      //NRF_LOG_INFO("BLE_GATTS_AUTHORIZE_TYPE_WRITE");
//      if (p_evt_rw_authorize_request->request.write.handle == p_wah->preset_1_handles.value_handle)
//      {
//        ble_gatts_rw_authorize_reply_params_t rw_authorize_reply;
//        bool                                  valid_data = true;
//        // Check for valid data.
//        if(p_evt_rw_authorize_request->request.write.len != sizeof(preset_config_8_t))
//        {
//            valid_data = false;
//        }
//        else
//        {
//          preset_config_8_t * p_preset_config = (preset_config_8_t *)p_evt_rw_authorize_request->request.write.data;
//           if ( (p_preset_config->FC1    > 255) ||
//                (p_preset_config->FC2    > 255) ||
//                (p_preset_config->Q1     > 255) ||
//                (p_preset_config->Q2     > 255))   
//            {
//              valid_data = false;
//            }
//        }
//
//        rw_authorize_reply.type = BLE_GATTS_AUTHORIZE_TYPE_WRITE;
//
//        if (valid_data)
//        {
//            rw_authorize_reply.params.write.update      = 1;
//            rw_authorize_reply.params.write.gatt_status = BLE_GATT_STATUS_SUCCESS;
//            rw_authorize_reply.params.write.p_data      = p_evt_rw_authorize_request->request.write.data;
//            rw_authorize_reply.params.write.len         = p_evt_rw_authorize_request->request.write.len;
//            rw_authorize_reply.params.write.offset      = p_evt_rw_authorize_request->request.write.offset;
//        }
//        else
//        {
//            rw_authorize_reply.params.write.update      = 0;
//            rw_authorize_reply.params.write.gatt_status = BLE_GATT_STATUS_ATTERR_WRITE_NOT_PERMITTED;
//        }
//
//        err_code = sd_ble_gatts_rw_authorize_reply(p_ble_evt->evt.gatts_evt.conn_handle,
//                                                   &rw_authorize_reply);
//        APP_ERROR_CHECK(err_code);
//
//        if ( valid_data && (p_wah->evt_handler != NULL))
//            {
//                ble_wah_evt_t evt;
//                evt.evt_type = BLE_WAH_EVT_PRESET_1_RECEIVED;
//                evt.p_data = p_evt_rw_authorize_request->request.write.data;
//                evt.length = p_evt_rw_authorize_request->request.write.len;
//                p_wah->evt_handler(p_wah, &evt);
//            }
//      }
//    }
//
//    if (p_evt_rw_authorize_request->type  == BLE_GATTS_AUTHORIZE_TYPE_READ)
//    {
//      NRF_LOG_INFO("BLE_GATTS_AUTHORIZE_TYPE_READ");
//    }
   
}

/**@brief Function for adding the Custom Value characteristic.
 *
 * @param[in]   p_cus        Custom Service structure.
 * @param[in]   p_cus_init   Information needed to initialize the service.
 *
 * @return      NRF_SUCCESS on success, otherwise an error code.
 */
static uint32_t preset_selection_value_char_add(ble_wah_t * p_wah, const ble_wah_init_t * p_wah_init)
{
    uint32_t            err_code;
    ble_gatts_char_md_t char_md;
    ble_gatts_attr_md_t cccd_md;
    ble_gatts_attr_t    attr_char_value;
    ble_uuid_t          ble_uuid;
    ble_gatts_attr_md_t attr_md;
    uint8_t             data = {0};

    // Add Custom Value characteristic
    memset(&cccd_md, 0, sizeof(cccd_md));

    //  Read  operation on cccd should be possible without authentication.
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&cccd_md.read_perm);
    BLE_GAP_CONN_SEC_MODE_SET_ENC_NO_MITM(&cccd_md.write_perm);
    
    //cccd_md.write_perm = p_motion_init->motion_value_char_attr_md.cccd_write_perm;
    cccd_md.vloc       = BLE_GATTS_VLOC_STACK;

    memset(&char_md, 0, sizeof(char_md));

    char_md.char_props.read   = 1; 
    char_md.char_props.write  = 1; 
    char_md.char_props.notify = 1; 
    char_md.p_char_user_desc  = NULL;
    char_md.p_char_pf         = NULL;
    char_md.p_user_desc_md    = NULL;
    char_md.p_cccd_md         = &cccd_md;  
    char_md.p_sccd_md         = NULL;

    ble_uuid.type = p_wah->uuid_type;
    ble_uuid.uuid = PRESET_SELECTION_CHAR_UUID;
		
    memset(&attr_md, 0, sizeof(attr_md));

    BLE_GAP_CONN_SEC_MODE_SET_ENC_NO_MITM(&attr_md.read_perm);
    BLE_GAP_CONN_SEC_MODE_SET_ENC_NO_MITM(&attr_md.write_perm);

    attr_md.vloc       = BLE_GATTS_VLOC_STACK;
    attr_md.rd_auth    = 0;
    attr_md.wr_auth    = 0;
    attr_md.vlen       = 0;

    memset(&attr_char_value, 0, sizeof(attr_char_value));

    attr_char_value.p_uuid    = &ble_uuid;
    attr_char_value.p_attr_md = &attr_md;
    attr_char_value.init_len  = sizeof(uint8_t);
    attr_char_value.init_offs = 0;
    attr_char_value.p_value   = (uint8_t *)&data;
    attr_char_value.max_len   = sizeof(uint8_t);

    err_code = sd_ble_gatts_characteristic_add(p_wah->service_handle, &char_md,
                                               &attr_char_value,
                                               &p_wah->preset_selection_value_handles);
    if (err_code != NRF_SUCCESS)
    {
        return err_code;
    }

    return NRF_SUCCESS;
}


/**@brief Function for adding the Custom Value characteristic.
 *
 * @param[in]   p_cus        Custom Service structure.
 * @param[in]   p_cus_init   Information needed to initialize the service.
 *
 * @return      NRF_SUCCESS on success, otherwise an error code.
 */
static uint32_t pedal_value_char_add(ble_wah_t * p_wah, const ble_wah_init_t * p_wah_init)
{
    uint32_t            err_code;
    ble_gatts_char_md_t char_md;
    ble_gatts_attr_md_t cccd_md;
    ble_gatts_attr_t    attr_char_value;
    ble_uuid_t          ble_uuid;
    ble_gatts_attr_md_t attr_md;
    uint16_t            data = {0};
    
    // Add Custom Value characteristic
    memset(&cccd_md, 0, sizeof(cccd_md));

    //  Read  operation on cccd should be possible without authentication.
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&cccd_md.read_perm);
    BLE_GAP_CONN_SEC_MODE_SET_ENC_NO_MITM(&cccd_md.write_perm);
    
    //cccd_md.write_perm = p_motion_init->motion_value_char_attr_md.cccd_write_perm;
    cccd_md.vloc       = BLE_GATTS_VLOC_STACK;

    memset(&char_md, 0, sizeof(char_md));

    char_md.char_props.notify        = 1;
    char_md.p_char_user_desc         = NULL;
    char_md.p_char_pf                = NULL;
    char_md.p_user_desc_md           = NULL;
    char_md.p_cccd_md                = NULL;
    char_md.p_sccd_md                = NULL;

    ble_uuid.type = p_wah->uuid_type;
    ble_uuid.uuid = PEDAL_VALUE_CHAR_UUID;
		
    memset(&attr_md, 0, sizeof(attr_md));

    BLE_GAP_CONN_SEC_MODE_SET_NO_ACCESS(&attr_md.read_perm);
    BLE_GAP_CONN_SEC_MODE_SET_NO_ACCESS(&attr_md.write_perm);

    attr_md.vloc       = BLE_GATTS_VLOC_STACK;
    attr_md.rd_auth    = 0;
    attr_md.wr_auth    = 0;
    attr_md.vlen       = 0;

    memset(&attr_char_value, 0, sizeof(attr_char_value));

    attr_char_value.p_uuid    = &ble_uuid;
    attr_char_value.p_attr_md = &attr_md;
    attr_char_value.init_len  = sizeof(uint16_t);
    attr_char_value.init_offs = 0;
    attr_char_value.p_value   = (uint8_t *)data;
    attr_char_value.max_len   = sizeof(uint16_t);

    err_code = sd_ble_gatts_characteristic_add(p_wah->service_handle, &char_md,
                                               &attr_char_value,
                                               &p_wah->pedal_value_handles);
    if (err_code != NRF_SUCCESS)
    {
        return err_code;
    }

    return NRF_SUCCESS;
}

/**@brief Function for adding the Custom Value characteristic.
 *
 * @param[in]   p_cus        Custom Service structure.
 * @param[in]   p_cus_init   Information needed to initialize the service.
 *
 * @return      NRF_SUCCESS on success, otherwise an error code.
 */
static uint32_t preset_1_char_add(ble_wah_t * p_wah, const ble_wah_init_t * p_wah_init)
{
    uint32_t            err_code;
    ble_gatts_char_md_t char_md;
    ble_gatts_attr_md_t cccd_md;
    ble_gatts_attr_t    attr_char_value;
    ble_uuid_t          ble_uuid;
    ble_gatts_attr_md_t attr_md;

    // Add Custom Value characteristic
    memset(&cccd_md, 0, sizeof(cccd_md));

    //  Read  operation on cccd should be possible without authentication.
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&cccd_md.read_perm);
    BLE_GAP_CONN_SEC_MODE_SET_ENC_NO_MITM(&cccd_md.write_perm);
  
    cccd_md.vloc       = BLE_GATTS_VLOC_STACK;

    memset(&char_md, 0, sizeof(char_md));

    char_md.char_props.notify        = 1;
    char_md.char_props.read          = 1;
    char_md.char_props.write         = 1;
    char_md.char_props.write_wo_resp = 0;
    char_md.p_char_user_desc         = NULL;
    char_md.p_char_pf                = NULL;
    char_md.p_user_desc_md           = NULL;
    char_md.p_cccd_md                = NULL;
    char_md.p_sccd_md                = NULL;

    ble_uuid.type = p_wah->uuid_type;
    ble_uuid.uuid = PRESET_1_CHAR_UUID;
		
    memset(&attr_md, 0, sizeof(attr_md));

    BLE_GAP_CONN_SEC_MODE_SET_ENC_NO_MITM(&attr_md.read_perm);
    BLE_GAP_CONN_SEC_MODE_SET_ENC_NO_MITM(&attr_md.write_perm);

    attr_md.vloc       = BLE_GATTS_VLOC_STACK;//
    attr_md.rd_auth    = 0;
    attr_md.wr_auth    = 0;
    attr_md.vlen       = 0;

    memset(&attr_char_value, 0, sizeof(attr_char_value));

    attr_char_value.p_uuid    = &ble_uuid;
    attr_char_value.p_attr_md = &attr_md;
    attr_char_value.init_len  = sizeof(preset_config_8_t);
    attr_char_value.init_offs = 0;
    attr_char_value.p_value   = &preset[0];
    attr_char_value.max_len   = sizeof(preset_config_8_t);

    err_code = sd_ble_gatts_characteristic_add(p_wah->service_handle, &char_md,
                                               &attr_char_value,
                                               &p_wah->preset_1_handles);
    if (err_code != NRF_SUCCESS)
    {
        return err_code;
    }

    return NRF_SUCCESS;
}

/**@brief Function for adding the Custom Value characteristic.
 *
 * @param[in]   p_cus        Custom Service structure.
 * @param[in]   p_cus_init   Information needed to initialize the service.
 *
 * @return      NRF_SUCCESS on success, otherwise an error code.
 */
static uint32_t preset_2_char_add(ble_wah_t * p_wah, const ble_wah_init_t * p_wah_init)
{
    uint32_t            err_code;
    ble_gatts_char_md_t char_md;
    ble_gatts_attr_md_t cccd_md;
    ble_gatts_attr_t    attr_char_value;
    ble_uuid_t          ble_uuid;
    ble_gatts_attr_md_t attr_md;

    // Add Custom Value characteristic
    memset(&cccd_md, 0, sizeof(cccd_md));

    //  Read  operation on cccd should be possible without authentication.
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&cccd_md.read_perm);
    BLE_GAP_CONN_SEC_MODE_SET_ENC_NO_MITM(&cccd_md.write_perm);
  
    cccd_md.vloc       = BLE_GATTS_VLOC_STACK;

    memset(&char_md, 0, sizeof(char_md));

    char_md.char_props.notify        = 1;
    char_md.char_props.read          = 1;
    char_md.char_props.write         = 1;
    char_md.char_props.write_wo_resp = 0;
    char_md.p_char_user_desc         = NULL;
    char_md.p_char_pf                = NULL;
    char_md.p_user_desc_md           = NULL;
    char_md.p_cccd_md                = NULL;
    char_md.p_sccd_md                = NULL;

    ble_uuid.type = p_wah->uuid_type;
    ble_uuid.uuid = PRESET_2_CHAR_UUID;
		
    memset(&attr_md, 0, sizeof(attr_md));

    BLE_GAP_CONN_SEC_MODE_SET_ENC_NO_MITM(&attr_md.read_perm);
    BLE_GAP_CONN_SEC_MODE_SET_ENC_NO_MITM(&attr_md.write_perm);

    attr_md.vloc       = BLE_GATTS_VLOC_STACK;
    attr_md.rd_auth    = 0;
    attr_md.wr_auth    = 0;
    attr_md.vlen       = 0;

    memset(&attr_char_value, 0, sizeof(attr_char_value));

    attr_char_value.p_uuid    = &ble_uuid;
    attr_char_value.p_attr_md = &attr_md;
    attr_char_value.init_len  = sizeof(preset_config_8_t);
    attr_char_value.init_offs = 0;
    attr_char_value.p_value   = &preset[1];
    attr_char_value.max_len   = sizeof(preset_config_8_t);

    err_code = sd_ble_gatts_characteristic_add(p_wah->service_handle, &char_md,
                                               &attr_char_value,
                                               &p_wah->preset_2_handles);
    if (err_code != NRF_SUCCESS)
    {
        return err_code;
    }

    return NRF_SUCCESS;
}

/**@brief Function for adding the Custom Value characteristic.
 *
 * @param[in]   p_cus        Custom Service structure.
 * @param[in]   p_cus_init   Information needed to initialize the service.
 *
 * @return      NRF_SUCCESS on success, otherwise an error code.
 */
static uint32_t preset_3_char_add(ble_wah_t * p_wah, const ble_wah_init_t * p_wah_init)
{
    uint32_t            err_code;
    ble_gatts_char_md_t char_md;
    ble_gatts_attr_md_t cccd_md;
    ble_gatts_attr_t    attr_char_value;
    ble_uuid_t          ble_uuid;
    ble_gatts_attr_md_t attr_md;

    // Add Custom Value characteristic
    memset(&cccd_md, 0, sizeof(cccd_md));

    //  Read  operation on cccd should be possible without authentication.
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&cccd_md.read_perm);
    BLE_GAP_CONN_SEC_MODE_SET_ENC_NO_MITM(&cccd_md.write_perm);
  
    cccd_md.vloc       = BLE_GATTS_VLOC_STACK;

    memset(&char_md, 0, sizeof(char_md));

    char_md.char_props.notify        = 1;
    char_md.char_props.read          = 1;
    char_md.char_props.write         = 1;
    char_md.char_props.write_wo_resp = 0;
    char_md.p_char_user_desc         = NULL;
    char_md.p_char_pf                = NULL;
    char_md.p_user_desc_md           = NULL;
    char_md.p_cccd_md                = NULL;
    char_md.p_sccd_md                = NULL;

    ble_uuid.type = p_wah->uuid_type;
    ble_uuid.uuid = PRESET_3_CHAR_UUID;
		
    memset(&attr_md, 0, sizeof(attr_md));

    BLE_GAP_CONN_SEC_MODE_SET_ENC_NO_MITM(&attr_md.read_perm);
    BLE_GAP_CONN_SEC_MODE_SET_ENC_NO_MITM(&attr_md.write_perm);

    attr_md.vloc       = BLE_GATTS_VLOC_STACK;
    attr_md.rd_auth    = 0;
    attr_md.wr_auth    = 0;
    attr_md.vlen       = 0;

    memset(&attr_char_value, 0, sizeof(attr_char_value));

    attr_char_value.p_uuid    = &ble_uuid;
    attr_char_value.p_attr_md = &attr_md;
    attr_char_value.init_len  = sizeof(preset_config_8_t);
    attr_char_value.init_offs = 0;
    attr_char_value.p_value   = &preset[2];
    attr_char_value.max_len   = sizeof(preset_config_8_t);

    err_code = sd_ble_gatts_characteristic_add(p_wah->service_handle, &char_md,
                                               &attr_char_value,
                                               &p_wah->preset_3_handles);
    if (err_code != NRF_SUCCESS)
    {
        return err_code;
    }

    return NRF_SUCCESS;
}

/**@brief Function for adding the Custom Value characteristic.
 *
 * @param[in]   p_cus        Custom Service structure.
 * @param[in]   p_cus_init   Information needed to initialize the service.
 *
 * @return      NRF_SUCCESS on success, otherwise an error code.
 */
static uint32_t preset_4_char_add(ble_wah_t * p_wah, const ble_wah_init_t * p_wah_init)
{
    uint32_t            err_code;
    ble_gatts_char_md_t char_md;
    ble_gatts_attr_md_t cccd_md;
    ble_gatts_attr_t    attr_char_value;
    ble_uuid_t          ble_uuid;
    ble_gatts_attr_md_t attr_md;

    // Add Custom Value characteristic
    memset(&cccd_md, 0, sizeof(cccd_md));

    //  Read  operation on cccd should be possible without authentication.
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&cccd_md.read_perm);
    BLE_GAP_CONN_SEC_MODE_SET_ENC_NO_MITM(&cccd_md.write_perm);
  
    cccd_md.vloc       = BLE_GATTS_VLOC_STACK;

    memset(&char_md, 0, sizeof(char_md));

    char_md.char_props.notify        = 1;
    char_md.char_props.read          = 1;
    char_md.char_props.write         = 1;
    char_md.char_props.write_wo_resp = 0;
    char_md.p_char_user_desc         = NULL;
    char_md.p_char_pf                = NULL;
    char_md.p_user_desc_md           = NULL;
    char_md.p_cccd_md                = NULL;
    char_md.p_sccd_md                = NULL;

    ble_uuid.type = p_wah->uuid_type;
    ble_uuid.uuid = PRESET_4_CHAR_UUID;
		
    memset(&attr_md, 0, sizeof(attr_md));

    BLE_GAP_CONN_SEC_MODE_SET_ENC_NO_MITM(&attr_md.read_perm);
    BLE_GAP_CONN_SEC_MODE_SET_ENC_NO_MITM(&attr_md.write_perm);

    attr_md.vloc       = BLE_GATTS_VLOC_STACK;
    attr_md.rd_auth    = 0;
    attr_md.wr_auth    = 0;
    attr_md.vlen       = 0;

    memset(&attr_char_value, 0, sizeof(attr_char_value));

    attr_char_value.p_uuid    = &ble_uuid;
    attr_char_value.p_attr_md = &attr_md;
    attr_char_value.init_len  = sizeof(preset_config_8_t);
    attr_char_value.init_offs = 0;
    attr_char_value.p_value   = &preset[3];
    attr_char_value.max_len   = sizeof(preset_config_8_t);

    err_code = sd_ble_gatts_characteristic_add(p_wah->service_handle, &char_md,
                                               &attr_char_value,
                                               &p_wah->preset_4_handles);
    if (err_code != NRF_SUCCESS)
    {
        return err_code;
    }

    return NRF_SUCCESS;
}

/**@brief Function for adding the Custom Value characteristic.
 *
 * @param[in]   p_cus        Custom Service structure.
 * @param[in]   p_cus_init   Information needed to initialize the service.
 *
 * @return      NRF_SUCCESS on success, otherwise an error code.
 */
static uint32_t calibration_char_add(ble_wah_t * p_wah, const ble_wah_init_t * p_wah_init)
{
    uint32_t                err_code;
    ble_gatts_char_md_t     char_md;
    ble_gatts_attr_md_t     cccd_md;
    ble_gatts_attr_t        attr_char_value;
    ble_uuid_t              ble_uuid;
    ble_gatts_attr_md_t     attr_md;
    //ble_wah_calib_config_t  data = p_wah_init->initial_calibration;

    // Add Custom Value characteristic
    memset(&cccd_md, 0, sizeof(cccd_md));

    //  Read  operation on cccd should be possible without authentication.
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&cccd_md.read_perm);
    BLE_GAP_CONN_SEC_MODE_SET_ENC_NO_MITM(&cccd_md.write_perm);
  
    cccd_md.vloc       = BLE_GATTS_VLOC_STACK;

    memset(&char_md, 0, sizeof(char_md));

    char_md.char_props.notify        = 1;
    char_md.char_props.read          = 1;
    char_md.char_props.write         = 1;
    char_md.char_props.write_wo_resp = 0;
    char_md.p_char_user_desc         = NULL;
    char_md.p_char_pf                = NULL;
    char_md.p_user_desc_md           = NULL;
    char_md.p_cccd_md                = NULL;
    char_md.p_sccd_md                = NULL;

    ble_uuid.type = p_wah->uuid_type;
    ble_uuid.uuid = CALIBRATION_CHAR_UUID;
		
    memset(&attr_md, 0, sizeof(attr_md));

    BLE_GAP_CONN_SEC_MODE_SET_ENC_NO_MITM(&attr_md.read_perm);
    BLE_GAP_CONN_SEC_MODE_SET_ENC_NO_MITM(&attr_md.write_perm);

    attr_md.vloc       = BLE_GATTS_VLOC_STACK;
    attr_md.rd_auth    = 0;
    attr_md.wr_auth    = 0;
    attr_md.vlen       = 0;

    memset(&attr_char_value, 0, sizeof(attr_char_value));

    attr_char_value.p_uuid    = &ble_uuid;
    attr_char_value.p_attr_md = &attr_md;
    attr_char_value.init_len  = sizeof(calib_config_8_t);
    attr_char_value.init_offs = 0;
    attr_char_value.p_value   = &calibration;
    attr_char_value.max_len   = sizeof(calib_config_8_t);

    err_code = sd_ble_gatts_characteristic_add(p_wah->service_handle, &char_md,
                                               &attr_char_value,
                                               &p_wah->calibration_handles);
    if (err_code != NRF_SUCCESS)
    {
        return err_code;
    }

    return NRF_SUCCESS;
}

/**@brief Function for adding the Custom Value characteristic.
 *
 * @param[in]   p_cus        Custom Service structure.
 * @param[in]   p_cus_init   Information needed to initialize the service.
 *
 * @return      NRF_SUCCESS on success, otherwise an error code.
 */
static uint32_t stroke_char_add(ble_wah_t * p_wah, const ble_wah_init_t * p_wah_init)
{
    uint32_t                err_code;
    ble_gatts_char_md_t     char_md;
    ble_gatts_attr_md_t     cccd_md;
    ble_gatts_attr_t        attr_char_value;
    ble_uuid_t              ble_uuid;
    ble_gatts_attr_md_t     attr_md;
    //ble_wah_calib_config_t  data = p_wah_init->initial_calibration;

    // Add Custom Value characteristic
    memset(&cccd_md, 0, sizeof(cccd_md));

    //  Read  operation on cccd should be possible without authentication.
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&cccd_md.read_perm);
    BLE_GAP_CONN_SEC_MODE_SET_ENC_NO_MITM(&cccd_md.write_perm);
  
    cccd_md.vloc       = BLE_GATTS_VLOC_STACK;

    memset(&char_md, 0, sizeof(char_md));

    char_md.char_props.notify        = 1;
    char_md.char_props.read          = 1;
    char_md.char_props.write         = 1;
    char_md.char_props.write_wo_resp = 0;
    char_md.p_char_user_desc         = NULL;
    char_md.p_char_pf                = NULL;
    char_md.p_user_desc_md           = NULL;
    char_md.p_cccd_md                = NULL;
    char_md.p_sccd_md                = NULL;

    ble_uuid.type = p_wah->uuid_type;
    ble_uuid.uuid = STROKE_CHAR_UUID;
		
    memset(&attr_md, 0, sizeof(attr_md));

    BLE_GAP_CONN_SEC_MODE_SET_ENC_NO_MITM(&attr_md.read_perm);
    BLE_GAP_CONN_SEC_MODE_SET_ENC_NO_MITM(&attr_md.write_perm);

    attr_md.vloc       = BLE_GATTS_VLOC_STACK;
    attr_md.rd_auth    = 0;
    attr_md.wr_auth    = 0;
    attr_md.vlen       = 0;

    memset(&attr_char_value, 0, sizeof(attr_char_value));

    attr_char_value.p_uuid    = &ble_uuid;
    attr_char_value.p_attr_md = &attr_md;
    attr_char_value.init_len  = sizeof(stroke_config_t);
    attr_char_value.init_offs = 0;
    attr_char_value.p_value   = &stroke;
    attr_char_value.max_len   = sizeof(stroke_config_t);

    err_code = sd_ble_gatts_characteristic_add(p_wah->service_handle, &char_md,
                                               &attr_char_value,
                                               &p_wah->stroke_handles);
    if (err_code != NRF_SUCCESS)
    {
        return err_code;
    }

    return NRF_SUCCESS;
}


/**@brief Function for adding the Custom Value characteristic.
 *
 * 
 */
uint32_t preset_selection_value_update(ble_wah_t * p_wah, uint8_t preset_selection_value)
{
 
    if (p_wah == NULL)
    {
        return NRF_ERROR_NULL;
    }

    uint32_t err_code = NRF_SUCCESS;
    ble_gatts_value_t gatts_value;

    // Initialize value struct.
    memset(&gatts_value, 0, sizeof(gatts_value));

    gatts_value.len     = sizeof(uint8_t);
    gatts_value.offset  = 0;
    gatts_value.p_value = &preset_selection_value;

    // Update database.
    err_code = sd_ble_gatts_value_set(p_wah->conn_handle,
                                      p_wah->preset_selection_value_handles.value_handle,
                                      &gatts_value);
    if (err_code != NRF_SUCCESS)
    {
        return err_code;
    }

    // Send value if connected and notifying.
    if ((p_wah->conn_handle != BLE_CONN_HANDLE_INVALID)) 
    {
        ble_gatts_hvx_params_t hvx_params;

        memset(&hvx_params, 0, sizeof(hvx_params));

        hvx_params.handle = p_wah->preset_selection_value_handles.value_handle;
        hvx_params.type   = BLE_GATT_HVX_NOTIFICATION;
        hvx_params.offset = gatts_value.offset;
        hvx_params.p_len  = &gatts_value.len;
        hvx_params.p_data = gatts_value.p_value;

        err_code = sd_ble_gatts_hvx(p_wah->conn_handle, &hvx_params);
    }
    else
    {
        err_code = NRF_ERROR_INVALID_STATE;
    }

    return err_code;
}

/**@brief Function for adding the Custom Value characteristic.
 *
 * 
 */
uint32_t pedal_data_value_update(ble_wah_t * p_wah, uint16_t data)
{

    if (p_wah == NULL)
    {
        return NRF_ERROR_NULL;
    }

    uint32_t err_code = NRF_SUCCESS;
    ble_gatts_value_t gatts_value;

    // Initialize value struct.
    memset(&gatts_value, 0, sizeof(gatts_value));

    gatts_value.len     = sizeof(uint16_t);
    gatts_value.offset  = 0;
    gatts_value.p_value = (uint8_t*)&data;

    // Update database.
    err_code = sd_ble_gatts_value_set(p_wah->conn_handle,
                                      p_wah->pedal_value_handles.value_handle,
                                      &gatts_value);
    if (err_code != NRF_SUCCESS)
    {
        return err_code;
    }

    // Send value if connected and notifying.
    if ((p_wah->conn_handle != BLE_CONN_HANDLE_INVALID)) 
    {
        ble_gatts_hvx_params_t hvx_params;

        memset(&hvx_params, 0, sizeof(hvx_params));

        hvx_params.handle = p_wah->pedal_value_handles.value_handle;
        hvx_params.type   = BLE_GATT_HVX_NOTIFICATION;
        hvx_params.offset = gatts_value.offset;
        hvx_params.p_len  = &gatts_value.len;
        hvx_params.p_data = gatts_value.p_value;

        err_code = sd_ble_gatts_hvx(p_wah->conn_handle, &hvx_params);
    }
    else
    {
        err_code = NRF_ERROR_INVALID_STATE;
    }

    return err_code;
}

/**@brief Function for adding the Custom Value characteristic.
 *
 * 
 */
uint32_t preset_1_update(ble_wah_t * p_wah)
{
 
    if (p_wah == NULL)
    {
        return NRF_ERROR_NULL;
    }

    uint32_t err_code = NRF_SUCCESS;
    ble_gatts_value_t gatts_value;

    // Initialize value struct.
    memset(&gatts_value, 0, sizeof(gatts_value));

    gatts_value.len     = sizeof(preset_config_8_t);
    gatts_value.offset  = 0;
    gatts_value.p_value = (uint8_t*)&preset[0];

    // Update database.
    err_code = sd_ble_gatts_value_set(p_wah->conn_handle,
                                      p_wah->preset_1_handles.value_handle,
                                      &gatts_value);
    if (err_code != NRF_SUCCESS)
    {
        return err_code;
    }

    // Send value if connected and notifying.
    if ((p_wah->conn_handle != BLE_CONN_HANDLE_INVALID)) 
    {
        ble_gatts_hvx_params_t hvx_params;
 
        memset(&hvx_params, 0, sizeof(hvx_params));

        hvx_params.handle = p_wah->preset_1_handles.value_handle;
        hvx_params.type   = BLE_GATT_HVX_NOTIFICATION;
        hvx_params.offset = gatts_value.offset;
        hvx_params.p_len  = &gatts_value.len;
        hvx_params.p_data = gatts_value.p_value;

        err_code = sd_ble_gatts_hvx(p_wah->conn_handle, &hvx_params);
    }
    else
    {
        err_code = NRF_ERROR_INVALID_STATE;
    }

    return err_code;
}

/**@brief Function for adding the Custom Value characteristic.
 *
 * 
 */
uint32_t preset_2_update(ble_wah_t * p_wah)
{
 
    if (p_wah == NULL)
    {
        return NRF_ERROR_NULL;
    }

    uint32_t err_code = NRF_SUCCESS;
    ble_gatts_value_t gatts_value;

    // Initialize value struct.
    memset(&gatts_value, 0, sizeof(gatts_value));

    gatts_value.len     = sizeof(preset_config_8_t);
    gatts_value.offset  = 0;
    gatts_value.p_value = (uint8_t*)&preset[1];

    // Update database.
    err_code = sd_ble_gatts_value_set(p_wah->conn_handle,
                                      p_wah->preset_2_handles.value_handle,
                                      &gatts_value);
    if (err_code != NRF_SUCCESS)
    {
        return err_code;
    }

    // Send value if connected and notifying.
    if ((p_wah->conn_handle != BLE_CONN_HANDLE_INVALID)) 
    {
        ble_gatts_hvx_params_t hvx_params;
 
        memset(&hvx_params, 0, sizeof(hvx_params));

        hvx_params.handle = p_wah->preset_2_handles.value_handle;
        hvx_params.type   = BLE_GATT_HVX_NOTIFICATION;
        hvx_params.offset = gatts_value.offset;
        hvx_params.p_len  = &gatts_value.len;
        hvx_params.p_data = gatts_value.p_value;

        err_code = sd_ble_gatts_hvx(p_wah->conn_handle, &hvx_params);
    }
    else
    {
        err_code = NRF_ERROR_INVALID_STATE;
    }

    return err_code;
}

/**@brief Function for adding the Custom Value characteristic.
 *
 * 
 */
uint32_t preset_3_update(ble_wah_t * p_wah)
{
 
    if (p_wah == NULL)
    {
        return NRF_ERROR_NULL;
    }

    uint32_t err_code = NRF_SUCCESS;
    ble_gatts_value_t gatts_value;

    // Initialize value struct.
    memset(&gatts_value, 0, sizeof(gatts_value));

    gatts_value.len     = sizeof(preset_config_8_t);
    gatts_value.offset  = 0;
    gatts_value.p_value = (uint8_t*)&preset[2];

    // Update database.
    err_code = sd_ble_gatts_value_set(p_wah->conn_handle,
                                      p_wah->preset_3_handles.value_handle,
                                      &gatts_value);
    if (err_code != NRF_SUCCESS)
    {
        return err_code;
    }

    // Send value if connected and notifying.
    if ((p_wah->conn_handle != BLE_CONN_HANDLE_INVALID)) 
    {
        ble_gatts_hvx_params_t hvx_params;
 
        memset(&hvx_params, 0, sizeof(hvx_params));

        hvx_params.handle = p_wah->preset_3_handles.value_handle;
        hvx_params.type   = BLE_GATT_HVX_NOTIFICATION;
        hvx_params.offset = gatts_value.offset;
        hvx_params.p_len  = &gatts_value.len;
        hvx_params.p_data = gatts_value.p_value;

        err_code = sd_ble_gatts_hvx(p_wah->conn_handle, &hvx_params);
    }
    else
    {
        err_code = NRF_ERROR_INVALID_STATE;
    }

    return err_code;
}

/**
 *
 * 
 */
uint32_t preset_4_update(ble_wah_t * p_wah)
{
 
    if (p_wah == NULL)
    {
        return NRF_ERROR_NULL;
    }

    uint32_t err_code = NRF_SUCCESS;
    ble_gatts_value_t gatts_value;

    // Initialize value struct.
    memset(&gatts_value, 0, sizeof(gatts_value));

    gatts_value.len     = sizeof(preset_config_8_t);
    gatts_value.offset  = 0;
    gatts_value.p_value = (uint8_t*)&preset[3];

    // Update database.
    err_code = sd_ble_gatts_value_set(p_wah->conn_handle,
                                      p_wah->preset_4_handles.value_handle,
                                      &gatts_value);
    if (err_code != NRF_SUCCESS)
    {
        return err_code;
    }

    // Send value if connected and notifying.
    if ((p_wah->conn_handle != BLE_CONN_HANDLE_INVALID)) 
    {
        ble_gatts_hvx_params_t hvx_params;
 
        memset(&hvx_params, 0, sizeof(hvx_params));

        hvx_params.handle = p_wah->preset_4_handles.value_handle;
        hvx_params.type   = BLE_GATT_HVX_NOTIFICATION;
        hvx_params.offset = gatts_value.offset;
        hvx_params.p_len  = &gatts_value.len;
        hvx_params.p_data = gatts_value.p_value;

        err_code = sd_ble_gatts_hvx(p_wah->conn_handle, &hvx_params);
    }
    else
    {
        err_code = NRF_ERROR_INVALID_STATE;
    }

    return err_code;
}

/**
 *
 * 
 */
uint32_t calibration_update(ble_wah_t * p_wah)
{
 
    if (p_wah == NULL)
    {
        return NRF_ERROR_NULL;
    }

    uint32_t err_code = NRF_SUCCESS;
    ble_gatts_value_t gatts_value;

    // Initialize value struct.
    memset(&gatts_value, 0, sizeof(gatts_value));

    gatts_value.len     = sizeof(calib_config_8_t);
    gatts_value.offset  = 0;
    gatts_value.p_value = (uint8_t*)&calibration;

    // Update database.
    err_code = sd_ble_gatts_value_set(p_wah->conn_handle,
                                      p_wah->calibration_handles.value_handle,
                                      &gatts_value);
    if (err_code != NRF_SUCCESS)
    {
        return err_code;
    }

    // Send value if connected and notifying.
    if ((p_wah->conn_handle != BLE_CONN_HANDLE_INVALID)) 
    {
        ble_gatts_hvx_params_t hvx_params;
 
        memset(&hvx_params, 0, sizeof(hvx_params));

        hvx_params.handle = p_wah->calibration_handles.value_handle;
        hvx_params.type   = BLE_GATT_HVX_NOTIFICATION;
        hvx_params.offset = gatts_value.offset;
        hvx_params.p_len  = &gatts_value.len;
        hvx_params.p_data = gatts_value.p_value;

        err_code = sd_ble_gatts_hvx(p_wah->conn_handle, &hvx_params);
    }
    else
    {
        err_code = NRF_ERROR_INVALID_STATE;
    }

    return err_code;
}

/**
 *
 * 
 */
uint32_t stroke_update(ble_wah_t * p_wah)
{
 
    if (p_wah == NULL)
    {
        return NRF_ERROR_NULL;
    }

    uint32_t err_code = NRF_SUCCESS;
    ble_gatts_value_t gatts_value;

    // Initialize value struct.
    memset(&gatts_value, 0, sizeof(gatts_value));

    gatts_value.len     = sizeof(stroke_config_t);
    gatts_value.offset  = 0;
    gatts_value.p_value = (uint8_t*)&stroke;

    // Update database.
    err_code = sd_ble_gatts_value_set(p_wah->conn_handle,
                                      p_wah->stroke_handles.value_handle,
                                      &gatts_value);
    if (err_code != NRF_SUCCESS)
    {
        return err_code;
    }

    // Send value if connected and notifying.
    if ((p_wah->conn_handle != BLE_CONN_HANDLE_INVALID)) 
    {
        ble_gatts_hvx_params_t hvx_params;
 
        memset(&hvx_params, 0, sizeof(hvx_params));

        hvx_params.handle = p_wah->stroke_handles.value_handle;
        hvx_params.type   = BLE_GATT_HVX_NOTIFICATION;
        hvx_params.offset = gatts_value.offset;
        hvx_params.p_len  = &gatts_value.len;
        hvx_params.p_data = gatts_value.p_value;

        err_code = sd_ble_gatts_hvx(p_wah->conn_handle, &hvx_params);
    }
    else
    {
        err_code = NRF_ERROR_INVALID_STATE;
    }

    return err_code;
}

/**@brief Function for adding the Custom Value characteristic.
 *
 * 
 */
void check_data_received(uint8_t m_preset_selection_value, uint8_t * data, uint16_t length)
{
    preset[m_preset_selection_value].FC1             = data[INDEX_FC1];
    preset[m_preset_selection_value].FC2             = data[INDEX_FC2];
    preset[m_preset_selection_value].Q1              = data[INDEX_Q1];
    preset[m_preset_selection_value].Q2              = data[INDEX_Q2];
    preset[m_preset_selection_value].LV1             = data[INDEX_LV1];
    preset[m_preset_selection_value].LV2             = data[INDEX_LV2];
    preset[m_preset_selection_value].STATUS          = data[INDEX_STATUS];
    preset[m_preset_selection_value].MODE            = data[INDEX_MODE];
    preset[m_preset_selection_value].TIME_AUTO_WAH   = data[INDEX_TIME_AUTO_WAH] | (uint16_t)data[INDEX_TIME_AUTO_WAH + 1] << 8;
    preset[m_preset_selection_value].TIME_AUTO_LEVEL = data[INDEX_TIME_AUTO_LEVEL] | (uint16_t)data[INDEX_TIME_AUTO_LEVEL + 1] << 8; 
    preset[m_preset_selection_value].IMPEDANCE       = data[INDEX_IMPEDANCE];
    preset[m_preset_selection_value].COLOR           = data[INDEX_COLOR];
    preset[m_preset_selection_value].HIGH_VOYEL      = data[INDEX_HIGH_VOYEL];
    preset[m_preset_selection_value].LOW_VOYEL       = data[INDEX_LOW_VOYEL];
    preset[m_preset_selection_value].MIX_DRY_WET1    = data[INDEX_MIX_DRY_WET1];
    preset[m_preset_selection_value].MIX_DRY_WET2    = data[INDEX_MIX_DRY_WET2];
    preset[m_preset_selection_value].FILTER_TYPE     = data[INDEX_FILTER_TYPE];
    preset[m_preset_selection_value].SOURCE          = data[INDEX_SOURCE];
    preset[m_preset_selection_value].BYPASS_SOURCE   = data[INDEX_BYPASS_SOURCE];
    preset[m_preset_selection_value].GAIN            = data[INDEX_GAIN];
    strcpy(preset[m_preset_selection_value].NAME,      "");
    strcpy(preset[m_preset_selection_value].NAME,      &data[INDEX_NAME]);

    //Si bit "STATUS" = 1, sauvergarde en flash, sinon, c'est le mode edition (changement on the fly)
    if(preset[m_preset_selection_value].STATUS == PRESET_SAVE_STATUS)
    {
        save_preset2flash(m_preset_selection_value);
        
        //Si un autre preset � le meme nom que celui-ci, il faut aussi le sauver en flash
        check_and_save_same_preset_name(m_preset_selection_value);

        //Update Flash contents by sending notification of all preset (needeed on App side)
        for(uint8_t i = 0; i < PRESET_NUMBER; i++)
        {
          send_notif(i);
        }
        
    ///Si bit "MODE" = 0, Command SPI & I2C chips in real time  
    }else if (preset[m_preset_selection_value].STATUS == PRESET_EDIT_STATUS)
    {
        config_preset();
    }
}

/**@brief Function for adding the Custom Value characteristic.
 *
 * 
 */
void check_and_save_same_preset_name(uint8_t m_preset_selection_value)
{
    for(uint8_t i=0; i<PRESET_NUMBER; i++)
    {
        if(i != m_preset_selection_value)
        {
            if(!strcmp(preset[m_preset_selection_value].NAME, preset[i].NAME))
            {
              NRF_LOG_INFO("PRESET_%d.NAME == PRESET_%d.NAME  (%s) ",  m_preset_selection_value, i, preset[m_preset_selection_value].NAME);

              preset[i].FC1             = preset[m_preset_selection_value].FC1;
              preset[i].FC2             = preset[m_preset_selection_value].FC2;
              preset[i].Q1              = preset[m_preset_selection_value].Q1;
              preset[i].Q2              = preset[m_preset_selection_value].Q2;
              preset[i].LV1             = preset[m_preset_selection_value].LV1;
              preset[i].LV2             = preset[m_preset_selection_value].LV2;
              preset[i].STATUS          = preset[m_preset_selection_value].STATUS;
              preset[i].MODE            = preset[m_preset_selection_value].MODE;
              preset[i].TIME_AUTO_WAH   = preset[m_preset_selection_value].TIME_AUTO_WAH;
              preset[i].TIME_AUTO_LEVEL = preset[m_preset_selection_value].TIME_AUTO_LEVEL;
              preset[i].IMPEDANCE       = preset[m_preset_selection_value].IMPEDANCE;
              preset[i].COLOR           = preset[m_preset_selection_value].COLOR;
              preset[i].HIGH_VOYEL      = preset[m_preset_selection_value].HIGH_VOYEL;
              preset[i].LOW_VOYEL       = preset[m_preset_selection_value].LOW_VOYEL;
              preset[i].MIX_DRY_WET1    = preset[m_preset_selection_value].MIX_DRY_WET1;
              preset[i].MIX_DRY_WET2    = preset[m_preset_selection_value].MIX_DRY_WET2;
              preset[i].FILTER_TYPE     = preset[m_preset_selection_value].FILTER_TYPE;
              preset[i].SOURCE          = preset[m_preset_selection_value].SOURCE;
              preset[i].BYPASS_SOURCE   = preset[m_preset_selection_value].BYPASS_SOURCE;
              preset[i].GAIN            = preset[m_preset_selection_value].GAIN;
  
              save_preset2flash(i);
              //Update Flash contents by sending notification of actual preset
              send_notif(i);

            }
        }
    }
}

/**@brief Function for adding the Custom Value characteristic.
 *
 * 
 */
void send_notif(uint8_t m_preset_selection_value)
{
    switch(m_preset_selection_value)
    {
      case 0:
          preset_1_update(m_wah_service);
        break;
      case 1:
          preset_2_update(m_wah_service);
        break;
      case 2:
          preset_3_update(m_wah_service);
        break;
      case 3:
          preset_4_update(m_wah_service);
        break;
      default:

        break;
    }
}

void reset_config_preset()
{
    uint32_t err_code;

    //Uninit saadc 
    saadc_uninit();

    //Stop Timers
    if(timer_is_running)
    {
        nrf_drv_timer_disable(&TIMER);
        //nrf_drv_timer_uninit(&TIMER);
        timer_is_running = false;
    }

}

/**@brief Function for adding the Custom Value characteristic.
 *
 * 
 */
void config_preset()
{
    uint32_t err_code;

    reset_config_preset();

    //Set GAIN
    uint8_t data_G = preset[m_preset_selection_value].GAIN;
    err_code = drv_AD5263_write(AD5263_ADDR, AD5263_CHANNEL_4, &data_G);
    APP_ERROR_CHECK(err_code);

    //Set Filter type
    set_filter_type(preset[m_preset_selection_value].FILTER_TYPE);

    //Set impedance
    set_impedance(preset[m_preset_selection_value].IMPEDANCE);

    //Check mode
    err_code = check_mode(preset[m_preset_selection_value].MODE);
    APP_ERROR_CHECK(err_code);

    NRF_LOG_INFO("PRESET CONFIGURED");    

    debug_preset(m_preset_selection_value);
}

/*
 *
 * 
 */
void config_calibration()
{
    uint32_t err_code;

    //Set EXTERNAL WAH gain
    set_gain_wah(calibration.GAIN);

    NRF_LOG_INFO("CALIBRATION CONFIGURED");    

    debug_calibration();
}

/*
 *
 * 
 */
void config_stroke()
{
    //Fill vectors
    stroke_response_fill_vectors(EXP, stroke.EXP_CURVE_RESPONSE, stroke.EXP_HEEL, stroke.EXP_TOE);    //To do -> change 0, 1023 by DATA_HELL & TOE from EXP stroke 
    stroke_response_fill_vectors(WAH, stroke.WAH_CURVE_RESPONSE, stroke.WAH_HEEL, stroke.WAH_TOE);    //To do -> change 0, 1023 by DATA_HELL & TOE from EXP stroke

    NRF_LOG_INFO("STROKE CONFIGURED");    

    debug_stroke();
}


/**@brief Function for adding the Custom Value characteristic.
 *
 * 
 */
void update_preset(int data)
{
    uint32_t err_code;
    uint8_t data_F, data_Q, data_L, data_M;
    uint16_t data_min, data_max;

    if(preset[m_preset_selection_value].SOURCE == EXP)
    {
      data_min = calibration.EXP_HEEL; //To do = change calibration.EXP_HEEL/TOE by stroke.EXP_HEEL/TOE
      data_max = calibration.EXP_TOE; //To do = change calibration.EXP_HEEL/TOE by stroke.EXP_HEEL/TOE
    }

    if(preset[m_preset_selection_value].SOURCE == WAH)
    {
      data_min = calibration.WAH_HEEL; //To do = change calibration.EXP_HEEL/TOE by stroke.EXP_HEEL/TOE
      data_max = calibration.WAH_TOE; //To do = change calibration.EXP_HEEL/TOE by stroke.EXP_HEEL/TOE
    }
   
    switch(preset[m_preset_selection_value].MODE)
    {
        case MANUAL_WAH_MODE:
            //Set F
            if(preset[m_preset_selection_value].BYPASS_SOURCE == AUTO)
            {
                auto_bypass_wah_mode(data, data_min, data_max);
            }else{
                map_wah_mode(data, data_min, data_max);
            }
            
          break;

        case MANUAL_LEVEL_MODE:
            //Set F
            data_F = map(data, data_min, data_max, preset[m_preset_selection_value].FC1, preset[m_preset_selection_value].FC2);
            err_code = drv_AD5263_write(AD5263_ADDR, AD5263_CHANNEL_2, &data_F);
            APP_ERROR_CHECK(err_code);
            err_code = drv_AD5263_write(AD5263_ADDR, AD5263_CHANNEL_3, &data_F);
            APP_ERROR_CHECK(err_code);

            //Set Q
            data_Q = map(data, data_min, data_max, preset[m_preset_selection_value].Q1, preset[m_preset_selection_value].Q2);
            err_code = drv_AD5263_write(AD5263_ADDR, AD5263_CHANNEL_1, &data_Q);
            APP_ERROR_CHECK(err_code);

            //Set LV
            data_L = map(data, data_min, data_max, preset[m_preset_selection_value].LV1, preset[m_preset_selection_value].LV2);
            err_code = drv_DS1882_write(DS1882_ADDR, DS1882_CHANNEL_1, &data_L);
            APP_ERROR_CHECK(err_code);

            //Set MIX_DRY_WET
//            data_M = map(data, data_min, data_max, preset[m_preset_selection_value].MIX_DRY_WET1, preset[m_preset_selection_value].MIX_DRY_WET2);
//            err_code = drv_DS1882_write(DS1882_ADDR, DS1882_CHANNEL_2, &data_M);
//            APP_ERROR_CHECK(err_code);
          break;

        case AUTO_WAH_MODE:
            //Set F
            data_F = map(data, 0, 255, preset[m_preset_selection_value].FC1, preset[m_preset_selection_value].FC2);
            err_code = drv_AD5263_write(AD5263_ADDR, AD5263_CHANNEL_2, &data_F);
            APP_ERROR_CHECK(err_code);
            err_code = drv_AD5263_write(AD5263_ADDR, AD5263_CHANNEL_3, &data_F);
            APP_ERROR_CHECK(err_code);
            //NRF_LOG_INFO("data_F %d", data_F);

            //Set Q 
            data_Q = map(data, 0, 255, preset[m_preset_selection_value].Q1, preset[m_preset_selection_value].Q2);
            err_code = drv_AD5263_write(AD5263_ADDR, AD5263_CHANNEL_1, &data_Q);
            APP_ERROR_CHECK(err_code);
            //NRF_LOG_INFO("data_Q %d", data_Q);

            //Set LV
            data_L = map(data, 0, 255, preset[m_preset_selection_value].LV1, preset[m_preset_selection_value].LV2);
            err_code = drv_DS1882_write(DS1882_ADDR, DS1882_CHANNEL_1, &data_L);
            APP_ERROR_CHECK(err_code);
//            NRF_LOG_INFO("data %d", data);
//            NRF_LOG_INFO("data_L %d", data_L);

            //Set MIX_DRY_WET
            data_M = map(data, 0, 255, preset[m_preset_selection_value].MIX_DRY_WET1, preset[m_preset_selection_value].MIX_DRY_WET2);
            err_code = drv_DS1882_write(DS1882_ADDR, DS1882_CHANNEL_2, &data_M);
            APP_ERROR_CHECK(err_code);
            //NRF_LOG_INFO("data_M %d", data_M);
          break;

        case AUTO_LEVEL_MODE:
            //Set F 
            data_F = map(data, 0, 63, preset[m_preset_selection_value].FC1, preset[m_preset_selection_value].FC2);
            err_code = drv_AD5263_write(AD5263_ADDR, AD5263_CHANNEL_2, &data_F);
            APP_ERROR_CHECK(err_code);
            err_code = drv_AD5263_write(AD5263_ADDR, AD5263_CHANNEL_3, &data_F);
            APP_ERROR_CHECK(err_code);

            //Set Q 
            data_Q = map(data, 0, 63, preset[m_preset_selection_value].Q1, preset[m_preset_selection_value].Q2);
            err_code = drv_AD5263_write(AD5263_ADDR, AD5263_CHANNEL_1, &data_Q);
            APP_ERROR_CHECK(err_code);

            //Set LV
            data_L = map(data, 0, 63, preset[m_preset_selection_value].LV1, preset[m_preset_selection_value].LV2);
            err_code = drv_DS1882_write(DS1882_ADDR, DS1882_CHANNEL_1, &data_L);
            APP_ERROR_CHECK(err_code);
//            NRF_LOG_INFO("data %d", data);
//            NRF_LOG_INFO("data_L %d", data_L);

            //Set MIX_DRY_WET
            data_M = map(data, 0, 63, preset[m_preset_selection_value].MIX_DRY_WET1, preset[m_preset_selection_value].MIX_DRY_WET2);
            err_code = drv_DS1882_write(DS1882_ADDR, DS1882_CHANNEL_2, &data_M);
            APP_ERROR_CHECK(err_code);

          break;
        case TALKBOX:

          break;

        case TEST:
            if( (data > 5) && !trigger_up)
            {
                trigger_up = true;
                NRF_LOG_INFO("trigger_up");
                
//                for(int i = 63; i > 0; i-=4)
//                {
                  //NRF_LOG_INFO("i = %d", i);
                  //Set MIX_DRY_WET TO FULL WET
//                  data_M = i;
//                  err_code = drv_DS1882_write(DS1882_ADDR, DS1882_CHANNEL_2, &data_M);
//                  APP_ERROR_CHECK(err_code);
//                  nrf_delay_ms(1);
//                }

                //Set MIX_DRY_WET TO FULL WET
                data_M = 0;
                err_code = drv_DS1882_write(DS1882_ADDR, DS1882_CHANNEL_2, &data_M);
                APP_ERROR_CHECK(err_code);
            }
          
            if((data > 5) && trigger_up)
            {
                trigger_down = false;

                //Set F
                data_F = map(data, 0, SAADC_RES, preset[m_preset_selection_value].FC1, preset[m_preset_selection_value].FC2);
                err_code = drv_AD5263_write(AD5263_ADDR, AD5263_CHANNEL_2, &data_F);
                APP_ERROR_CHECK(err_code);
                err_code = drv_AD5263_write(AD5263_ADDR, AD5263_CHANNEL_3, &data_F);
                APP_ERROR_CHECK(err_code);

                //Set Q
                data_Q = map(data, 0, SAADC_RES, preset[m_preset_selection_value].Q1, preset[m_preset_selection_value].Q2);
                err_code = drv_AD5263_write(AD5263_ADDR, AD5263_CHANNEL_1, &data_Q);
                APP_ERROR_CHECK(err_code);

                //Set LV
                data_L = map(data, 0, SAADC_RES, preset[m_preset_selection_value].LV1, preset[m_preset_selection_value].LV2);
                err_code = drv_DS1882_write(DS1882_ADDR, DS1882_CHANNEL_1, &data_L);
                APP_ERROR_CHECK(err_code);

            }

            if((data < 2) && !trigger_down)
            {
                trigger_down = true;
                NRF_LOG_INFO("trigger_down");

//                for(int i = 0; i < 64; i+=4)
//                {
                  //NRF_LOG_INFO("i = %d", i);
                  //Set MIX_DRY_WET TO FULL WET
//                  data_M = i;
//                  err_code = drv_DS1882_write(DS1882_ADDR, DS1882_CHANNEL_2, &data_M);
//                  APP_ERROR_CHECK(err_code);
//                  nrf_delay_ms(1);
//                }

                //Set MIX_DRY_WET TO FULL WET
                data_M = 63;
                err_code = drv_DS1882_write(DS1882_ADDR, DS1882_CHANNEL_2, &data_M);
                APP_ERROR_CHECK(err_code);
              
            }

            if((data < 2) && trigger_down)
            {
                trigger_up = false;
            }
            
          break;

        default:
          break;
    }
  
    //NRF_LOG_INFO("PRESET UPDATED");    
}

/**@brief Function for adding the Custom Value characteristic.
 *
 * 
 */
void set_filter_type(uint8_t filter_type)
{
    switch(filter_type)
    {
      case BAND_PASS:
          nrf_drv_gpiote_out_clear(F_SELECT_A);
          nrf_drv_gpiote_out_clear(F_SELECT_B);
        break;
      
      case LOW_PASS:
          nrf_drv_gpiote_out_clear(F_SELECT_A);
          nrf_drv_gpiote_out_set(F_SELECT_B);
        break;

      case HIGH_PASS:
          nrf_drv_gpiote_out_set(F_SELECT_A);
          nrf_drv_gpiote_out_clear(F_SELECT_B);
        break;

      case NOTCH:
          nrf_drv_gpiote_out_set(F_SELECT_A);
          nrf_drv_gpiote_out_set(F_SELECT_B);
        break;

      default:
        break;
    }

}


/**@brief Function for adding the Custom Value characteristic.
 *
 * 
 */
void set_gain_wah(uint8_t gain)
{
    switch(gain)
    {
      case 0:
          nrf_drv_gpiote_out_clear(GAIN_WAH_CONTROL_A);
          nrf_drv_gpiote_out_clear(GAIN_WAH_CONTROL_B);
        break;
      
      case 1:
          nrf_drv_gpiote_out_set(GAIN_WAH_CONTROL_A);
          nrf_drv_gpiote_out_clear(GAIN_WAH_CONTROL_B);
        break;

      case 2:
          nrf_drv_gpiote_out_clear(GAIN_WAH_CONTROL_A);
          nrf_drv_gpiote_out_set(GAIN_WAH_CONTROL_B);
        break;

      case 3:
          nrf_drv_gpiote_out_set(GAIN_WAH_CONTROL_A);
          nrf_drv_gpiote_out_set(GAIN_WAH_CONTROL_B);
        break;

      default:
        break;
    }

}


/**@brief Function for adding the Custom Value characteristic.
 *
 * 
 */
void set_impedance(uint8_t impedance)
{
    impedance == LOW_Z ? nrf_drv_gpiote_out_clear(IN_IMPEDANCE) : nrf_drv_gpiote_out_set(IN_IMPEDANCE);
}


/**@brief Function for adding the Custom Value characteristic.
 *
 * 
 */
uint32_t check_mode(uint8_t mode)
{
    ret_code_t err_code;
   
    switch(mode)
    {
        case MANUAL_WAH_MODE:
        case MANUAL_LEVEL_MODE:
            saadc_start(m_wah_service, SAMPLING_20MS, preset[m_preset_selection_value].SOURCE);
          break;
        case AUTO_WAH_MODE:
        case AUTO_LEVEL_MODE:
            timer_start();
          break;
        case TALKBOX:

          break;
        case TEST:
            saadc_start(m_wah_service, SAMPLING_20MS, preset[m_preset_selection_value].SOURCE);
          break;

        default:
          break;
    }

    return NRF_SUCCESS;
}

/**@brief Function 
 *
 */
static void timer_event_handler(nrf_timer_event_t event_type, void* p_context)
{
    ret_code_t err_code;

    //bsp_board_led_invert(5);

    switch (event_type)
    {
        case NRF_TIMER_EVENT_COMPARE0:

            //Test
//            if(preset[m_preset_selection_value].FILTER_TYPE == NOTCH)
//            {
//              if(auto_data_up)
//              {
//                  update_preset(0);
//                  auto_data_up = false;
//              }else
//              {
//                  update_preset(63);
//                  auto_data_up = true;
//              }
//              break;
//            }
            
            if(preset[m_preset_selection_value].MODE == AUTO_WAH_MODE)
            {
//                err_code = app_sched_event_put(0, 0, auto_wah_scheduler_event_handler);
//                APP_ERROR_CHECK(err_code);

                  uint8_t max_value, min_value;

                  if(auto_data_up)
                  {
                      cpt_timer++;
                      update_preset(cpt_timer);
                      if(cpt_timer == preset[m_preset_selection_value].FC2)
                      {
                          auto_data_up = false;
                      }
                  }else
                  {
                      cpt_timer--;
                      update_preset(cpt_timer);
                      if(cpt_timer == preset[m_preset_selection_value].FC1)
                      {
                          auto_data_up = true;
                      }
                  }


            }else if(preset[m_preset_selection_value].MODE == AUTO_LEVEL_MODE)
            {
//                err_code = app_sched_event_put(0, 0, auto_level_scheduler_event_handler);
//                APP_ERROR_CHECK(err_code);

                  if(auto_data_up)
                  {
                      cpt_timer++;
                      update_preset(cpt_timer);
                      if(cpt_timer == preset[m_preset_selection_value].LV2)
                      {
                          auto_data_up = false;
                      }
                  }else
                  {
                      cpt_timer--;
                      update_preset(cpt_timer);
                      if(cpt_timer == preset[m_preset_selection_value].LV1)
                      {
                          auto_data_up = true;
                      }
                  }
                  
            }

                  break;

              default:
                  //Do nothing.
                  break;
          }
}

/**@brief Function 
 *
 */
//static void app_timer_periodic_handler_auto_wah(void * p_context)
//{
//    UNUSED_PARAMETER(p_context);
//
//    ret_code_t err_code;
//    err_code = app_sched_event_put(0, 0, auto_wah_scheduler_event_handler);
//    APP_ERROR_CHECK(err_code);
//}


/**@brief Function 
 *
 */
//static void app_timer_periodic_handler_auto_level(void * p_context)
//{
//    UNUSED_PARAMETER(p_context);
//
//    ret_code_t err_code;
//    err_code = app_sched_event_put(0, 0, auto_level_scheduler_event_handler);
//    APP_ERROR_CHECK(err_code);
//}


/**@brief Function 
 *
 */
void timer_start()
{
    ret_code_t err_code;
    uint32_t min_to_map;
    uint16_t time;
    uint8_t step_nbr;
    uint32_t time_us;
    uint32_t time_ticks;
    cpt_timer = 0;
    auto_data_up = true;
    timer_is_running = true;

    switch(preset[m_preset_selection_value].MODE)
    {
        case AUTO_WAH_MODE:
          step_nbr = (preset[m_preset_selection_value].FC2 - preset[m_preset_selection_value].FC1);
          min_to_map = (step_nbr*600)/1000;                                                                     //temps minimal pour effectuer un balayage (en ms)
          time = map(preset[m_preset_selection_value].TIME_AUTO_WAH, 0, 65535, min_to_map, 1000);
          step_nbr == 0 ? 1 : step_nbr;
          time_us = (time*1000)/step_nbr;
          break;

        case AUTO_LEVEL_MODE:
          step_nbr = (preset[m_preset_selection_value].LV2 - preset[m_preset_selection_value].LV1);
          min_to_map = (step_nbr*600)/1000;
          time = map(preset[m_preset_selection_value].TIME_AUTO_LEVEL, 0, 65535, min_to_map, 1000);
          step_nbr == 0 ? 1 : step_nbr;
          time_us = (time*1000)/step_nbr;
          break;
    }
    
    time_ticks = nrf_drv_timer_us_to_ticks(&TIMER, time_us); 
    //NRF_LOG_INFO("time_ticks %d", time_ticks);

    nrf_drv_timer_extended_compare(
         &TIMER, NRF_TIMER_CC_CHANNEL0, time_ticks, NRF_TIMER_SHORT_COMPARE0_CLEAR_MASK, true);

    nrf_drv_timer_enable(&TIMER);

}

void update_calibration(uint8_t * p_data, uint16_t size)
{
    calibration.SOURCE              = p_data[0];
    calibration.EXP_STATUS          = p_data[1];
    calibration.WAH_STATUS          = p_data[2];
    calibration.DATA                = p_data[3] | (uint16_t)p_data[4] << 8; 
    calibration.GAIN                = p_data[5];

    NRF_LOG_INFO("SOURCE                 = %d", calibration.SOURCE);
    NRF_LOG_INFO("EXP_CALIBRATION_STATUS = %d", calibration.EXP_STATUS);
    NRF_LOG_INFO("WAH_CALIBRATION_STATUS = %d", calibration.WAH_STATUS);
    NRF_LOG_INFO("DATA                   = %d", calibration.DATA);
    NRF_LOG_INFO("GAIN                   = %d", calibration.GAIN);

    if(calibration.SOURCE == EXP)
    {
        NRF_LOG_INFO("EXP_CALIBRATION_INPROGRESS");
        
        switch( calibration.EXP_STATUS )
        {
          case GO_DOWN:
               reset_config_preset();
               saadc_start(m_wah_service, SAMPLING_20MS, EXP); //WAH
            break;

          case GO_UP:
               calibration.EXP_HEEL = calibration.DATA;
            break;

          case DONE:
              calibration.EXP_TOE  = calibration.DATA;
              NRF_LOG_INFO("calibration.EXP_HEEL = %d", calibration.EXP_HEEL);
              NRF_LOG_INFO("calibration.EXP_TOE = %d" , calibration.EXP_TOE);
              write_calibration_done();

              //Copy data in stroke field
              stroke.EXP_HEEL = calibration.EXP_HEEL;
              stroke.EXP_TOE  = calibration.EXP_TOE;
              stroke_response_fill_vectors(EXP, stroke.EXP_CURVE_RESPONSE, stroke.EXP_HEEL, stroke.EXP_TOE);
              write_stroke_done();

              //Send notif to UI with calibration data
              calibration_update(m_wah_service);

              //Restart effect in use
              config_preset();
            break;

          default:
            break;
        }
    } 
   
    if(calibration.SOURCE == WAH)
    {
        NRF_LOG_INFO("WAH_CALIBRATION_INPROGRESS");

        switch( calibration.WAH_STATUS )
        {
          case GO_DOWN:
            reset_config_preset();
            saadc_start(m_wah_service, SAMPLING_20MS, WAH); //WAH
            break;

          case GO_UP:
            if(calib_wah_heel)
            {
              calibration.WAH_HEEL = calibration.DATA;
              calib_wah_heel = false;
            }
            NRF_LOG_INFO("calibration.WAH_HEEL = %d", calibration.WAH_HEEL);
            //Change gain
            set_gain_wah(calibration.GAIN);
            //m_data_heel = get_saadc_data();
            break;

          case DONE:
            calibration.WAH_TOE  = calibration.DATA;
            //m_data_toe = get_saadc_data();
            NRF_LOG_INFO("calibration.WAH_HEEL = %d", calibration.WAH_HEEL);
            NRF_LOG_INFO("calibration.WAH_TOE = %d" , calibration.WAH_TOE);
            write_calibration_done();

            //Copy data in stroke field
            stroke.WAH_HEEL = calibration.WAH_HEEL;
            stroke.WAH_TOE  = calibration.WAH_TOE;
            stroke_response_fill_vectors(WAH, stroke.WAH_CURVE_RESPONSE, stroke.WAH_HEEL, stroke.WAH_TOE);
            write_stroke_done();

            calib_wah_heel = true;

            //Send notif to UI with calibration data
            calibration_update(m_wah_service);

            //Restart effect in use
            config_preset();
            break;

          default:
            break;
        }
    }
}
 
void update_stroke(uint8_t * p_data, uint16_t size)
{
    stroke.STATUS              = p_data[0];
    stroke.SOURCE              = p_data[1];
    stroke.EXP_CURVE_RESPONSE  = p_data[2];
    stroke.WAH_CURVE_RESPONSE  = p_data[3];
//    stroke.EXP_HEEL            = p_data[4]  | (uint16_t)p_data[5]  << 8; 
//    stroke.EXP_TOE             = p_data[6]  | (uint16_t)p_data[7]  << 8;
//    stroke.WAH_HEEL            = p_data[8]  | (uint16_t)p_data[9]  << 8; 
//    stroke.WAH_TOE             = p_data[10] | (uint16_t)p_data[11] << 8;

    NRF_LOG_INFO("STATUS                 = %d", stroke.STATUS);
    NRF_LOG_INFO("SOURCE                 = %d", stroke.SOURCE);
    NRF_LOG_INFO("EXP_CURVE_RESPONSE     = %d", stroke.EXP_CURVE_RESPONSE);
    NRF_LOG_INFO("WAH_CURVE_RESPONSE     = %d", stroke.WAH_CURVE_RESPONSE);
    NRF_LOG_INFO("EXP_HEEL               = %d", stroke.EXP_HEEL);
    NRF_LOG_INFO("EXP_TOE                = %d", stroke.EXP_TOE);
    NRF_LOG_INFO("WAH_HEEL               = %d", stroke.WAH_HEEL);
    NRF_LOG_INFO("WAH_TOE                = %d", stroke.WAH_TOE);

    if(stroke.SOURCE == EXP)
    {
        NRF_LOG_INFO("EXP_STROKE_INPROGRESS");
        
        switch( stroke.STATUS )
        {
          case STROKE_CONFIG:
            break;

          case CURVE_RESPONSE:
              stroke_response_fill_vectors(EXP, stroke.EXP_CURVE_RESPONSE, stroke.EXP_HEEL, stroke.EXP_TOE);  
              write_stroke_done();

              //Send notif to UI with stroke data
              stroke_update(m_wah_service);

            break;

          default:
            break;
      
        }
    }

    if(stroke.SOURCE == WAH)
    {
        NRF_LOG_INFO("WAH_STROKE_INPROGRESS");

        switch( stroke.STATUS )
        {
          case STROKE_CONFIG:  
            break;

          case CURVE_RESPONSE:
              stroke_response_fill_vectors(WAH, stroke.WAH_CURVE_RESPONSE, stroke.WAH_HEEL, stroke.WAH_TOE);  
              write_stroke_done();

              //Send notif to UI with stroke data
              stroke_update(m_wah_service);

            break;

          default:
            break;
      
        }
    }
}

void auto_bypass_wah_mode(uint16_t data, uint16_t data_min, uint16_t data_max)
{   
    uint32_t err_code;
    uint8_t data_F, data_Q, data_L, data_M;

    if( (data > 5) && !trigger_up)
    {
        trigger_up = true;
        NRF_LOG_INFO("trigger_up");

      //                for(int i = 63; i > 0; i-=4)
      //                {
          //NRF_LOG_INFO("i = %d", i);
          //Set MIX_DRY_WET TO FULL WET
      //                  data_M = i;
      //                  err_code = drv_DS1882_write(DS1882_ADDR, DS1882_CHANNEL_2, &data_M);
      //                  APP_ERROR_CHECK(err_code);
      //                  nrf_delay_ms(1);
      //                }

        //Set MIX_DRY_WET TO FULL WET
        data_M = 0;
        err_code = drv_DS1882_write(DS1882_ADDR, DS1882_CHANNEL_2, &data_M);
        APP_ERROR_CHECK(err_code);
    }

    if((data > 5) && trigger_up)
    {
        trigger_down = false;
        map_wah_mode(data, data_min, data_max);
    }

    if((data < 2) && !trigger_down)
    {
        trigger_down = true;
        NRF_LOG_INFO("trigger_down");

      //                for(int i = 0; i < 64; i+=4)
      //                {
          //NRF_LOG_INFO("i = %d", i);
          //Set MIX_DRY_WET TO FULL WET
      //                  data_M = i;
      //                  err_code = drv_DS1882_write(DS1882_ADDR, DS1882_CHANNEL_2, &data_M);
      //                  APP_ERROR_CHECK(err_code);
      //                  nrf_delay_ms(1);
      //                }

        //Set MIX_DRY_WET TO FULL WET
        data_M = 63;
        err_code = drv_DS1882_write(DS1882_ADDR, DS1882_CHANNEL_2, &data_M);
        APP_ERROR_CHECK(err_code);

    }

    if((data < 2) && trigger_down)
    {
        trigger_up = false;
    }


}

void map_wah_mode(uint16_t data, uint16_t data_min, uint16_t data_max)
{
    uint32_t err_code;
    uint8_t data_F, data_Q, data_L, data_M;

    //Set F
    data_F = map(data, data_min, data_max, preset[m_preset_selection_value].FC1, preset[m_preset_selection_value].FC2);
    err_code = drv_AD5263_write(AD5263_ADDR, AD5263_CHANNEL_2, &data_F);
    APP_ERROR_CHECK(err_code);
    err_code = drv_AD5263_write(AD5263_ADDR, AD5263_CHANNEL_3, &data_F);
    APP_ERROR_CHECK(err_code);

    //Set Q
    data_Q =map(data, data_min, data_max, preset[m_preset_selection_value].Q1, preset[m_preset_selection_value].Q2);  
    err_code = drv_AD5263_write(AD5263_ADDR, AD5263_CHANNEL_1, &data_Q);
    APP_ERROR_CHECK(err_code);

    //Set LV 
    data_L = map(data, data_min, data_max, preset[m_preset_selection_value].LV1, preset[m_preset_selection_value].LV2);
    err_code = drv_DS1882_write(DS1882_ADDR, DS1882_CHANNEL_1, &data_L);
    APP_ERROR_CHECK(err_code);

                //Set MIX_DRY_WET
    //            data_M = map(data, data_min, data_max, preset[m_preset_selection_value].MIX_DRY_WET1, preset[m_preset_selection_value].MIX_DRY_WET2);
    //            err_code = drv_DS1882_write(DS1882_ADDR, DS1882_CHANNEL_2, &data_M);
    //            APP_ERROR_CHECK(err_code);
}

void debug_preset (uint8_t m_preset_selection_value)
{
    #ifdef DEBUG_PRESET_RUNTIME
      NRF_LOG_INFO("************ACTIVEPRESET_STRUCTURE**************");
      NRF_LOG_INFO("PRESET_              %d", m_preset_selection_value);   
      NRF_LOG_INFO("FC1 =                %d", preset[m_preset_selection_value].FC1);     
      NRF_LOG_INFO("FC2 =                %d", preset[m_preset_selection_value].FC2);
      NRF_LOG_INFO("Q1 =                 %d", preset[m_preset_selection_value].Q1); 
      NRF_LOG_INFO("Q2 =                 %d", preset[m_preset_selection_value].Q2);    
      NRF_LOG_INFO("LV1 =                %d", preset[m_preset_selection_value].LV1);
      NRF_LOG_INFO("LV2 =                %d", preset[m_preset_selection_value].LV2);
      NRF_LOG_INFO("STATUS =             %d", preset[m_preset_selection_value].STATUS);      
      NRF_LOG_INFO("MODE =               %d", preset[m_preset_selection_value].MODE);     
      NRF_LOG_INFO("TIME_AUTO_WAH =      %d", preset[m_preset_selection_value].TIME_AUTO_WAH);      
      NRF_LOG_INFO("TIME_AUTO_LEVEL =    %d", preset[m_preset_selection_value].TIME_AUTO_LEVEL);     
      NRF_LOG_INFO("IMPEDANCE =          %d", preset[m_preset_selection_value].IMPEDANCE);       
      NRF_LOG_INFO("COLOR =              %d", preset[m_preset_selection_value].COLOR);       
      NRF_LOG_INFO("HIGH_VOYEL =         %d", preset[m_preset_selection_value].HIGH_VOYEL);      
      NRF_LOG_INFO("LOW_VOYEL =          %d", preset[m_preset_selection_value].LOW_VOYEL);    
      NRF_LOG_INFO("MIX_DRY_WET1 =       %d", preset[m_preset_selection_value].MIX_DRY_WET1);       
      NRF_LOG_INFO("MIX_DRY_WET2 =       %d", preset[m_preset_selection_value].MIX_DRY_WET2);      
      NRF_LOG_INFO("FILTER_TYPE =        %d", preset[m_preset_selection_value].FILTER_TYPE);     
      NRF_LOG_INFO("SOURCE =             %d", preset[m_preset_selection_value].SOURCE);  
      NRF_LOG_INFO("BYPASS_SOURCE =      %d", preset[m_preset_selection_value].BYPASS_SOURCE);       
      NRF_LOG_INFO("GAIN =               %d", preset[m_preset_selection_value].GAIN);  
      NRF_LOG_INFO("NAME =               %s", preset[m_preset_selection_value].NAME);  
      NRF_LOG_INFO("********************************************************");
    #endif
}

void debug_calibration ()
{
    #ifdef DEBUG_PRESET_RUNTIME
      NRF_LOG_INFO("************ACTIVE CALIBRATION_STRUCTURE**************");
      NRF_LOG_INFO("SOURCE =                                %d", calibration.SOURCE);
      NRF_LOG_INFO("EXP_STATUS =                            %d", calibration.EXP_STATUS);
      NRF_LOG_INFO("WAH_STATUS =                            %d", calibration.WAH_STATUS);
      NRF_LOG_INFO("DATA =                                  %d", calibration.DATA);
      NRF_LOG_INFO("GAIN =                                  %d", calibration.GAIN);
      NRF_LOG_INFO("EXP_HEEL =                              %d", calibration.EXP_HEEL);
      NRF_LOG_INFO("EXP_TOE =                               %d", calibration.EXP_TOE);
      NRF_LOG_INFO("WAH_HEEL =                              %d", calibration.WAH_HEEL);
      NRF_LOG_INFO("WAH_TOE =                               %d", calibration.WAH_TOE);
      NRF_LOG_INFO("********************************************************");
    #endif
}

void debug_stroke ()
{
    #ifdef DEBUG_PRESET_RUNTIME
      NRF_LOG_INFO("************ACTIVE STROKE_STRUCTURE***************");
      NRF_LOG_INFO("STATUS                 = %d", stroke.STATUS);
      NRF_LOG_INFO("SOURCE                 = %d", stroke.SOURCE);
      NRF_LOG_INFO("EXP_CURVE_RESPONSE     = %d", stroke.EXP_CURVE_RESPONSE);
      NRF_LOG_INFO("WAH_CURVE_RESPONSE     = %d", stroke.WAH_CURVE_RESPONSE);
      NRF_LOG_INFO("EXP_HEEL               = %d", stroke.EXP_HEEL);
      NRF_LOG_INFO("EXP_TOE                = %d", stroke.EXP_TOE);
      NRF_LOG_INFO("WAH_HEEL               = %d", stroke.WAH_HEEL);
      NRF_LOG_INFO("WAH_TOE                = %d", stroke.WAH_TOE);
      NRF_LOG_INFO("********************************************************");
    #endif
}

