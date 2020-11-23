#include "stroke.h"
#include "utils.h"

float out[255];
float out4log[255];
float out_calib[255];
float linear_in[255];
float loga[255];
float expo[255];

float step_out = 1024/255;

static int _min_calib;
static int _max_calib;
static int _curve;

extern volatile calib_config_8_t       calibration;
extern volatile stroke_config_t        stroke;

/*******************************************************************************

*******************************************************************************/
float* stroke_response_fill_vectors(curve_t curve_response, uint16_t min_calib, uint16_t max_calib)
{
    _min_calib = min_calib;
    _max_calib = max_calib;
    _curve = curve_response;

    fill_x_vector();

    switch( _curve )
    {
      case RAW:
       
        break;

      case LOG:
          return fill_log_vector();
        break;

      case EXPO:
          return fill_expo_vector();
        break;

      default:
        break;
    }
        
}

/*******************************************************************************

*******************************************************************************/
void fill_x_vector()
{
    //Fill x vector
    for(int i=0; i<255; i++)
    {
        out[i] = (float)i * step_out;
    }

    //Map x vector to calibration limits
    for(int i=0; i<255; i++)
    {
        out_calib[i] = map(out[i], 0, 1023, _min_calib, _max_calib) ;
        out4log[i] = map(out[i], 0, 1023, _min_calib, _max_calib) - _min_calib; //Need to start from 0 to convert to log function
    }
}

/*******************************************************************************

*******************************************************************************/
float* fill_log_vector()
{
    //Fill log vector
    for(int i=0; i<255; i++)
    {
        if( i == 0)
        {
            loga[i] = 0;
        }else
        {
            loga[i] = logf(out4log[i]);
        }
    }

    //Mapping from calib
    for(int i=0; i<255; i++)
    {
        loga[i] = mapfloat(loga[i], 0, loga[254], _min_calib, _max_calib);
    }

    return loga;
}

/*******************************************************************************

*******************************************************************************/
float* fill_expo_vector()
{
    //Fill expo vector
    for(int i=0; i<255; i++)
    {
        expo[i] = (exp(out[i]/100));
    }

    //Mapping from calib
    for(int i=0; i<255; i++)
    {
        expo[i] = mapfloat(expo[i], 0, expo[254], _min_calib, _max_calib);
    }

    return expo;
    //NRF_LOG_HEXDUMP_INFO(exp_in, 255);
}


/*******************************************************************************

*******************************************************************************/
uint16_t map_calib(uint16_t data, uint8_t source)
{
//    NRF_LOG_INFO("map_calib source %d", source);
//    NRF_LOG_INFO("map_calib calibration.EXP_CURVE_RESPONSE %d", calibration.EXP_CURVE_RESPONSE);

    if(source == EXP)
    {
        switch(stroke.EXP_CURVE_RESPONSE)
        {
          case LOG:
              data = (uint16_t)FmultiMap(data, out_calib, loga , 255);
            break;

          case EXPO:
              data = (uint16_t)FmultiMap(data, out_calib, expo , 255);
            break;

          default:
            break;
        }
    }

    if(source == WAH)
    {
        switch(stroke.WAH_CURVE_RESPONSE)
        {
          case LOG:
              data = (uint16_t)FmultiMap(data, out_calib, loga , 255);
            break;

          case EXPO:
              data = (uint16_t)FmultiMap(data, out_calib, expo , 255);
            break;

          default:
            break;
        }
    }

    return data;
}

