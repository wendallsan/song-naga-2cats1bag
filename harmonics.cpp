#include "daisy_seed.h"
#include "daisysp.h"
#include "harmonics.h"
#include <algorithm>

namespace daisysp
{
    void Harmonics::SetBalance( float balance ){
        balance_ = balance; 
    }
    void Harmonics::Init( float sampleRate ){
        balance_ = 0.5;
        dcblock_.Init( sampleRate );
    }
    float Harmonics::TanhShape( float signal, float a ) {
        a = a > 1.0? a : 1.0;
        return tanh( signal * a );
    }
    float Harmonics::TanhOffset( float signal, float d, float a ){
        a = a > 1.0? a : 1.0;
        d = fclamp( d, 0.0, 0.2 );
        // return dcblock_.Process( tanh( signal + d ) * a );
        return tanh( signal + d ) * a;
    }
    float Harmonics::ModHT( float signal, float d, float a ){
        a = a > 1.0? a : 1.0;
        d = fclamp( d, 0.0, 0.5 );
        float x = ( signal + d ) * a;
        float y = exp( x ) - exp( -x * 1.2 ) / ( exp( x ) + exp( -x ) );
        // return dcblock_.Process( y - d );
        return y - d;
    }
    float Harmonics::Overdrive( float signal, float shape ){
        shape = shape > 0.0? shape : 0.0;
        float signX = signal >= 0.0? 1.0 : -1.0;
        float pX = signal * signX;
        float y = 1.0 - exp( shape * log( 1.0 - pX ) );
        y = fclamp( y, 0.0, 1.0 );
        return y * signX;
    }
    float Harmonics::AtanDrive( float signal, float drive ){
        drive = drive > 0.0? drive: 0.0;
        float outDrive = 1.0 / ( atan( drive ) );
        outDrive = outDrive > 0.1? outDrive : 0.1;
        float x = atan( signal * drive );
        return x * outDrive;
    }
    float Harmonics::PolyWs( float signal, float d ){
        float m = signal * d;
        d = d < 1.0? d : 1.0;
        float s = 1.0 / d;
        float b = ( m * m ) * 0.28;
        float a = b + 1.0;
        float n = m / a;
        // return dcblock_.Process( n * s );
        return n * s;
    }
    float Harmonics::PolyVws( float signal, float c, float g, float d ){
        c = fclamp( c, 0.0, 1.0 );
        g = fclamp( g, 0.25, 4.0 );
        float m = signal * d;
        d = d > 1.0? d : 1.0;
        float s = 1.0 / d;
        float b = ( m * m ) * c;
        float a = b + g;
        float y = m / a;
        // return dcblock_.Process( y * s );
        return y * s;
    }
    float Harmonics::ScDistort( float signal, float drive ){
        drive = drive > 0.0? drive : 0.0;
        float x1 = signal * drive;
        float x2 = abs( x1 ) + 1.0;
        return x1 / x2;
    }
    float Harmonics::ScSoftClip( float signal, float drive ){
        drive = drive > 0.0? drive: 0.0;
        float x1 = signal * drive;
        float x2 = ( x1 * x1 ) + 0.25;
        return x1 / x2;
    }
    float Harmonics::Process( float in ){
        float signal1 = TanhShape( in, fmap( balance_, 1.0, 10.0 ) ),
            // signal2 = ScSoftClip( in, 2.0 ),
            signal2 = TanhOffset( 
                in, 
                fmap( balance_, 0.1, 0.2 ), 
                fmap( balance_, 4.0, 10.0 ) 
            ),
            // signal2 = ModHT( in, 0.5, 10 ), // WORKS BUT WONKY
            // signal2 = PolyVws( in, 0.8, 2.0, 5.0 ), // WORKS BUT MEH.
            inAmount = 0.0,
            signal1Amount = 0.0,
            signal2Amount = 0.0;
        if( balance_ > 0.25 && balance_ < 0.75 )
            inAmount = balance_ > 0.5?
                1.0 - ( ( balance_ - 0.5 ) * 4.0 ) : 
                ( balance_ -0.25 ) * 4.0;
        if( balance_ >= 0.5 ) signal1Amount = ( balance_ - 0.5 ) * 2.0;
        else signal2Amount = ( abs( balance_ - 0.5 ) ) * 2.0;
        return ( in * inAmount ) + ( signal1 * signal1Amount ) + ( signal2 * signal2Amount );
    }
}