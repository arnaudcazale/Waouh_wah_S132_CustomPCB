#include "utils.h"



//static bool flash_writing = false;
//static bool flash_initializing = false;
static fds_record_desc_t    my_record_desc;

static preset_config_32_t preset_32[PRESET_NUMBER];
preset_config_8_t preset[PRESET_NUMBER];
calib_config_32_t calibration_32;
calib_config_8_t calibration;

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

    preset_32[0].FC1             = 255;
    preset_32[0].FC2             = 0;
    preset_32[0].Q1              = 128;
    preset_32[0].Q2              = 128;
    preset_32[0].LV1             = 0;
    preset_32[0].LV2             = 0;
    preset_32[0].STATUS          = PRESET_EDIT_STATUS;
    preset_32[0].MODE            = MANUAL_WAH_MODE;
    preset_32[0].TIME_AUTO_WAH   = 127;
    preset_32[0].TIME_AUTO_LEVEL = 126;
    preset_32[0].IMPEDANCE       = LOW_Z;
    preset_32[0].COLOR           = COLOR_1;
    preset_32[0].HIGH_VOYEL      = ae;
    preset_32[0].LOW_VOYEL       = uh;
    preset_32[0].MIX_DRY_WET     = 0;
    preset_32[0].FILTER_TYPE     = LOW_PASS;
    strcpy(preset_32[0].NAME, "PRESET_1");

    //flash_writing = true;
    write_preset_config(0);
    //while(!flash_writing);

    preset_32[1].FC1             = 50;
    preset_32[1].FC2             = 200;
    preset_32[1].Q1              = 50;
    preset_32[1].Q2              = 200;
    preset_32[1].LV1             = 10;
    preset_32[1].LV2             = 50;
    preset_32[1].STATUS          = PRESET_EDIT_STATUS;
    preset_32[1].MODE            = MANUAL_WAH_MODE;
    preset_32[1].TIME_AUTO_WAH   = 666;
    preset_32[1].TIME_AUTO_LEVEL = 222;
    preset_32[1].IMPEDANCE       = HIGH_Z;
    preset_32[1].COLOR           = COLOR_2;
    preset_32[1].HIGH_VOYEL      = ae;
    preset_32[1].LOW_VOYEL       = uh;
    preset_32[1].MIX_DRY_WET     = 0;
    preset_32[1].FILTER_TYPE     = HIGH_PASS;
    strcpy(preset_32[1].NAME, "PRESET_2");

    //flash_writing = true;
    write_preset_config(1);
    //while(flash_writing);

    preset_32[2].FC1             = 50;
    preset_32[2].FC2             = 150;
    preset_32[2].Q1              = 5;
    preset_32[2].Q2              = 150;
    preset_32[2].LV1             = 20;
    preset_32[2].LV2             = 40;
    preset_32[2].STATUS          = PRESET_EDIT_STATUS;
    preset_32[2].MODE            = MANUAL_WAH_MODE;
    preset_32[2].TIME_AUTO_WAH   = 1000;
    preset_32[2].TIME_AUTO_LEVEL = 500;
    preset_32[2].IMPEDANCE       = LOW_Z;
    preset_32[2].COLOR           = COLOR_3;
    preset_32[2].HIGH_VOYEL      = ae;
    preset_32[2].LOW_VOYEL       = uh;
    preset_32[2].MIX_DRY_WET     = 0;
    preset_32[2].FILTER_TYPE     = BAND_PASS;
    strcpy(preset_32[2].NAME, "PRESET_3");

    //flash_writing = true;
    write_preset_config(2);
    //while(flash_writing);

    preset_32[3].FC1             = 75;
    preset_32[3].FC2             = 125;
    preset_32[3].Q1              = 75;
    preset_32[3].Q2              = 125;
    preset_32[3].LV1             = 30;
    preset_32[3].LV2             = 40;
    preset_32[3].STATUS          = PRESET_EDIT_STATUS;
    preset_32[3].MODE            = MANUAL_WAH_MODE;
    preset_32[3].TIME_AUTO_WAH   = 999;
    preset_32[3].TIME_AUTO_LEVEL = 444;
    preset_32[3].IMPEDANCE       = HIGH_Z;
    preset_32[3].COLOR           = COLOR_4;
    preset_32[3].HIGH_VOYEL      = ae;
    preset_32[3].LOW_VOYEL       = uh;
    preset_32[3].MIX_DRY_WET     = 0;
    preset_32[3].FILTER_TYPE     = NOTCH;
    strcpy(preset_32[3].NAME, "PRESET_4");

    //flash_writing = true;
    write_preset_config(3);
    //while(flash_writing);

    calibration_32.STATUS   = NONE;
    calibration_32.MAX_DATA = 0;
    calibration_32.MIN_DATA = 0;

    //flash_writing = true;
    write_calibration_config();
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
        preset_32[idx_prst].MIX_DRY_WET     = data->MIX_DRY_WET;
        preset_32[idx_prst].FILTER_TYPE     = data->FILTER_TYPE;
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
        preset[idx_prst].MIX_DRY_WET     = preset_32[idx_prst].MIX_DRY_WET;
        preset[idx_prst].FILTER_TYPE     = preset_32[idx_prst].FILTER_TYPE;
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
          NRF_LOG_INFO("MIX_DRY_WET =        %d", preset[idx_prst].MIX_DRY_WET); 
          NRF_LOG_INFO("FILTER_TYPE =        %d", preset[idx_prst].FILTER_TYPE); 
          NRF_LOG_INFO("NAME =               %s", preset[idx_prst].NAME); 
        #endif

    }

    //Load Calibration Config
    calib_config_32_t * data_calib;

    data_calib = m_fds_read_calibration(FILE_ID_CALIB, RECORD_KEY_CALIB);

    calibration_32.STATUS           = data_calib->STATUS;
    calibration_32.MAX_DATA         = data_calib->MAX_DATA;
    calibration_32.MIN_DATA         = data_calib->MIN_DATA;

    calibration.STATUS              = calibration_32.STATUS;
    calibration.MAX_DATA            = calibration_32.MAX_DATA;
    calibration.MIN_DATA            = calibration_32.MIN_DATA;

    #ifdef DEBUG_PRESET
          NRF_LOG_INFO("***************************************");        
          NRF_LOG_INFO("CALIBRATION");
          NRF_LOG_INFO("STATUS =                %d", data_calib->STATUS);
          NRF_LOG_INFO("MAX_DATA =              %d", data_calib->MAX_DATA);
          NRF_LOG_INFO("MIN_DATA =              %d", data_calib->MIN_DATA);
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
    preset_32[idx_prst].MIX_DRY_WET     = preset[idx_prst].MIX_DRY_WET;
    preset_32[idx_prst].FILTER_TYPE     = preset[idx_prst].FILTER_TYPE;
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
      NRF_LOG_INFO("MIX_DRY_WET =        %d", preset_32[idx_prst].MIX_DRY_WET); 
      NRF_LOG_INFO("FILTER_TYPE =        %d", preset_32[idx_prst].FILTER_TYPE); 
      NRF_LOG_INFO("NAME =               %s", preset_32[idx_prst].NAME); 
    #endif

    //flash_writing = true;
    write_preset_config(idx_prst);
    //while(!flash_writing);
}

long map(long data, long in_min, long in_max, long out_min, long out_max)
{
    return (data - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

