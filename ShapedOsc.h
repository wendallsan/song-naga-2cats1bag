#pragma once
#ifndef DSY_SN_SHAPEDOSC_H
#define DSY_SN_SHAPEDOSC_H

#include "daisysp.h"
#include <stdint.h>
#include "Utility/dsp.h"
#ifdef __cplusplus

namespace daisysp {
    class ShapedOsc {
        public:
            ShapedOsc(){}
            ~ShapedOsc(){}
            void Init( float );
            float Process();
            float Process( bool );
            void SetFreq( float );
            void SetWaveshape( float );
            void SetWaveformMode( bool );
            void SetPW( float );
            void SetSyncFreq( float );
            void SetSync( bool );
        private:
            void initOscillators();
            bool waveformMode_,
                enableSync_;
            Oscillator sineOsc_;
            BlOsc squareOsc_;
            VariableShapeOscillator triSawOsc_;
            float sampleRate_,
                freq_,
                waveShape_,
                pw_;
    };
};
#endif
#endif