#include "harmonics.h"
#include <algorithm>

namespace daisysp
{
    void Harmonics::SetBalance( float balance ){
        balance_ = balance;
    }
    void Harmonics::Init(){
        balance_ = 0.5;
    }

    float oneDecimal( float f ){
        float value = ( int )( f * 10.0 + 0.5 );
        return ( float )value / 10.0;
    }

    float Harmonics::Process( float in ){
        
/*

The Fourier Series decomposes a periodic function with period T into sines and cosines with 
frequencies nT,n=0,1,2,… which are called the nth harmonics of the signal. The more harmonics 
are used, the more accurate can a function be described.
For even functions, i.e. x(t)=x(−t), the Fourier series only consists of cosines. 
For odd functions, i.e. x(t)=−x(−t), the Fourier series only consists of sines.
At jump discontinuities, the value of the Fourier series is in the middle of the jump. Around the jump, overshooting of the series occurs, which is called Gibbs Phenonemon. The amount of overshooting does not reduce with the number of harmonics, but the duration reduces.
Smooth functions need less harmonics to be accurately described by the Fourier series.

*/
        // REDUCE RESOLUTION OF BALANCE TO ONE DECIMAL POINT
        float finalBalance = oneDecimal( balance_ );
        float finalSignal;
        if( finalBalance == 0.5 ) finalSignal = in; // IF BALANCE IS CENTERED, DON'T MODIFY ANYTHING
        if( finalBalance < 0.5 ) {
            float alteredSignal = in;
            // HANDLE ODD HARMONICS
            // LIFTED THE 'SOFT CLIPPING' FROM THE DISTORTION EXAMPLE
            if( in > 0 ) alteredSignal = 1 - expf( -in );
            alteredSignal = -1 + expf( alteredSignal );
            // FINAL SIGNAL IS MIX OF IN AND ALTERED SIGNAL            
            float mixRange = finalBalance * 2.0; // from 0 to 1
            finalSignal = ( in * mixRange/1.0 ) + ( alteredSignal * 1.0/mixRange );
        }
        if( finalBalance > 0.5 ){
            float alteredSignal;
            // HANDLE EVEN HARMONICS
            // LIFTED THE HARD CLIPPING FROM THE DISTORTION EXAMPLE: 
            alteredSignal = in > 1.f ? 1.f : in;
            alteredSignal = alteredSignal < -1.f ? -1.f : alteredSignal;
            float mixRange = ( finalBalance * 2.0 ) - 1.0; // from 0 to 1
            finalSignal = ( in * mixRange/1.0 ) + ( alteredSignal * 1.0/mixRange );
        }
        return finalSignal;
    }
}