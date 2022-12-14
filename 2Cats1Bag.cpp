#include "daisy_seed.h"
#include "daisysp.h"
#include "Wavefolder.h"
#include "Harmonics.h"
#include "ShapedOsc.h"

using namespace daisy;
using namespace daisysp;

DaisySeed hw;
MidiUsbHandler midi;
Oscillator modulator;
// VariableShapeOscillator primary;
ShapedOsc primary;
Wavefolder fold;
Harmonics harmonics;
float modulatorCoarsePitchAdjust, 
	modulatorFinePitchAdjust, 
	modulatorFrequency, 
	modulatorFrequencyModAmountAdjust,
	modulatorAmpModAmountAdjust,	
	primaryCoarsePitchAdjust, 
	primaryFinePitchAdjust, 
	primaryFrequency, 
	primaryWaveshapeAdjust,	
	folderTimbreModAdjust,
	folderTimbreAdjust,
	folderSymmetryAdjust,
	ampLevel,
	harmonicsAdjust,
	chaosAdjust,
	lastFinalModulatorFrequency = 0.0,
	lastPrimarySignal = 0.5,
	lastFinalPrimaryFrequency = 0.0,
	twoSemitonesRatio = 0.059463 * 2.0;
int primaryMidiNote = 36,
	modulatorMidiNote = 36;
int debugIterator = 0;
enum AdcChannel {
	primaryCoarsePitchKnob,
	primaryFinePitchKnob,
	primaryWaveshapeKnob,	
	modulatorCoarsePitchKnob,
	modulatorFinePitchKnob,
	modulatorFrequencyModKnob,
	modulatorAmpKnob,
	modulatorTimbreModKnob,
	folderTimbreKnob,
	folderSymmetryKnob,
	harmonicsKnob,
	chaosKnob,
	NUM_ADC_CHANNELS
};
Switch primaryWaveSelectButton, 
	modulatorWaveSelectButton, 
	modulatorRangeSelectButton,
	syncSelectButton;
void AudioCallback(
	AudioHandle::InputBuffer in,
	AudioHandle::OutputBuffer out,
	size_t size
){
	for( size_t i = 0; i < size; i++ ){
		// HANDLE CHAOS KNOB
		float chaosMod = chaosAdjust > 0.0? lastPrimarySignal * chaosAdjust * 10.0 : 1.0;
		float finalModulatorFrequency = fclamp( modulatorFrequency * ( chaosMod + 1.0), 0.01, 20000.0 );		
		lastFinalModulatorFrequency = finalModulatorFrequency;

		modulator.SetFreq( finalModulatorFrequency );
		/* 
		BUG: THERE IS A 'JITTER' IN THE MOD OSC WHEN THE CHAOS KNOB IS AT ZERO
		THIS JITTER GOES AWAY IF YOU ADJUST THE CHAOS KNOB UP JUST A LITTLE.
		THIS ONLY OCCURS ON WHEN MOD WAVEFORM SELECT IS SET TO SINE.
		*/
		// ALSO SET FREQUENCY ON PRIMARY OSC FOR SYNC PURPOSES
		/* 
		NOTE THAT WE DON'T HEAR THIS OSCILLATOR, WE JUST HEAR THE 
		OSCILLATOR WHOSE FREQ IS SET USING THE SetSyncFreq METHOD
		( THIS IS DONE IN THE AUDIOCALLBACK )
		*/ 
		primary.SetSyncFreq( finalModulatorFrequency );	
		float modulatorSignal = modulator.Process();
		float frequencyModSignal = modulatorSignal * ( modulatorFrequencyModAmountAdjust * 10.0 );
		float ampModSignal = modulatorSignal * modulatorAmpModAmountAdjust;
		// CLAMP THE PRIMARY OSC FREQUENCY AT AUDIO RANGES (20HZ MIN)
		float finalPrimaryFrequency = fclamp( primaryFrequency * ( frequencyModSignal + 1.0 ), 20.0, 20000.0  );
		primary.SetFreq( finalPrimaryFrequency );
		float finalFold = folderTimbreAdjust + ( (  folderTimbreModAdjust / 2.0 ) * modulatorSignal ) ;
		finalFold = fmap( finalFold, 1.0, 10.0 ); // MAP finalFold TO A RANGE BETWEEN 1 AND 10
		fold.SetGain( finalFold ); // SET FOLD GAIN		
		float primarySignal = primary.Process( modulator.IsEOC() ); // GET THE PRIMARY OSC SIGNAL
		lastPrimarySignal = primarySignal; // STORE THE SIGNAL AT THIS POINT FOR CHAOS MODULATION PURPOSES
		lastFinalPrimaryFrequency = finalPrimaryFrequency;		
		primarySignal = primarySignal * ( ampModSignal + 0.5 ); // MODIFY AMPLITUDE ACCORDING TO AMP MOD
		primarySignal = primarySignal + fmap( folderSymmetryAdjust, -1.0, 1.0 ); // OFFSET SIGNAL ACCORDING TO SYMMETRY
		primarySignal = fold.Process( primarySignal ); // MODIFY THROUGH WAVEFOLDER
		primarySignal = harmonics.Process( primarySignal );  // MODIFY THROUGH HARMONICS
		out[0][i] = modulatorSignal; // SEND THE MODULATOR SIGNAL TO OUTPUT 1
		out[1][i] = primarySignal; // SEND THE PRIMARYSIGNAL TO OUTPUT 2
	}
}
void handleMidi(){
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
		}
	}
}
void handleKnobs(){
	modulatorCoarsePitchAdjust = hw.adc.GetFloat( modulatorCoarsePitchKnob );
	modulatorFinePitchAdjust = hw.adc.GetFloat( modulatorFinePitchKnob );
	modulatorFrequencyModAmountAdjust = hw.adc.GetFloat( modulatorFrequencyModKnob );
	modulatorAmpModAmountAdjust = hw.adc.GetFloat( modulatorAmpKnob );
	primaryCoarsePitchAdjust = hw.adc.GetFloat( primaryCoarsePitchKnob );
	primaryFinePitchAdjust = hw.adc.GetFloat( primaryFinePitchKnob );
	primaryWaveshapeAdjust = hw.adc.GetFloat( primaryWaveshapeKnob );
	folderTimbreAdjust = hw.adc.GetFloat( folderTimbreKnob );
	folderSymmetryAdjust = hw.adc.GetFloat( folderSymmetryKnob );
	folderTimbreModAdjust = hw.adc.GetFloat( modulatorTimbreModKnob );	
	harmonicsAdjust = hw.adc.GetFloat( harmonicsKnob );	
	chaosAdjust = hw.adc.GetFloat( chaosKnob );	
}
float setCoarseFrequency( float baseFrequency, float adjust ){ // COARSE ADJUST THE FREQUENCY BY +/- 2 OCTAVES
	return fmap( adjust,  baseFrequency / 4.0, baseFrequency * 4.0 );
}
float setFineFrequency( float baseFrequency, float adjust ){ // FINE ADJUST THE FREQUENCY BY +/- 2 SEMITONES
	return  fmap(
		adjust, 
		baseFrequency - ( baseFrequency * twoSemitonesRatio ), 
		baseFrequency + ( baseFrequency * twoSemitonesRatio )
	);
}
void setPrimaryFrequency(){  // CALCULATE AND SET PRIMARY FREQUENCY
	primaryFrequency = setCoarseFrequency( mtof( primaryMidiNote ), primaryCoarsePitchAdjust );
	primaryFrequency = setFineFrequency ( primaryFrequency, primaryFinePitchAdjust );
}
void setModulatorFrequency(){ // CALCULATE AND SET MODULATOR FREQUENCY
	// IF RANGE IS LFO, IGNORE MIDI AND MAP THE COARSE AND FINE KNOBS TO A RANGE OF 0.05 - 20HZ
	if( !modulatorRangeSelectButton.Pressed() )
		modulatorFrequency = fmap( modulatorCoarsePitchAdjust, 0.05, 20 );		
	else // ELSE SET FREQUENCY TO MIDI NOTE, COARSE, AND FINE KNOBS
		modulatorFrequency = setCoarseFrequency( mtof( modulatorMidiNote ), modulatorCoarsePitchAdjust );
}
void debounceButtons(){
	modulatorRangeSelectButton.Debounce();
	modulatorWaveSelectButton.Debounce();
	primaryWaveSelectButton.Debounce(); 
	syncSelectButton.Debounce();
}
int main( void ){
	hw.Configure();
	hw.Init();
	modulatorRangeSelectButton.Init( hw.GetPin( 26 ), 100 );
	modulatorWaveSelectButton.Init( hw.GetPin( 27 ), 100 );
	primaryWaveSelectButton.Init( hw.GetPin( 14 ), 100 );
	syncSelectButton.Init( hw.GetPin( 29 ), 100 );
	MidiUsbHandler::Config midiConfig;
	midiConfig.transport_config.periph = MidiUsbTransport::Config::INTERNAL;
    midi.Init( midiConfig );
	hw.SetAudioSampleRate( SaiHandle::Config::SampleRate::SAI_48KHZ );
	float sampleRate = hw.AudioSampleRate();
	primary.Init( sampleRate );
	modulator.Init( sampleRate );
	harmonics.Init( sampleRate );
	hw.StartAudio( AudioCallback );
	AdcChannelConfig adcConfig[ NUM_ADC_CHANNELS ];
    adcConfig[ modulatorCoarsePitchKnob ].InitSingle( daisy::seed::A0 );
    adcConfig[ modulatorFinePitchKnob ].InitSingle( daisy::seed::A1 );
    adcConfig[ modulatorFrequencyModKnob ].InitSingle( daisy::seed::A2 );
    adcConfig[ primaryCoarsePitchKnob ].InitSingle( daisy::seed::A3 );
    adcConfig[ primaryFinePitchKnob ].InitSingle( daisy::seed::A4 );
    adcConfig[ primaryWaveshapeKnob ].InitSingle( daisy::seed::A5 );
	adcConfig[ modulatorAmpKnob ].InitSingle( daisy::seed::A8 );
    adcConfig[ folderTimbreKnob ].InitSingle( daisy::seed::A6 );
    adcConfig[ folderSymmetryKnob ].InitSingle( daisy::seed::A7 );
    adcConfig[ modulatorTimbreModKnob ].InitSingle( daisy::seed::A9 );
    adcConfig[ harmonicsKnob ].InitSingle( daisy::seed::A10 );
    adcConfig[ chaosKnob ].InitSingle( daisy::seed::A11 );
    hw.adc.Init( adcConfig, NUM_ADC_CHANNELS );
    hw.adc.Start();
	while( true ){
		debounceButtons();
		handleMidi();
		handleKnobs();		
		// SET PRIMARY PULSEWIDTH / WAVESHAPE
		primary.SetPW( primaryWaveSelectButton.Pressed()? fmap( primaryWaveshapeAdjust, 0.2, 0.8 ) : primaryWaveshapeAdjust );
		primary.SetSync( syncSelectButton.Pressed() );
		setModulatorFrequency();
		setPrimaryFrequency();
		// SET THE PRIMARY WAVEFORM ACCORDING TO THE BUTTON STATE
		primary.SetWaveshape( primaryWaveshapeAdjust );
		// SET THE MODULATOR WAVEFORM ACCORDING TO THE BUTTON STATE
		modulator.SetWaveform( modulatorWaveSelectButton.Pressed()? 
			modulator.WAVE_SIN :
			modulator.WAVE_POLYBLEP_TRI
		);
		primary.SetWaveformMode( primaryWaveSelectButton.Pressed() );
		harmonics.SetBalance( harmonicsAdjust );
		System::Delay( 1 );
	}
}