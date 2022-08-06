#include "daisy_seed.h"
#include "daisysp.h"
#include "wavefolder.h"

using namespace daisy;
using namespace daisysp;

DaisySeed hw;
MidiUsbHandler midi;
Oscillator modulator;
VariableShapeOscillator primary;
Wavefolder fold;
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
	modulatorAmpKnob,
	modulatorTimbreModKnob,
	folderTimbreKnob,
	folderSymmetryKnob,
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
		float modulatorSignal = modulator.Process();
		float frequencyModSignal = modulatorSignal * modulatorFrequencyModAmountAdjust * 10.0;
		float ampModSignal = modulatorSignal * modulatorAmpModAmountAdjust;
		primary.SetSyncFreq( primaryFrequency * ( frequencyModSignal + 1.0 ) );
		float finalFold = folderTimbreAdjust + ( (  folderTimbreModAdjust / 2.0 ) * modulatorSignal ) ;
		finalFold = fmap( finalFold, 1.0, 10.0 ); // MAP finalFold TO A RANGE BETWEEN 1 AND 10
		fold.SetGain( finalFold ); // SET FOLD GAIN		
		float finalSignal = primary.Process(); // GET THE PRIMARY OSC SIGNAL
		finalSignal = finalSignal * ( ampModSignal + 0.5 ); // MODIFY AMPLITUDE ACCORDING TO AMP MOD
		finalSignal = finalSignal + fmap( folderSymmetryAdjust, -1.0, 1.0 ); // OFFSET SIGNAL ACCORDING TO SYMMETRY
		finalSignal = fold.Process( finalSignal ); // MODIFY THROUGH WAVEFOLDER
		out[0][i] = modulatorSignal; // SEND THE MODULATOR SIGNAL TO OUTPUT 1
        out[1][i] = finalSignal; // SEND THE PRIMARYSIGNAL TO OUTPUT 2
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
		modulatorFrequency = setCoarseFrequency(  mtof( modulatorMidiNote ), modulatorCoarsePitchAdjust );
	modulator.SetFreq( modulatorFrequency );
	// ALSO SET FREQUENCY ON PRIMARY OSC FOR SYNC PURPOSES
	// NOTE THAT WE DON'T HEAR THIS OSCILLATOR, WE JUST HEAR THE 
	// OSCILLATOR WHOSE FREQ IS SET USING THE SetSyncFreq METHOD
	// ( THIS IS DONE IN THE AUDIOCALLBACK )
	primary.SetFreq( modulatorFrequency );	
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
	primaryWaveSelectButton.Init( hw.GetPin( 28 ), 100 );
	syncSelectButton.Init( hw.GetPin( 29 ), 100 );

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
	adcConfig[ modulatorAmpKnob ].InitSingle( daisy::seed::A8 );
    adcConfig[ folderTimbreKnob ].InitSingle( daisy::seed::A6 );
    adcConfig[ folderSymmetryKnob ].InitSingle( daisy::seed::A7 );
    adcConfig[ modulatorTimbreModKnob ].InitSingle( daisy::seed::A9 );
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
		primary.SetWaveshape( primaryWaveSelectButton.Pressed()? 1.0 : 0.0 );		
		// SET THE MODULATOR WAVEFORM ACCORDING TO THE BUTTON STATE
		// BUG: DAISY OVERLOADS IN LFO MODE AT THE LOWEST SETTINGS WHEN FM MOD IS TURNED UP HIGH
		modulator.SetWaveform( modulatorWaveSelectButton.Pressed()? 
			modulator.WAVE_SIN :
			modulator.WAVE_POLYBLEP_TRI
		);
		System::Delay( 1 );
	}
}