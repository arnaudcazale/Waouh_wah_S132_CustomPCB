#include "stroke.h"
#include "utils.h"

float out[255];
float out4log_exp[255];
float out_calib_exp[255];
float out4log_wah[255];
float out_calib_wah[255];
//float linear_in[255];
float loga_exp[255];
float expo_exp[255];
float loga_wah[255];
float expo_wah[255];

float step_out = 1024/255;

static int _min_calib;
static int _max_calib;
static int _curve;
static uint8_t _source;

extern volatile calib_config_8_t       calibration;
extern volatile stroke_config_t        stroke;

/*******************************************************************************

*******************************************************************************/
float* stroke_response_fill_vectors(uint8_t source, curve_t curve_response, uint16_t min_calib, uint16_t max_calib)
{
    _source = source;
    _min_calib = min_calib;
    _max_calib = max_calib;
    _curve = curve_response;

   

    if(_source == EXP)
    {
        fill_x_vector_exp();
        switch( _curve )
        {
          case RAW:
       
            break;

          case LOG:
              return fill_log_vector_exp();
            break;

          case EXPO:
              return fill_expo_vector_exp();
            break;

          default:
            break;
        }
    }

    if(_source == WAH)
    {
        fill_x_vector_wah();
        switch( _curve )
        {
          case RAW:
       
            break;

          case LOG:
              return fill_log_vector_wah();
            break;

          case EXPO:
              return fill_expo_vector_wah();
            break;

          default:
            break;
        }


    }

    
        
}

/*******************************************************************************

*******************************************************************************/
void fill_x_vector_exp()
{
    //Fill x vector
    for(int i=0; i<255; i++)
    {
        out[i] = (float)i * step_out;
    }

    //Map x vector to calibration limits
    for(int i=0; i<255; i++)
    {
        out_calib_exp[i] = map(out[i], 0, 1023, _min_calib, _max_calib) ;
        out4log_exp[i] = map(out[i], 0, 1023, _min_calib, _max_calib) - _min_calib; //Need to start from 0 to convert to log function
    }
}

/*******************************************************************************

*******************************************************************************/
float* fill_log_vector_exp()
{
    //Fill log vector
    for(int i=0; i<255; i++)
    {
        if( i == 0)
        {
            loga_exp[i] = 0;
        }else
        {
            loga_exp[i] = logf(out4log_exp[i]);
        }
    }

    //Mapping from calib
    for(int i=0; i<255; i++)
    {
        loga_exp[i] = mapfloat(loga_exp[i], 0, loga_exp[254], _min_calib, _max_calib);
    }

    return loga_exp;
}

/*******************************************************************************

*******************************************************************************/
float* fill_expo_vector_exp()
{
    //Fill expo vector
    for(int i=0; i<255; i++)
    {
        expo_exp[i] = (exp(out[i]/100));
    }

    //Mapping from calib
    for(int i=0; i<255; i++)
    {
        expo_exp[i] = mapfloat(expo_exp[i], 0, expo_exp[254], _min_calib, _max_calib);
    }

    return expo_exp;
    //NRF_LOG_HEXDUMP_INFO(exp_in, 255);
}

/*******************************************************************************

*******************************************************************************/
void fill_x_vector_wah()
{
    //Fill x vector
    for(int i=0; i<255; i++)
    {
        out[i] = (float)i * step_out;
    }

    //Map x vector to calibration limits
    for(int i=0; i<255; i++)
    {
        out_calib_wah[i] = map(out[i], 0, 1023, _min_calib, _max_calib) ;
        out4log_wah[i] = map(out[i], 0, 1023, _min_calib, _max_calib) - _min_calib; //Need to start from 0 to convert to log function
    }
}

/*******************************************************************************

*******************************************************************************/
float* fill_log_vector_wah()
{
    //Fill log vector
    for(int i=0; i<255; i++)
    {
        if( i == 0)
        {
            loga_wah[i] = 0;
        }else
        {
            loga_wah[i] = logf(out4log_wah[i]);
        }
    }

    //Mapping from calib
    for(int i=0; i<255; i++)
    {
        loga_wah[i] = mapfloat(loga_wah[i], 0, loga_wah[254], _min_calib, _max_calib);
    }

    return loga_wah;
}

/*******************************************************************************

*******************************************************************************/
float* fill_expo_vector_wah()
{
    //Fill expo vector
    for(int i=0; i<255; i++)
    {
        expo_wah[i] = (exp(out[i]/100));
    }

    //Mapping from calib
    for(int i=0; i<255; i++)
    {
        expo_wah[i] = mapfloat(expo_wah[i], 0, expo_wah[254], _min_calib, _max_calib);
    }

    return expo_wah;
    //NRF_LOG_HEXDUMP_INFO(exp_in, 255);
}


/*******************************************************************************

*******************************************************************************/
uint16_t map_calib(uint16_t data, uint8_t source)
{
//    NRF_LOG_INFO("map_calib source %d", source);
//    NRF_LOG_INFO("map_calib calibration.EXP_CURVE_RESPONSE %d", stroke.WAH_CURVE_RESPONSE);

    if(source == EXP)
    {
        switch(stroke.EXP_CURVE_RESPONSE)
        {
          case LOG:
              data = (uint16_t)FmultiMap(data, out_calib_exp, loga_exp , 255);
            break;

          case EXPO:
              data = (uint16_t)FmultiMap(data, out_calib_exp, expo_exp , 255);
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
              data = (uint16_t)FmultiMap(data, out_calib_wah, loga_wah , 255);
            break;

          case EXPO:
              data = (uint16_t)FmultiMap(data, out_calib_wah, expo_wah , 255);
            break;

          default:
            break;
        }
    }

    return data;
}

