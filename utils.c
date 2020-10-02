#include "utils.h"



static bool flash_writing = false;
static bool flash_initializing = false;
static fds_record_desc_t    my_record_desc;

static preset_config_32_t preset_32[PRESET_NUMBER];
preset_config_8_t preset[PRESET_NUMBER];

void clear_all_leds()
{
    nrf_gpio_pin_set(LED_1); 
    nrf_gpio_pin_set(LED_2); 
    nrf_gpio_pin_set(LED_3); 
    nrf_gpio_pin_set(LED_4); 
}

/*******************************************************************************

*******************************************************************************/
void update_led(uint8_t led) 
{
      clear_all_leds();

      switch(led)
      {
        case 0:
            nrf_gpio_pin_clear(LED_1); 
          break;
        case 1:
            nrf_gpio_pin_clear(LED_2); 
          break;
        case 2:
            nrf_gpio_pin_clear(LED_3); 
          break;
        case 3:
            nrf_gpio_pin_clear(LED_4); 
          break;
      }
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
    preset_32[0].FC2             = 1;
    preset_32[0].Q1              = 2;
    preset_32[0].Q2              = 3;
    preset_32[0].LV1             = 4;
    preset_32[0].LV2             = 5;
    preset_32[0].STATUS          = PRESET_EDIT_STATUS;
    preset_32[0].MODE            = MANUAL_WAH_MODE;
    preset_32[0].TIME_AUTO_WAH   = 127;
    preset_32[0].TIME_AUTO_LEVEL = 126;
    preset_32[0].IMPEDANCE       = LOW_Z;
    preset_32[0].COLOR           = COLOR_1;
    strcpy(preset_32[0].NAME, "PRESET_1");

    //flash_writing = true;
    write_preset_config(0);
    //while(!flash_writing);

    preset_32[1].FC1             = 10;
    preset_32[1].FC2             = 11;
    preset_32[1].Q1              = 12;
    preset_32[1].Q2              = 13;
    preset_32[1].LV1             = 14;
    preset_32[1].LV2             = 15;
    preset_32[1].STATUS          = PRESET_EDIT_STATUS;
    preset_32[1].MODE            = MANUAL_WAH_MODE;
    preset_32[1].TIME_AUTO_WAH   = 666;
    preset_32[1].TIME_AUTO_LEVEL = 222;
    preset_32[1].IMPEDANCE       = LOW_Z;
    preset_32[1].COLOR           = COLOR_2;
    strcpy(preset_32[1].NAME, "PRESET_2");

    //flash_writing = true;
    write_preset_config(1);
    //while(flash_writing);

    preset_32[2].FC1             = 20;
    preset_32[2].FC2             = 21;
    preset_32[2].Q1              = 22;
    preset_32[2].Q2              = 23;
    preset_32[2].LV1             = 24;
    preset_32[2].LV2             = 25;
    preset_32[2].STATUS          = PRESET_EDIT_STATUS;
    preset_32[2].MODE            = MANUAL_WAH_MODE;
    preset_32[2].TIME_AUTO_WAH   = 1000;
    preset_32[2].TIME_AUTO_LEVEL = 500;
    preset_32[2].IMPEDANCE       = LOW_Z;
    preset_32[2].COLOR           = COLOR_3;
    strcpy(preset_32[2].NAME, "PRESET_3");

    //flash_writing = true;
    write_preset_config(2);
    //while(flash_writing);

    preset_32[3].FC1             = 30;
    preset_32[3].FC2             = 31;
    preset_32[3].Q1              = 32;
    preset_32[3].Q2              = 33;
    preset_32[3].LV1             = 34;
    preset_32[3].LV2             = 35;
    preset_32[3].STATUS          = PRESET_EDIT_STATUS;
    preset_32[3].MODE            = MANUAL_WAH_MODE;
    preset_32[3].TIME_AUTO_WAH   = 999;
    preset_32[3].TIME_AUTO_LEVEL = 444;
    preset_32[3].IMPEDANCE       = LOW_Z;
    preset_32[3].COLOR           = COLOR_4;
    strcpy(preset_32[3].NAME, "PRESET_4");

    //flash_writing = true;
    write_preset_config(3);
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
        strcpy(preset[idx_prst].NAME,      preset_32[idx_prst].NAME);

        #ifdef DEBUG_PRESET
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
          NRF_LOG_INFO("NAME =               %s", preset[idx_prst].NAME); 
        #endif

    }

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
    strcpy(preset_32[idx_prst].NAME,      "");
    strcpy(preset_32[idx_prst].NAME,      preset[idx_prst].NAME);

    #ifdef DEBUG_PRESET
      NRF_LOG_INFO("***************************************");        
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
      NRF_LOG_INFO("NAME =               %s", preset_32[idx_prst].NAME); 
    #endif

    //flash_writing = true;
    write_preset_config(idx_prst);
    //while(!flash_writing);
}

