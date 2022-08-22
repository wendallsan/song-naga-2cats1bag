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
        float Process( float in );
        void Init();
        float OneDecimal( float f );

    private:
        float balance_;
}; // namespace daisysp
};
#endif
#endif