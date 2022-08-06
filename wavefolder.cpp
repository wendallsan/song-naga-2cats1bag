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
        in  *= gain_; // MULTIPLY IN BY GAIN
        ft  = floorf( ( in + 1.0f ) * 0.5f );
        //    ^ ROUNDS DOWN TO NEAREST WHOLE NUMBER VALUE
                        
        sgn = static_cast<int>(ft) % 2 == 0 ? 1.0f : -1.0f;
        //    ^ CONVERTS ft TO AN INT
        //                         ^ IS ft MODULUS 2 IS ZERO?
        //                                    ^ EITHER +/- 1.0
        return sgn * (in - 2.0f * ft);
        //     ^ IS EITHER +/- 1.0
        //            ^ in -2.0f = [-1.0...0.0]

    }
}