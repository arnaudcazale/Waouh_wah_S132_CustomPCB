#include "utils.h"



//static bool flash_writing = false;
//static bool flash_initializing = false;
static fds_record_desc_t    my_record_desc;

static preset_config_32_t preset_32[PRESET_NUMBER];
preset_config_8_t preset[PRESET_NUMBER];
calib_config_32_t calibration_32;
calib_config_8_t calibration;
stroke_config_32_t stroke_32;
stroke_config_t stroke;

/*******************************************************************************

*******************************************************************************/
void update_led(uint8_t led) 
{
      bsp_board_leds_presets_off();
      bsp_board_led_on(led);
}

/*******************************************************************************

*******************************************************************************/
void load_presets_from_flash(bool restore_factory)
{
    ret_code_t err_code;
    //flash_initializing = true;
    err_code = fds_test_init();
    //while(!flash_initializing);
    APP_ERROR_CHECK(err_code);
  
    //verify if something is set into memory
    err_code = check_memory();
    if(err_code != NRF_SUCCESS )
    {
        NRF_LOG_INFO("Memory empty");
        err_code = write_factory_presets();
        if (err_code == NRF_SUCCESS){
            NRF_LOG_INFO("WRITE FACTORY PRESET SUCCESS");              
        }else{
            NRF_LOG_INFO("ERROR WRITE FACTORY PRESET"); 
        }
    }

    if(restore_factory)
    {
        err_code = write_factory_presets();
        if (err_code == NRF_SUCCESS){
            NRF_LOG_INFO("WRITE FACTORY PRESET SUCCESS");              
        }else{
            NRF_LOG_INFO("ERROR WRITE FACTORY PRESET"); 
        }
    }
    

    NRF_LOG_INFO("Loading_Presets...");
    load_flash_config();

    
    
}

/*******************************************************************************
* Function Name  : init handler
* Description    : 
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
ret_code_t fds_test_init (void)
{
    ret_code_t ret = fds_register(my_fds_evt_handler);
    if (ret != FDS_SUCCESS)
    {
                    return ret;
                    
    }
    ret = fds_init();
    if (ret != FDS_SUCCESS)
    {
                    return ret;
    }
    
    return NRF_SUCCESS;
}

/*******************************************************************************
* Function Name  : event handler
* Description    : 
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
static void my_fds_evt_handler(fds_evt_t const * const p_fds_evt)
{

    //NRF_LOG_INFO("FDS handler - %d - %d\r\n", p_fds_evt->id, p_fds_evt->result);

    switch (p_fds_evt->id)
    {
        case FDS_EVT_INIT:
            if (p_fds_evt->result == FDS_SUCCESS)
            {
                //NRF_LOG_INFO("INIT SUCCESS");
                //flash_initializing = false;
            }
            break;
        case FDS_EVT_WRITE:
            if (p_fds_evt->result == FDS_SUCCESS)
            {
                //flash_writing = false;
                //NRF_LOG_INFO("WRITE SUCCESS");
            }
            break;
        case FDS_EVT_UPDATE:
            if (p_fds_evt->result == FDS_SUCCESS)
            {
                //flash_writing = false;
                //NRF_LOG_INFO("UPDATE SUCCESS");
            }
            break;
        case FDS_EVT_DEL_RECORD:
            if (p_fds_evt->result == FDS_SUCCESS)
            {
               //NRF_LOG_INFO("delete record sucess \n\r");
            }
        break;
        case FDS_EVT_GC:     
            if (p_fds_evt->result == FDS_SUCCESS)
            { 
               //NRF_LOG_INFO("Garbage collector succes \n\r");
            }
            break;
        default:
            //NRF_LOG_INFO("m_FDS handler - %d - %d\r\n", p_fds_evt->id, p_fds_evt->result);
            break;
    }
}

/*******************************************************************************

*******************************************************************************/
ret_code_t check_memory()
{
  //Check if the first record exists
  fds_flash_record_t  flash_record;
  fds_record_desc_t   record_desc;
  fds_find_token_t    ftok;
  /* It is required to zero the token before first use. */
  memset(&ftok, 0x00, sizeof(fds_find_token_t));
  
  ret_code_t err_code = fds_record_find(FILE_ID_PRST_BASE, RECORD_KEY_PRST, &record_desc, &ftok);
  return err_code;
}

/*******************************************************************************

*******************************************************************************/
ret_code_t write_factory_presets()
{
    uint32_t err_code;
    uint8_t idx_prst; 

    preset_32[0].FC1             = 0;
    preset_32[0].FC2             = 255;
    preset_32[0].Q1              = 128;
    preset_32[0].Q2              = 128;
    preset_32[0].LV1             = 0;
    preset_32[0].LV2             = 63;
    preset_32[0].STATUS          = PRESET_EDIT_STATUS;
    preset_32[0].MODE            = AUTO_WAH_MODE;
    preset_32[0].TIME_AUTO_WAH   = 64000;
    preset_32[0].TIME_AUTO_LEVEL = 32000;
    preset_32[0].IMPEDANCE       = LOW_Z;
    preset_32[0].COLOR           = COLOR_1;
    preset_32[0].HIGH_VOYEL      = ae;
    preset_32[0].LOW_VOYEL       = uh;
    preset_32[0].MIX_DRY_WET1    = 0;
    preset_32[0].MIX_DRY_WET2    = 0;
    preset_32[0].FILTER_TYPE     = LOW_PASS;
    preset_32[0].SOURCE          = EXP;
    preset_32[0].BYPASS_SOURCE   = INTERNAL;
    preset_32[0].GAIN            = 128;
    strcpy(preset_32[0].NAME, "PRESET_1");

    //flash_writing = true;
    write_preset_config(0);
    //while(!flash_writing);

    preset_32[1].FC1             = 128;
    preset_32[1].FC2             = 128;
    preset_32[1].Q1              = 128;
    preset_32[1].Q2              = 128;
    preset_32[1].LV1             = 0;
    preset_32[1].LV2             = 63;
    preset_32[1].STATUS          = PRESET_EDIT_STATUS;
    preset_32[1].MODE            = AUTO_LEVEL_MODE;
    preset_32[1].TIME_AUTO_WAH   = 32000;
    preset_32[1].TIME_AUTO_LEVEL = 64000;
    preset_32[1].IMPEDANCE       = LOW_Z;
    preset_32[1].COLOR           = COLOR_2;
    preset_32[1].HIGH_VOYEL      = ae;
    preset_32[1].LOW_VOYEL       = uh;
    preset_32[1].MIX_DRY_WET1    = 0;
    preset_32[1].MIX_DRY_WET2    = 0;
    preset_32[1].FILTER_TYPE     = LOW_PASS;
    preset_32[1].SOURCE          = EXP;
    preset_32[1].BYPASS_SOURCE   = INTERNAL;
    preset_32[1].GAIN            = 128;
    strcpy(preset_32[1].NAME, "PRESET_2");

    //flash_writing = true;
    write_preset_config(1);
    //while(flash_writing);

    preset_32[2].FC1             = 50;
    preset_32[2].FC2             = 200;
    preset_32[2].Q1              = 0;
    preset_32[2].Q2              = 255;
    preset_32[2].LV1             = 0;
    preset_32[2].LV2             = 0;
    preset_32[2].STATUS          = PRESET_EDIT_STATUS;
    preset_32[2].MODE            = MANUAL_WAH_MODE;
    preset_32[2].TIME_AUTO_WAH   = 32000;
    preset_32[2].TIME_AUTO_LEVEL = 32000;
    preset_32[2].IMPEDANCE       = LOW_Z;
    preset_32[2].COLOR           = COLOR_3;
    preset_32[2].HIGH_VOYEL      = ae;
    preset_32[2].LOW_VOYEL       = uh;
    preset_32[2].MIX_DRY_WET1    = 0;
    preset_32[2].MIX_DRY_WET2    = 0;
    preset_32[2].FILTER_TYPE     = LOW_PASS;
    preset_32[2].SOURCE          = EXP;
    preset_32[2].BYPASS_SOURCE   = INTERNAL;
    preset_32[2].GAIN            = 128;
    strcpy(preset_32[2].NAME, "PRESET_3");

    //flash_writing = true;
    write_preset_config(2);
    //while(flash_writing);

    preset_32[3].FC1             = 128;
    preset_32[3].FC2             = 128;
    preset_32[3].Q1              = 128;
    preset_32[3].Q2              = 128;
    preset_32[3].LV1             = 0;
    preset_32[3].LV2             = 63;
    preset_32[3].STATUS          = PRESET_EDIT_STATUS;
    preset_32[3].MODE            = MANUAL_LEVEL_MODE;
    preset_32[3].TIME_AUTO_WAH   = 32000;
    preset_32[3].TIME_AUTO_LEVEL = 32000;
    preset_32[3].IMPEDANCE       = LOW_Z;
    preset_32[3].COLOR           = COLOR_4;
    preset_32[3].HIGH_VOYEL      = ae;
    preset_32[3].LOW_VOYEL       = uh;
    preset_32[3].MIX_DRY_WET1    = 0;
    preset_32[3].MIX_DRY_WET2    = 0;
    preset_32[3].FILTER_TYPE     = LOW_PASS;
    preset_32[3].SOURCE          = WAH;
    preset_32[3].BYPASS_SOURCE   = INTERNAL;
    preset_32[3].GAIN            = 128;
    strcpy(preset_32[3].NAME, "PRESET_4");

    //flash_writing = true;
    write_preset_config(3);
    //while(flash_writing);

    calibration_32.EXP_STATUS = NONE;
    calibration_32.WAH_STATUS = NONE;
    calibration_32.DATA       = 0;
    calibration_32.GAIN       = 0;
    calibration_32.EXP_HEEL   = 0;
    calibration_32.EXP_TOE    = 1023;
    calibration_32.WAH_HEEL   = 0;
    calibration_32.WAH_TOE    = 1023;

    //flash_writing = true;
    write_calibration_config();
    //while(flash_writing);

    stroke_32.STATUS             = STROKE_CONFIG;
    stroke_32.SOURCE             = EXP;
    stroke_32.EXP_CURVE_RESPONSE = RAW;
    stroke_32.WAH_CURVE_RESPONSE = RAW;
    stroke_32.EXP_HEEL           = calibration_32.EXP_HEEL;
    stroke_32.EXP_TOE            = calibration_32.EXP_TOE;
    stroke_32.WAH_HEEL           = calibration_32.WAH_HEEL;
    stroke_32.WAH_TOE            = calibration_32.WAH_TOE;


    //flash_writing = true;
    write_stroke_config();
    //while(flash_writing);

    return NRF_SUCCESS;

}

/*******************************************************************************

*******************************************************************************/
void write_preset_config(uint8_t idx_prst)
{
   uint32_t err_code;
   uint16_t file_id;
   file_id = FILE_ID_PRST_BASE + idx_prst;
   err_code = m_fds_write_preset(file_id, RECORD_KEY_PRST, &preset_32[idx_prst]);
   APP_ERROR_CHECK(err_code);
}

/*******************************************************************************

*******************************************************************************/
void write_calibration_config()
{
   uint32_t err_code;
   err_code = m_fds_write_calibration(FILE_ID_CALIB, RECORD_KEY_CALIB, &calibration_32);
   APP_ERROR_CHECK(err_code);
}

/*******************************************************************************

*******************************************************************************/
void write_stroke_config()
{
   uint32_t err_code;
   err_code = m_fds_write_stroke(FILE_ID_STROKE, RECORD_KEY_STROKE, &stroke_32);
   APP_ERROR_CHECK(err_code);
}

/*******************************************************************************

*******************************************************************************/
void write_calibration_done()
{
    calibration_32.EXP_STATUS         = calibration.EXP_STATUS;
    calibration_32.WAH_STATUS         = calibration.WAH_STATUS;
    calibration_32.DATA               = calibration.DATA;
    calibration_32.GAIN               = calibration.GAIN;
    calibration_32.EXP_HEEL           = calibration.EXP_HEEL;
    calibration_32.EXP_TOE            = calibration.EXP_TOE;
    calibration_32.WAH_HEEL           = calibration.WAH_HEEL;
    calibration_32.WAH_TOE            = calibration.WAH_TOE;

    //flash_writing = true;
    write_calibration_config();
    //while(flash_writing);
}

/*******************************************************************************

*******************************************************************************/
void write_stroke_done()
{
    stroke_32.STATUS             = stroke.STATUS;
    stroke_32.SOURCE             = stroke.SOURCE;
    stroke_32.EXP_CURVE_RESPONSE = stroke.EXP_CURVE_RESPONSE;
    stroke_32.WAH_CURVE_RESPONSE = stroke.WAH_CURVE_RESPONSE;
    stroke_32.EXP_HEEL           = stroke.EXP_HEEL;
    stroke_32.EXP_TOE            = stroke.EXP_TOE;
    stroke_32.WAH_HEEL           = stroke.WAH_HEEL;
    stroke_32.WAH_TOE            = stroke.WAH_TOE;

    //flash_writing = true;
    write_stroke_config();
    //while(flash_writing);
}


/*******************************************************************************

*******************************************************************************/
ret_code_t m_fds_write_preset(uint16_t FILE_ID, uint16_t RECORD_KEY, preset_config_32_t * write_data)
{ 
    ret_code_t ret;

    fds_record_t        record;
    fds_record_desc_t   record_desc;

    fds_record_desc_t desc = {0};
    fds_find_token_t  tok  = {0};

    record.file_id              = FILE_ID;
    record.key                  = RECORD_KEY;
    record.data.p_data          = write_data;  
    record.data.length_words    = ( sizeof(preset_config_32_t) + 3 ) / sizeof(uint32_t);
  
    ret_code_t rc = fds_record_find(FILE_ID, RECORD_KEY , &desc, &tok);  //Search for record. Update if found else write a new one

    if (rc == FDS_SUCCESS)
    {   
        //NRF_LOG_INFO("PRESET Variables Record was found thus it is being updated");
        rc = fds_record_update(&desc, &record);
        //APP_ERROR_CHECK(rc);
        if(rc == FDS_ERR_NO_SPACE_IN_FLASH)
        {
             ret_code_t rc = fds_gc();// try to do garbage collection
             APP_ERROR_CHECK(rc);
        }

    }
    else
    {
        ret_code_t ret = fds_record_write(&record_desc, &record);
        APP_ERROR_CHECK(ret);
        //NRF_LOG_INFO("First time writing PRESET record");
    }

    return NRF_SUCCESS;
    
}

/*******************************************************************************

*******************************************************************************/
ret_code_t m_fds_write_calibration(uint16_t FILE_ID, uint16_t RECORD_KEY, calib_config_32_t * write_data)
{ 
    ret_code_t ret;

    fds_record_t        record;
    fds_record_desc_t   record_desc;

    fds_record_desc_t desc = {0};
    fds_find_token_t  tok  = {0};

    record.file_id              = FILE_ID;
    record.key                  = RECORD_KEY;
    record.data.p_data          = write_data;  
    record.data.length_words    = ( sizeof(calib_config_32_t) + 3 ) / sizeof(uint32_t);
  
    ret_code_t rc = fds_record_find(FILE_ID, RECORD_KEY , &desc, &tok);  //Search for record. Update if found else write a new one

    if (rc == FDS_SUCCESS)
    {   
        //NRF_LOG_INFO("PRESET Variables Record was found thus it is being updated");
        rc = fds_record_update(&desc, &record);
        //APP_ERROR_CHECK(rc);
        if(rc == FDS_ERR_NO_SPACE_IN_FLASH)
        {
             ret_code_t rc = fds_gc();// try to do garbage collection
             APP_ERROR_CHECK(rc);
        }

    }
    else
    {
        ret_code_t ret = fds_record_write(&record_desc, &record);
        APP_ERROR_CHECK(ret);
        //NRF_LOG_INFO("First time writing PRESET record");
    }

    return NRF_SUCCESS;
    
}

/*******************************************************************************

*******************************************************************************/
ret_code_t m_fds_write_stroke(uint16_t FILE_ID, uint16_t RECORD_KEY, stroke_config_32_t * write_data)
{ 
    ret_code_t ret;

    fds_record_t        record;
    fds_record_desc_t   record_desc;

    fds_record_desc_t desc = {0};
    fds_find_token_t  tok  = {0};

    record.file_id              = FILE_ID;
    record.key                  = RECORD_KEY;
    record.data.p_data          = write_data;  
    record.data.length_words    = ( sizeof(stroke_config_32_t) + 3 ) / sizeof(uint32_t);
  
    ret_code_t rc = fds_record_find(FILE_ID, RECORD_KEY , &desc, &tok);  //Search for record. Update if found else write a new one

    if (rc == FDS_SUCCESS)
    {   
        //NRF_LOG_INFO("PRESET Variables Record was found thus it is being updated");
        rc = fds_record_update(&desc, &record);
        //APP_ERROR_CHECK(rc);
        if(rc == FDS_ERR_NO_SPACE_IN_FLASH)
        {
             ret_code_t rc = fds_gc();// try to do garbage collection
             APP_ERROR_CHECK(rc);
        }

    }
    else
    {
        ret_code_t ret = fds_record_write(&record_desc, &record);
        APP_ERROR_CHECK(ret);
        //NRF_LOG_INFO("First time writing PRESET record");
    }

    return NRF_SUCCESS;
    
}

/*******************************************************************************

*******************************************************************************/
void load_flash_config()
{
    preset_config_32_t * data;
    uint16_t file_id;
   
    for(uint8_t idx_prst=0; idx_prst<PRESET_NUMBER; idx_prst++)
    {
        file_id = FILE_ID_PRST_BASE + idx_prst;
        data = m_fds_read_preset(file_id, RECORD_KEY_PRST);

        preset_32[idx_prst].FC1             = data->FC1;
        preset_32[idx_prst].FC2             = data->FC2;
        preset_32[idx_prst].Q1              = data->Q1;
        preset_32[idx_prst].Q2              = data->Q2;
        preset_32[idx_prst].LV1             = data->LV1;
        preset_32[idx_prst].LV2             = data->LV2;
        preset_32[idx_prst].STATUS          = data->STATUS;
        preset_32[idx_prst].MODE            = data->MODE;
        preset_32[idx_prst].TIME_AUTO_WAH   = data->TIME_AUTO_WAH;
        preset_32[idx_prst].TIME_AUTO_LEVEL = data->TIME_AUTO_LEVEL;
        preset_32[idx_prst].IMPEDANCE       = data->IMPEDANCE;
        preset_32[idx_prst].COLOR           = data->COLOR;
        preset_32[idx_prst].HIGH_VOYEL      = data->HIGH_VOYEL;
        preset_32[idx_prst].LOW_VOYEL       = data->LOW_VOYEL;
        preset_32[idx_prst].MIX_DRY_WET1    = data->MIX_DRY_WET1;
        preset_32[idx_prst].MIX_DRY_WET2    = data->MIX_DRY_WET2;
        preset_32[idx_prst].FILTER_TYPE     = data->FILTER_TYPE;
        preset_32[idx_prst].SOURCE          = data->SOURCE;
        preset_32[idx_prst].BYPASS_SOURCE   = data->BYPASS_SOURCE;
        preset_32[idx_prst].GAIN            = data->GAIN;
        strcpy(preset_32[idx_prst].NAME,      data->NAME);

        preset[idx_prst].FC1             = preset_32[idx_prst].FC1;
        preset[idx_prst].FC2             = preset_32[idx_prst].FC2;
        preset[idx_prst].Q1              = preset_32[idx_prst].Q1;
        preset[idx_prst].Q2              = preset_32[idx_prst].Q2;
        preset[idx_prst].LV1             = preset_32[idx_prst].LV1;
        preset[idx_prst].LV2             = preset_32[idx_prst].LV2;
        preset[idx_prst].STATUS          = preset_32[idx_prst].STATUS;
        preset[idx_prst].MODE            = preset_32[idx_prst].MODE;
        preset[idx_prst].TIME_AUTO_WAH   = preset_32[idx_prst].TIME_AUTO_WAH;
        preset[idx_prst].TIME_AUTO_LEVEL = preset_32[idx_prst].TIME_AUTO_LEVEL;
        preset[idx_prst].IMPEDANCE       = preset_32[idx_prst].IMPEDANCE;
        preset[idx_prst].COLOR           = preset_32[idx_prst].COLOR;
        preset[idx_prst].HIGH_VOYEL      = preset_32[idx_prst].HIGH_VOYEL;
        preset[idx_prst].LOW_VOYEL       = preset_32[idx_prst].LOW_VOYEL;
        preset[idx_prst].MIX_DRY_WET1    = preset_32[idx_prst].MIX_DRY_WET1;
        preset[idx_prst].MIX_DRY_WET2    = preset_32[idx_prst].MIX_DRY_WET2;
        preset[idx_prst].FILTER_TYPE     = preset_32[idx_prst].FILTER_TYPE;
        preset[idx_prst].SOURCE          = preset_32[idx_prst].SOURCE;
        preset[idx_prst].BYPASS_SOURCE   = preset_32[idx_prst].BYPASS_SOURCE;
        preset[idx_prst].GAIN            = preset_32[idx_prst].GAIN;
        strcpy(preset[idx_prst].NAME,      preset_32[idx_prst].NAME);

        #ifdef DEBUG_PRESET_FLASH
          NRF_LOG_INFO("***************************************");        
          NRF_LOG_INFO("PRESET_              %d", idx_prst);
          NRF_LOG_INFO("FC1 =                %d", preset[idx_prst].FC1);
          NRF_LOG_INFO("FC2 =                %d", preset[idx_prst].FC2);
          NRF_LOG_INFO("Q1 =                 %d", preset[idx_prst].Q1);
          NRF_LOG_INFO("Q2 =                 %d", preset[idx_prst].Q2); 
          NRF_LOG_INFO("LV1 =                %d", preset[idx_prst].LV1);
          NRF_LOG_INFO("LV2 =                %d", preset[idx_prst].LV2);
          NRF_LOG_INFO("STATUS =             %d", preset[idx_prst].STATUS);
          NRF_LOG_INFO("MODE =               %d", preset[idx_prst].MODE); 
          NRF_LOG_INFO("TIME_AUTO_WAH =      %d", preset[idx_prst].TIME_AUTO_WAH); 
          NRF_LOG_INFO("TIME_AUTO_LEVEL =    %d", preset[idx_prst].TIME_AUTO_LEVEL);
          NRF_LOG_INFO("IMPEDANCE =          %d", preset[idx_prst].IMPEDANCE); 
          NRF_LOG_INFO("COLOR =              %d", preset[idx_prst].COLOR); 
          NRF_LOG_INFO("HIGH_VOYEL =         %d", preset[idx_prst].HIGH_VOYEL); 
          NRF_LOG_INFO("LOW_VOYEL =          %d", preset[idx_prst].LOW_VOYEL);
          NRF_LOG_INFO("MIX_DRY_WET1 =       %d", preset[idx_prst].MIX_DRY_WET1); 
          NRF_LOG_INFO("MIX_DRY_WET2 =       %d", preset[idx_prst].MIX_DRY_WET2); 
          NRF_LOG_INFO("FILTER_TYPE =        %d", preset[idx_prst].FILTER_TYPE);
          NRF_LOG_INFO("SOURCE =             %d", preset[idx_prst].SOURCE); 
          NRF_LOG_INFO("BYPASS_SOURCE =      %d", preset[idx_prst].BYPASS_SOURCE);
          NRF_LOG_INFO("GAIN =               %d", preset[idx_prst].GAIN); 
          NRF_LOG_INFO("NAME =               %s", preset[idx_prst].NAME); 
        #endif

    }

    //Load Calibration Config
    calib_config_32_t * data_calib;

    data_calib = m_fds_read_calibration(FILE_ID_CALIB, RECORD_KEY_CALIB);

    calibration_32.EXP_STATUS                       = data_calib->EXP_STATUS;
    calibration_32.WAH_STATUS                       = data_calib->WAH_STATUS;
    calibration_32.DATA                             = data_calib->DATA;
    calibration_32.GAIN                             = data_calib->GAIN;
    calibration_32.SOURCE                           = data_calib->SOURCE;
    calibration_32.EXP_HEEL                         = data_calib->EXP_HEEL;
    calibration_32.EXP_TOE                          = data_calib->EXP_TOE;
    calibration_32.WAH_HEEL                         = data_calib->WAH_HEEL;
    calibration_32.WAH_TOE                          = data_calib->WAH_TOE;

    calibration.EXP_STATUS                          = calibration_32.EXP_STATUS;
    calibration.WAH_STATUS                          = calibration_32.WAH_STATUS;
    calibration.DATA                                = calibration_32.DATA;
    calibration.GAIN                                = calibration_32.GAIN;
    calibration.SOURCE                              = calibration_32.SOURCE;
    calibration.EXP_HEEL                            = calibration_32.EXP_HEEL;
    calibration.EXP_TOE                             = calibration_32.EXP_TOE;
    calibration.WAH_HEEL                            = calibration_32.WAH_HEEL;
    calibration.WAH_TOE                             = calibration_32.WAH_TOE;

    #ifdef DEBUG_PRESET_FLASH
          NRF_LOG_INFO("***************************************");        
          NRF_LOG_INFO("CALIBRATION");
          NRF_LOG_INFO("SOURCE =                                %d", calibration.SOURCE);
          NRF_LOG_INFO("EXP_STATUS =                            %d", calibration.EXP_STATUS);
          NRF_LOG_INFO("WAH_STATUS =                            %d", calibration.WAH_STATUS);
          NRF_LOG_INFO("DATA =                                  %d", calibration.DATA);
          NRF_LOG_INFO("GAIN =                                  %d", calibration.GAIN);
          NRF_LOG_INFO("EXP_HEEL =                              %d", calibration.EXP_HEEL);
          NRF_LOG_INFO("EXP_TOE =                               %d", calibration.EXP_TOE);
          NRF_LOG_INFO("WAH_HEEL =                              %d", calibration.WAH_HEEL);
          NRF_LOG_INFO("WAH_TOE =                               %d", calibration.WAH_TOE);
    #endif

    //Load Calibration Config
    stroke_config_32_t * data_stroke;

    data_stroke = m_fds_read_stroke(FILE_ID_STROKE, RECORD_KEY_STROKE);

    stroke_32.STATUS                = data_stroke->STATUS;
    stroke_32.SOURCE                = data_stroke->SOURCE;
    stroke_32.EXP_CURVE_RESPONSE    = data_stroke->EXP_CURVE_RESPONSE;
    stroke_32.WAH_CURVE_RESPONSE    = data_stroke->WAH_CURVE_RESPONSE;
    stroke_32.EXP_HEEL              = data_stroke->EXP_HEEL;
    stroke_32.EXP_TOE               = data_stroke->EXP_TOE;
    stroke_32.WAH_HEEL              = data_stroke->WAH_HEEL;
    stroke_32.WAH_TOE               = data_stroke->WAH_TOE;

    stroke.STATUS                = stroke_32.STATUS;
    stroke.SOURCE                = stroke_32.SOURCE;
    stroke.EXP_CURVE_RESPONSE    = stroke_32.EXP_CURVE_RESPONSE;
    stroke.WAH_CURVE_RESPONSE    = stroke_32.WAH_CURVE_RESPONSE;
    stroke.EXP_HEEL              = stroke_32.EXP_HEEL;
    stroke.EXP_TOE               = stroke_32.EXP_TOE;
    stroke.WAH_HEEL              = stroke_32.WAH_HEEL;
    stroke.WAH_TOE               = stroke_32.WAH_TOE;

    //Fill vectors
    //stroke_response_fill_vectors(stroke.SOURCE, stroke.EXP_CURVE_RESPONSE, stroke.EXP_HEEL, stroke.EXP_TOE);    //To do -> change 0, 1023 by DATA_HELL & TOE from EXP stroke 
    //stroke_response_fill_vectors(stroke.SOURCE, stroke.WAH_CURVE_RESPONSE, stroke.WAH_HEEL, stroke.WAH_TOE);    //To do -> change 0, 1023 by DATA_HELL & TOE from EXP stroke

    #ifdef DEBUG_PRESET_FLASH
          NRF_LOG_INFO("***************************************");        
          NRF_LOG_INFO("STROKE");
          NRF_LOG_INFO("STATUS                 = %d", stroke.STATUS);
          NRF_LOG_INFO("SOURCE                 = %d", stroke.SOURCE);
          NRF_LOG_INFO("EXP_CURVE_RESPONSE     = %d", stroke.EXP_CURVE_RESPONSE);
          NRF_LOG_INFO("WAH_CURVE_RESPONSE     = %d", stroke.WAH_CURVE_RESPONSE);
          NRF_LOG_INFO("EXP_HEEL               = %d", stroke.EXP_HEEL);
          NRF_LOG_INFO("EXP_TOE                = %d", stroke.EXP_TOE);
          NRF_LOG_INFO("WAH_HEEL               = %d", stroke.WAH_HEEL);
          NRF_LOG_INFO("WAH_TOE                = %d", stroke.WAH_TOE);
    #endif
}

/*******************************************************************************

*******************************************************************************/
preset_config_32_t * m_fds_read_preset(uint16_t FILE_ID, uint16_t RECORD_KEY)
{	
  preset_config_32_t * data;
  uint32_t  err_code;

  fds_flash_record_t  flash_record;
  fds_find_token_t    ftok;
  /* It is required to zero the token before first use. */
  memset(&ftok, 0x00, sizeof(fds_find_token_t));
  
  err_code = fds_record_find(FILE_ID, RECORD_KEY, &my_record_desc, &ftok);
  APP_ERROR_CHECK(err_code);

  err_code = fds_record_open(&my_record_desc, &flash_record);
  APP_ERROR_CHECK(err_code);

  //NRF_LOG_INFO("Found Record ID = %d \r\n",my_record_desc.record_id);			
  data = (preset_config_32_t *) flash_record.p_data;	
   
  /* Access the record through the flash_record structure. */
  /* Close the record when done. */
  err_code = fds_record_close(&my_record_desc);
  APP_ERROR_CHECK(err_code);
      
  return data;
}

/*******************************************************************************

*******************************************************************************/
calib_config_32_t * m_fds_read_calibration(uint16_t FILE_ID, uint16_t RECORD_KEY)
{	
  calib_config_32_t * data;
  uint32_t  err_code;

  fds_flash_record_t  flash_record;
  fds_find_token_t    ftok;
  /* It is required to zero the token before first use. */
  memset(&ftok, 0x00, sizeof(fds_find_token_t));
  
  err_code = fds_record_find(FILE_ID, RECORD_KEY, &my_record_desc, &ftok);
  APP_ERROR_CHECK(err_code);

  err_code = fds_record_open(&my_record_desc, &flash_record);
  APP_ERROR_CHECK(err_code);

  //NRF_LOG_INFO("Found Record ID = %d \r\n",my_record_desc.record_id);			
  data = (calib_config_32_t *) flash_record.p_data;	
   
  /* Access the record through the flash_record structure. */
  /* Close the record when done. */
  err_code = fds_record_close(&my_record_desc);
  APP_ERROR_CHECK(err_code);
      
  return data;
}

/*******************************************************************************

*******************************************************************************/
stroke_config_32_t * m_fds_read_stroke(uint16_t FILE_ID, uint16_t RECORD_KEY)
{	
  stroke_config_32_t * data;
  uint32_t  err_code;

  fds_flash_record_t  flash_record;
  fds_find_token_t    ftok;
  /* It is required to zero the token before first use. */
  memset(&ftok, 0x00, sizeof(fds_find_token_t));
  
  err_code = fds_record_find(FILE_ID, RECORD_KEY, &my_record_desc, &ftok);
  APP_ERROR_CHECK(err_code);

  err_code = fds_record_open(&my_record_desc, &flash_record);
  APP_ERROR_CHECK(err_code);

  //NRF_LOG_INFO("Found Record ID = %d \r\n",my_record_desc.record_id);			
  data = (stroke_config_32_t *) flash_record.p_data;	
   
  /* Access the record through the flash_record structure. */
  /* Close the record when done. */
  err_code = fds_record_close(&my_record_desc);
  APP_ERROR_CHECK(err_code);
      
  return data;
}

/*******************************************************************************

*******************************************************************************/
void save_preset2flash(uint8_t idx_prst)
{	
    preset_32[idx_prst].FC1             = preset[idx_prst].FC1;
    preset_32[idx_prst].FC2             = preset[idx_prst].FC2;
    preset_32[idx_prst].Q1              = preset[idx_prst].Q1;
    preset_32[idx_prst].Q2              = preset[idx_prst].Q2;
    preset_32[idx_prst].LV1             = preset[idx_prst].LV1;
    preset_32[idx_prst].LV2             = preset[idx_prst].LV2;
    preset_32[idx_prst].STATUS          = preset[idx_prst].STATUS;
    preset_32[idx_prst].MODE            = preset[idx_prst].MODE;
    preset_32[idx_prst].TIME_AUTO_WAH   = preset[idx_prst].TIME_AUTO_WAH;
    preset_32[idx_prst].TIME_AUTO_LEVEL = preset[idx_prst].TIME_AUTO_LEVEL;
    preset_32[idx_prst].IMPEDANCE       = preset[idx_prst].IMPEDANCE;
    preset_32[idx_prst].COLOR           = preset[idx_prst].COLOR;
    preset_32[idx_prst].HIGH_VOYEL      = preset[idx_prst].HIGH_VOYEL;
    preset_32[idx_prst].LOW_VOYEL       = preset[idx_prst].LOW_VOYEL;
    preset_32[idx_prst].MIX_DRY_WET1    = preset[idx_prst].MIX_DRY_WET1;
    preset_32[idx_prst].MIX_DRY_WET2    = preset[idx_prst].MIX_DRY_WET2;
    preset_32[idx_prst].FILTER_TYPE     = preset[idx_prst].FILTER_TYPE;
    preset_32[idx_prst].BYPASS_SOURCE   = preset[idx_prst].BYPASS_SOURCE;
    preset_32[idx_prst].GAIN            = preset[idx_prst].GAIN;
    strcpy(preset_32[idx_prst].NAME,      "");
    strcpy(preset_32[idx_prst].NAME,      preset[idx_prst].NAME);

    #ifdef DEBUG_PRESET
      NRF_LOG_INFO("************INTO FLASH*****************");        
      NRF_LOG_INFO("PRESET_              %d", idx_prst);
      NRF_LOG_INFO("FC1 =                %d", preset_32[idx_prst].FC1);
      NRF_LOG_INFO("FC2 =                %d", preset_32[idx_prst].FC2);
      NRF_LOG_INFO("Q1 =                 %d", preset_32[idx_prst].Q1);
      NRF_LOG_INFO("Q2 =                 %d", preset_32[idx_prst].Q2); 
      NRF_LOG_INFO("LV1 =                %d", preset_32[idx_prst].LV1);
      NRF_LOG_INFO("LV2 =                %d", preset_32[idx_prst].LV2);
      NRF_LOG_INFO("STATUS =             %d", preset_32[idx_prst].STATUS);
      NRF_LOG_INFO("MODE =               %d", preset_32[idx_prst].MODE); 
      NRF_LOG_INFO("TIME_AUTO_WAH =      %d", preset_32[idx_prst].TIME_AUTO_WAH); 
      NRF_LOG_INFO("TIME_AUTO_LEVEL =    %d", preset_32[idx_prst].TIME_AUTO_LEVEL);
      NRF_LOG_INFO("IMPEDANCE =          %d", preset_32[idx_prst].IMPEDANCE);
      NRF_LOG_INFO("COLOR =              %d", preset_32[idx_prst].COLOR);
      NRF_LOG_INFO("HIGH_VOYEL =         %d", preset_32[idx_prst].HIGH_VOYEL); 
      NRF_LOG_INFO("LOW_VOYEL =          %d", preset_32[idx_prst].LOW_VOYEL);
      NRF_LOG_INFO("MIX_DRY_WET1 =       %d", preset_32[idx_prst].MIX_DRY_WET1); 
      NRF_LOG_INFO("MIX_DRY_WET2 =       %d", preset_32[idx_prst].MIX_DRY_WET2); 
      NRF_LOG_INFO("FILTER_TYPE =        %d", preset_32[idx_prst].FILTER_TYPE); 
      NRF_LOG_INFO("BYPASS_SOURCE =      %d", preset_32[idx_prst].BYPASS_SOURCE); 
      NRF_LOG_INFO("GAIN =               %d", preset_32[idx_prst].GAIN); 
      NRF_LOG_INFO("NAME =               %s", preset_32[idx_prst].NAME); 
    #endif

    //flash_writing = true;
    write_preset_config(idx_prst);
    //while(!flash_writing);
}

/*******************************************************************************

*******************************************************************************/
long map(long data, long in_min, long in_max, long out_min, long out_max)
{
    return (data - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

/*******************************************************************************

*******************************************************************************/
//float mapfloat(float x, float in_min, float in_max, float out_min, float out_max)
//{
// return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
//}

float mapfloat(float x, float in_min, float in_max, float out_min, float out_max)
{
 return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

/*******************************************************************************

*******************************************************************************/
float FmultiMap(float val, float * _in, float * _out, uint8_t size)
{
  // take care the value is within range
  // val = constrain(val, _in[0], _in[size-1]);
  if (val <= _in[0]) return _out[0];
  if (val >= _in[size-1]) return _out[size-1];

  // search right interval
  uint8_t pos = 1;  // _in[0] allready tested
  while(val > _in[pos]) pos++;

  // this will handle all exact "points" in the _in array
  if (val == _in[pos]) return _out[pos];

  // interpolate in the right segment for the rest
  return (val - _in[pos-1]) * (_out[pos] - _out[pos-1]) / (_in[pos] - _in[pos-1]) + _out[pos-1];
}
