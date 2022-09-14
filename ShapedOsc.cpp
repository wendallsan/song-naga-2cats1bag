#include "ShapedOsc.h"
#include <algorithm>
namespace daisysp {
    void ShapedOsc::Init( float sampleRate ){
        sampleRate_ = sampleRate;
        freq_ = 220.0; // INIT FREQ TO 220HZ
        waveformMode_ = false; // INIT TO SAW MODE
        initOscillators();
    }
    void ShapedOsc::initOscillators(){                
        sineOsc_.Init( sampleRate_ );
        sineOsc_.SetWaveform( sineOsc_.WAVE_SIN );
        sineOsc_.SetFreq( freq_ );
        sineOsc_.SetAmp( 1.0 );
        sineOsc_.PhaseAdd( 0.25 );
        squareOsc_.Init( sampleRate_ );
        squareOsc_.SetWaveform( squareOsc_.WAVE_SQUARE );
        squareOsc_.SetFreq( freq_ );
        squareOsc_.SetAmp( 1.0 );
        triSawOsc_.Init( sampleRate_ );
        triSawOsc_.SetWaveshape( 0 );
        triSawOsc_.SetFreq( freq_ );
        triSawOsc_.SetSyncFreq( freq_ );
        triSawOsc_.SetPW( 0.0 );
        triSawOsc_.SetSync( false );
    }
    float ShapedOsc::Process( bool syncReset ){
        // HANDLE SYNC
        if( enableSync_ ){
            if( syncReset ){
                sineOsc_.Reset();
                squareOsc_.Reset();
            }
        }
        return waveformMode_?
			waveShape_ <= 0.5?
				( squareOsc_.Process() * ( waveShape_ * 2.0 ) ) -
				( sineOsc_.Process() * ( 1.0 - ( waveShape_ * 2.0 ) ) ) : 
				squareOsc_.Process() : 
			waveShape_ <= 0.5?
				triSawOsc_.Process() * ( waveShape_ * 2.0 ) + 
				sineOsc_.Process() * ( 1.0 - ( waveShape_ * 2.0 ) ) : 
				triSawOsc_.Process();
    }
    float ShapedOsc::Process(){
        return Process( false );
    }    
    void ShapedOsc::SetSyncFreq( float freq ){
        triSawOsc_.SetFreq( freq );
    }
    void ShapedOsc::SetFreq( float freq ){
        freq_ = freq;
        // SET FREQ ON OSCILLATORS
        sineOsc_.SetFreq( freq );
        squareOsc_.SetFreq( freq );
        triSawOsc_.SetSyncFreq( freq );
    }
    void ShapedOsc::SetWaveformMode( bool mode ){
        waveformMode_ = mode;
    }
    void ShapedOsc::SetWaveshape( float shape ){
        waveShape_ = shape;
    }
    void ShapedOsc::SetPW( float pw ){
        squareOsc_.SetPw( pw );
        triSawOsc_.SetPW( pw );
    }
    void ShapedOsc::SetSync( bool enable ){
        enableSync_ = enable;
        triSawOsc_.SetSync( enable );
    }
}