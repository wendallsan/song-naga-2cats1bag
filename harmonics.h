#pragma once
#ifndef DSY_HARMONICS_H
#define DSY_HARMONICS_H

#include <stdint.h>
#include "Utility/dsp.h"
#ifdef __cplusplus

/** @file harmonics.h */

namespace daisysp
{
/**  
       @brief Harmonics
       @date August 2022
*/
class Harmonics {
    public:
        Harmonics(){}
        ~Harmonics(){}
        void SetBalance( float amount );
        void Init( float sampleRate );
        float Process( float in );
        float TanhShape( float signal, float a );
        float TanhOffset( float signal, float d, float a );
        float ModHT( float signal, float d, float a );
        float Overdrive( float signal, float shape );
        float AtanDrive( float signal, float drive );
        float PolyWs( float signal, float d );
        float PolyVws( float signal, float c, float g, float d );
        float ScDistort( float signal, float drive );
        float ScSoftClip( float signal, float drive );

    private:
        float balance_;
        DcBlock dcblock_;
}; // namespace daisysp
};
#endif
#endif