#pragma once
#ifndef DSY_WAVEFOLDER_H
#define DSY_WAVEFOLDER_H

#include <stdint.h>
#include "Utility/dsp.h"
#ifdef __cplusplus

/** @file Wavefolder.h */

namespace daisysp
{
/**  
       @brief Wavefolder
       @date August 2022
*/
class Wavefolder {
    public:
        Wavefolder(){}
        ~Wavefolder(){}
        void SetGain( float amount );
        float Process( float in );
        void Init();

    private:
        float gain_;
}; // namespace daisysp
};
#endif
#endif