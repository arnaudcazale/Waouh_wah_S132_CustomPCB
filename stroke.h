#ifndef STROKE_H__
#define STROKE_H__

#include <stdint.h>
#include <stdbool.h>
#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"
#include <math.h>

typedef enum 
{
    RAW,
    LOG,
    EXPO
} curve_response_t;

//#ifdef __GNUC__
//    #ifdef PACKED
//        #undef PACKED
//    #endif
//
//    #define PACKED(TYPE) TYPE __attribute__ ((packed))
//#endif

//typedef PACKED( struct
//{
//    float              VECTOR_EXP[255];
//    float              VECTOR_WAH[255];
//
//}) stroke_response_config_t;

void stroke_response_fill_vectors(curve_response_t, uint16_t, uint16_t);
static void fill_x_vector();
static void fill_log_vector();
static void fill_expo_vector();

#endif // STROKE_H__