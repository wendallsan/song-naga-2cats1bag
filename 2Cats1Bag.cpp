#include "daisy_seed.h"
#include "daisysp.h"

using namespace daisy;
using namespace daisysp;

DaisySeed hw;
MidiUsbHandler midi;
Oscillator modulator;
VariableShapeOscillator primary;
Fold fold;
float modulatorFrequencyModAmountAdjust, 
	modulatorCoarsePitchAdjust, 
	modulatorFinePitchAdjust, 
	modulatorFrequency, 
	
	primaryCoarsePitchAdjust, 
	primaryFinePitchAdjust, 
	primaryFrequency, 
	primaryWaveshapeAdjust,
	
	folderTimbreAdjust,
	folderSymmetryAdjust,

	twoSemitonesRatio = 0.059463 * 2.0;

int primaryMidiNote = 24,
	modulatorMidiNote = 24;

enum AdcChannel {
	primaryCoarsePitchKnob,
	primaryFinePitchKnob,
	primaryWaveshapeKnob,
	
	modulatorCoarsePitchKnob,
	modulatorFinePitchKnob,
	modulatorFrequencyModKnob,
	// modulatorAmpKnob,
	// modulatorTimbreKnob,

	folderTimbreKnob,
	folderSymmetryKnob,

	NUM_ADC_CHANNELS
};

Switch primaryWaveSelectButton, 
	modulatorWaveSelectButton, 
	modulatorRangeSelectButton;

void AudioCallback(
	AudioHandle::InputBuffer in,
    AudioHandle::OutputBuffer out,
    size_t size
){
	for( size_t i = 0; i < size; i++ ){
		float modulatorSignal = modulator.Process();
		float adjustedModulatorSignal = ( modulatorSignal * 2 ) * modulatorFrequencyModAmountAdjust;
		primary.SetSyncFreq( primaryFrequency * ( adjustedModulatorSignal + 1.0 ) );

		float finalSignal = primary.Process();
		// TODO: FOLD!
		/*
		Symmetry – Adds an offset to the input so the top half of the wave starts folding earlier.
		Harmonics - A square wave has all odd harmonics. A saw wave has all harmonics. Subtract a square wave from 
		a saw wave and you get all even harmonics. You can add odd harmonics to a signal by clipping the 
		tops and bottoms of the input (making it more square). So if you clip the input you are 
		emphasizing odd harmonics, and if you take that clipped signal and subtract it from the input 
		you get an emphasis on even harmonics.  
		Chebyshev distortion might be useful to look into. It is doing similar things, emphasizing certain harmonics.
		http://www.endino.com/archive/arch2.html
		*/
		finalSignal = finalSignal + fmap( folderSymmetryAdjust, -1.0, 1.0 );
		
		// MAYBE WE NEED TO INCREASE VOLUME INTO THE FOLDER? nope.
		// finalSignal = finalSignal * ( 1.0 +  ( folderTimbreAdjust * 10.0 ) );
		
		finalSignal = fold.Process( finalSignal );
		out[0][i] = modulatorSignal;
        out[1][i] = finalSignal;
	}
}

void handleMidi(){
	// GET MIDI NOTE NUMBER
	midi.Listen();
	while( midi.HasEvents() ){
		auto midiEvent = midi.PopEvent();
		if( midiEvent.type == NoteOn ){
			auto noteMessage = midiEvent.AsNoteOn();
			if( noteMessage.velocity != 0 ){
				switch( noteMessage.channel ){
					case 0:
						modulatorMidiNote = noteMessage.note;
						break;
					default: 
						primaryMidiNote = noteMessage.note;
				}
			}			
			// TODO: ADD CC CONTROLS FOR KNOBS AND STUFF
		}
	}
}

void handleKnobs(){
	// GET KNOB POSITIONS
	modulatorCoarsePitchAdjust = hw.adc.GetFloat( modulatorCoarsePitchKnob );
	modulatorFinePitchAdjust = hw.adc.GetFloat( modulatorFinePitchKnob );
	modulatorFrequencyModAmountAdjust = hw.adc.GetFloat( modulatorFrequencyModKnob );

	primaryCoarsePitchAdjust = hw.adc.GetFloat( primaryCoarsePitchKnob );
	primaryFinePitchAdjust = hw.adc.GetFloat( primaryFinePitchKnob );
	primaryWaveshapeAdjust = hw.adc.GetFloat( primaryWaveshapeKnob );

	folderTimbreAdjust = hw.adc.GetFloat( folderTimbreKnob );
	folderSymmetryAdjust = hw.adc.GetFloat( folderSymmetryKnob );
	
}

void setPrimaryFrequency(){  // CALCULATE AND SET PRIMARY FREQUENCY
		primaryFrequency = mtof( primaryMidiNote ); // GET FREQUENCY FROM MIDI NOTE
		primaryFrequency = fmap( // COARSE ADJUST THE FREQUENCY UP OR DOWN BY 2 OCTAVES 
			primaryCoarsePitchAdjust, 
			primaryFrequency / 4.0, 
			primaryFrequency * 4.0
		);
		primaryFrequency = fmap( // FINE ADJUST THE FREQUENCY UP OR DOWN BY 2 SEMITONES
			primaryFinePitchAdjust, 
			primaryFrequency - ( primaryFrequency * twoSemitonesRatio ), 
			primaryFrequency + ( primaryFrequency * twoSemitonesRatio )
		);
}

void setModulatorFrequency(){ // CALCULATE AND SET MODULATOR FREQUENCY
		if( !modulatorRangeSelectButton.Pressed() ){ // SET FREQUENCY COARSE KNOB RANGE TO 0.05 - 10 HZ
			// IGNORE MIDI NOTE IN FOR LFO-Y STUFF (??)
			modulatorFrequency = fmap( modulatorCoarsePitchAdjust, 0.05, 10 );		
		} else { // SET FREQUENCY TO MIDI NOTE AND COARSE KNOBS
			modulatorFrequency = mtof( modulatorMidiNote ); // GET FREQUENCY FROM MIDI NOTE
			modulatorFrequency = fmap( // COARSE ADJUST THE FREQUENCY BY +/- 2 OCTAVES 
				modulatorCoarsePitchAdjust, 
				modulatorFrequency / 4.0, 
				modulatorFrequency * 4.0
			);
		}
		modulatorFrequency = fmap( // FINE ADJUST THE FREQUENCY BY +/- 2 SEMITONES
			modulatorFinePitchAdjust, 
			modulatorFrequency - ( modulatorFrequency * twoSemitonesRatio ), 
			modulatorFrequency + ( modulatorFrequency * twoSemitonesRatio )
		);
		modulator.SetFreq( modulatorFrequency );		
}

int main( void ){
	hw.Configure();
	hw.Init();
	modulatorRangeSelectButton.Init( hw.GetPin( 26 ), 100 );
	modulatorWaveSelectButton.Init( hw.GetPin( 27 ), 100 );
	primaryWaveSelectButton.Init( hw.GetPin( 28 ), 100 );

	MidiUsbHandler::Config midiConfig;
    midiConfig.transport_config.periph = MidiUsbTransport::Config::INTERNAL;
    midi.Init( midiConfig );
	hw.SetAudioSampleRate( SaiHandle::Config::SampleRate::SAI_48KHZ );
	float sampleRate = hw.AudioSampleRate();

	primary.Init( sampleRate );
    modulator.Init( sampleRate );

	hw.StartAudio( AudioCallback );

	AdcChannelConfig adcConfig[ NUM_ADC_CHANNELS ];

    adcConfig[ modulatorCoarsePitchKnob ].InitSingle( daisy::seed::A0 );
    adcConfig[ modulatorFinePitchKnob ].InitSingle( daisy::seed::A1 );
    adcConfig[ modulatorFrequencyModKnob ].InitSingle( daisy::seed::A2 );

    adcConfig[ primaryCoarsePitchKnob ].InitSingle( daisy::seed::A3 );
    adcConfig[ primaryFinePitchKnob ].InitSingle( daisy::seed::A4 );
    adcConfig[ primaryWaveshapeKnob ].InitSingle( daisy::seed::A5 );
	
    adcConfig[ folderTimbreKnob ].InitSingle( daisy::seed::A6 );
    adcConfig[ folderSymmetryKnob ].InitSingle( daisy::seed::A7 );

    hw.adc.Init( adcConfig, NUM_ADC_CHANNELS );
    hw.adc.Start();

	while( true ){
		// DEBOUNCE THE BUTTONS BEFORE WE USE THEM
		modulatorRangeSelectButton.Debounce();
		modulatorWaveSelectButton.Debounce();
		primaryWaveSelectButton.Debounce(); 

		handleMidi();
		handleKnobs();
		
		// SET PRIMARY PULSEWIDTH / WAVESHAPE
		primary.SetPW( primaryWaveSelectButton.Pressed()? fmap( primaryWaveshapeAdjust, 0.2, 0.8 ) : primaryWaveshapeAdjust );

		setModulatorFrequency();
		setPrimaryFrequency();

		// SET THE WAVEFORM ACCORDING TO THE BUTTON STATE
		primary.SetWaveshape( primaryWaveSelectButton.Pressed()? 1.0 : 0.0 );		
		
		// SET THE WAVEFORM ACCORDING TO THE BUTTON STATE
		modulator.SetWaveform( modulatorWaveSelectButton.Pressed()? 
			modulator.WAVE_SIN : 
			modulator.WAVE_POLYBLEP_TRI 
		);

		// SET THE TIMBRE
		fold.SetIncrement( 1.0 );

		System::Delay( 1 );
	}
}