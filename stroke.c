#include "stroke.h"
#include "utils.h"

float out[255];
float out4log[255];
float out_calib[255];
float linear_in[255];
float log_in[255];
float exp_in[255];

float step_out = 1024/255;

static int _min_calib;
static int _max_calib;

/*******************************************************************************

*******************************************************************************/
void stroke_response_fill_vectors(curve_response_t exp_curve_response, uint16_t min_calib, uint16_t max_calib)
{
    _min_calib = min_calib;
    _max_calib = max_calib;

    fill_x_vector();

    switch( exp_curve_response )
    {
      case RAW:
       
        break;

      case LOG:
          fill_log_vector();
        break;

      case EXPO:
          fill_expo_vector();
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
void fill_log_vector()
{
    //Fill log vector
    for(int i=0; i<255; i++)
    {
        if( i == 0)
        {
            log_in[i] = 0;
        }else
        {
            log_in[i] = logf(out4log[i]);
        }
    }

    //Mapping from calib
    for(int i=0; i<255; i++)
    {
        log_in[i] = mapfloat(log_in[i], 0, log_in[254], _min_calib, _max_calib);
    }
    
}

/*******************************************************************************

*******************************************************************************/
void fill_expo_vector()
{
    //Fill expo vector
    for(int i=0; i<255; i++)
    {
        exp_in[i] = (exp(out[i]/100));
    }

    //Mapping from calib
    for(int i=0; i<255; i++)
    {
        exp_in[i] = mapfloat(exp_in[i], 0, exp_in[254], _min_calib, _max_calib);
    }

    //NRF_LOG_HEXDUMP_INFO(exp_in, 255);
}
