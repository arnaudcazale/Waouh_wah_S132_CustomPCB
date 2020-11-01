#ifndef UTILS_H__
#define UTILS_H__

#include <stdint.h>
#include <stdbool.h>
#include "nrf_drv_gpiote.h"
#include "pca10040.h"
#include "fds.h"
#include "nrf_log.h"
#include "nrf_log_internal.h"
#include "nrf_log_ctrl.h"
#include "nrf_delay.h"
#include "ble_wah.h"


#define RECORD_KEY_PRST        0x2222
#define FILE_ID_PRST_BASE      0x1110
//Flash info for PRESET_1
#define FILE_ID_PRST_1         0x1110
//Flash info for PRESET_2
#define FILE_ID_PRST_2         0x1111
//Flash info for PRESET_3
#define FILE_ID_PRST_3         0x1112
//Flash info for PRESET_4
#define FILE_ID_PRST_4         0x1113
//Flash info for CALIBRATION structure
#define RECORD_KEY_CALIB       0x2223
#define FILE_ID_CALIB          0x1114

#define PRESET_NUMBER          4

//#define DEBUG_PRESET_FLASH

#define INDEX_FC1              0
#define INDEX_FC2              1
#define INDEX_Q1               2 
#define INDEX_Q2               3 
#define INDEX_LV1              4 
#define INDEX_LV2              5 
#define INDEX_STATUS           6 
#define INDEX_MODE             7 
#define INDEX_TIME_AUTO_WAH    8 
#define INDEX_TIME_AUTO_LEVEL  10
#define INDEX_IMPEDANCE        12
#define INDEX_COLOR            13
#define INDEX_HIGH_VOYEL       14
#define INDEX_LOW_VOYEL        15
#define INDEX_MIX_DRY_WET1     16
#define INDEX_MIX_DRY_WET2     17
#define INDEX_FILTER_TYPE      18
#define INDEX_SOURCE           19
#define INDEX_NAME             20

#define NAME_MAX_LENGTH        20

enum
{
    PRESET_EDIT_STATUS,           
    PRESET_SAVE_STATUS,     
};

enum
{
    MANUAL_WAH_MODE,           
    MANUAL_LEVEL_MODE,  
    AUTO_WAH_MODE,           
    AUTO_LEVEL_MODE,  
    TALKBOX,
    TEST,
};

enum
{         
    LOW_Z, 
    HIGH_Z,  
};

enum
{
    COLOR_1,           
    COLOR_2,  
    COLOR_3,
    COLOR_4,
};

enum
{       
    BAND_PASS, 
    LOW_PASS,
    HIGH_PASS,
    NOTCH,
};

enum
{
    ae,           
    ah,  
    aw,
    e,
    ee,
    er,
    l,
    oo,
    u,
    uh
};

enum
{
    NONE,
    GO_DOWN,
    GO_UP,
    DONE
};

enum
{
    EXP = 2,
    WAH = 3,
};

#ifdef __GNUC__
    #ifdef PACKED
        #undef PACKED
    #endif

    #define PACKED(TYPE) TYPE __attribute__ ((packed))
#endif

typedef PACKED( struct
{
    __ALIGN(4) uint8_t                  FC1;
    __ALIGN(4) uint8_t                  FC2;
    __ALIGN(4) uint8_t                  Q1;
    __ALIGN(4) uint8_t                  Q2;
    __ALIGN(4) uint8_t                  LV1;
    __ALIGN(4) uint8_t                  LV2;
    __ALIGN(4) uint8_t                  STATUS;
    __ALIGN(4) uint8_t                  MODE;
    __ALIGN(4) uint16_t                 TIME_AUTO_WAH;
    __ALIGN(4) uint16_t                 TIME_AUTO_LEVEL;
    __ALIGN(4) uint8_t                  IMPEDANCE;
    __ALIGN(4) uint8_t                  COLOR;
    __ALIGN(4) uint8_t                  HIGH_VOYEL;
    __ALIGN(4) uint8_t                  LOW_VOYEL;
    __ALIGN(4) uint8_t                  MIX_DRY_WET1;
    __ALIGN(4) uint8_t                  MIX_DRY_WET2;
    __ALIGN(4) uint8_t                  FILTER_TYPE;
    __ALIGN(4) uint8_t                  SOURCE;
    __ALIGN(4) char                     NAME[NAME_MAX_LENGTH];
   
}) preset_config_32_t;

typedef PACKED( struct
{
     uint8_t                  FC1;
     uint8_t                  FC2;
     uint8_t                  Q1;
     uint8_t                  Q2;
     uint8_t                  LV1;
     uint8_t                  LV2;
     uint8_t                  STATUS;
     uint8_t                  MODE;
     uint16_t                 TIME_AUTO_WAH;
     uint16_t                 TIME_AUTO_LEVEL;
     uint8_t                  IMPEDANCE;
     uint8_t                  COLOR;
     uint8_t                  HIGH_VOYEL;
     uint8_t                  LOW_VOYEL;
     uint8_t                  MIX_DRY_WET1;
     uint8_t                  MIX_DRY_WET2;
     uint8_t                  FILTER_TYPE;
     uint8_t                  SOURCE;
     char                     NAME[NAME_MAX_LENGTH];

}) preset_config_8_t;

typedef PACKED( struct
{
    __ALIGN(4) uint8_t    STATUS;
    __ALIGN(4) uint16_t   DATA_HEEL;
    __ALIGN(4) uint16_t   DATA_TOE;
    __ALIGN(4) uint8_t    GAIN;

}) calib_config_32_t;

typedef PACKED( struct
{
    uint8_t    STATUS;
    uint16_t   DATA_HEEL;
    uint16_t   DATA_TOE;
    uint8_t    GAIN;      

}) calib_config_8_t;


void update_led(uint8_t led);

void load_presets_from_flash(bool);
static ret_code_t fds_test_init (void);
static void my_fds_evt_handler(fds_evt_t const * );
static ret_code_t check_memory(void);
static ret_code_t write_factory_presets(void);
static void write_preset_config(uint8_t);
static void write_calibration_config(void);
void write_calibration_done(uint8_t, uint16_t, uint16_t, uint8_t);
static ret_code_t m_fds_find_and_delete(uint16_t, uint16_t);
static ret_code_t m_fds_write_preset(uint16_t, uint16_t, preset_config_32_t*);
static ret_code_t m_fds_write_calibration(uint16_t, uint16_t, calib_config_32_t*);
static void load_flash_config(void);
preset_config_32_t * m_fds_read_preset(uint16_t, uint16_t);
calib_config_32_t *  m_fds_read_calibration(uint16_t, uint16_t);
void convert_to_byte_format(void);
void save_preset2flash(uint8_t);
long map(long, long, long, long, long);




#endif // UTILS_H__