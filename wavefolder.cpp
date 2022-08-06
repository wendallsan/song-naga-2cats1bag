#include "Wavefolder.h"
#include <algorithm>

namespace daisysp
{
    void Wavefolder::SetGain( float gain ){
        gain_ = gain;
    }
    void Wavefolder::Init(){
        gain_ = 0.0;
    }
    float Wavefolder::Process( float in ){
        float ft, sgn;
        in  *= gain_;
        ft  = floorf((in + 1.0f) * 0.5f);
        sgn = static_cast<int>(ft) % 2 == 0 ? 1.0f : -1.0f;
        return sgn * (in - 2.0f * ft);
    }
}