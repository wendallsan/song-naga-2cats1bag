![Song Naga Logo](https://songnaga.dansteeby.com/wp-content/uploads/2022/06/song-naga-logo-300x217.png)  
![Song Naga](https://songnaga.dansteeby.com/wp-content/uploads/2022/08/Untitled-1@4x.png)

##  Presents  

![2 Cats 1 Bag logo](https://songnaga.dansteeby.com/wp-content/uploads/2022/08/2-cats-1-bag.png)  

A Complex Oscillator for the Daisy Seed DSP platform, inspired by the Buchla 259.    

![Panel Concept](https://songnaga.dansteeby.com/wp-content/uploads/2022/08/panel-concept.png)  
Panel Concept Artwork, laid out to use the [Simple musical instrument design platform](https://www.synthux.academy/simple).  

This is a robust complex oscillator that provides an amazing range of sonic vistas through a Modulator that can affect the Primary Oscillator's frequency, sync, amplitude and wavefolding.  

![Flowchart of module audio signal](https://songnaga.dansteeby.com/wp-content/uploads/2022/08/2cats1bag-flowchart-03.png)

## Features  

### USB-MIDI Control  

The instrument is recognized as a USB-MIDI device, allowing control of the oscillators' frequencies by MIDI note data.

## Common Oscillator Controls  

Both the Modulator and Primary Oscillators share the following features:  

- **Coarse Pitch Knob** - adjusts the frequency of the oscillator by +/- 2 octaves
- **Fine Pitch Knob** - adjusts the frequency of the oscillator by +/- 2 semitones  

### Modulator Oscillator  

- **MIDI Control** - the Modulator accepts MIDI data on any MIDI channel except 2 when the Range Switch is not in LFO mode.  
- **Range Switch** - switches between audio-rate mode and LFO mode.  LFO mode rate ranges from 0.05 - 20Hz and incoming MIDI data is ignored.  
- **Wave Select Switch** - switches the Modulator Oscillator's waveform between sine and triangle waveforms
- **Frequency Mod Knob** - adjusts the amount of modulation of the frequency of the Primary Oscillator
- **Amp Mod Knob** - adjusts the amount of modulation of the amplifier
- **Timbre Mode Knob** - adjusts the amount of modulation of the wavefolder  

### Primary Oscillator  

- **MIDI Control** - the Modulator accepts MIDI data on channel 2  
- **Wave Select Switch** - switches the Primary Oscillator's waveform between triangle-like and square-like waveforms
- **Waveshape Knob** - changes the shape of the waveform between Saw / Triangle / Ramp when the Primary Oscillator's Wave Select Switch is set to triangle, or changes the pulse width of the squarewave when the Wave Select Switch is set to square.  
- **Sync Switch** - hard syncs the Primary Oscillator to the Modulator Oscillator when enabled  
- **Chaos Knob** - frequency modulates the Modulator by the Primary output- guaranteed to get weird!  

### Wavefolder   

- **Timbre Knob** - adjusts the amount of wavefolding applied to the output of the Primary Oscillator  
- **Symmetry Knob** - adjusts the offset of Primary Oscillator before it is sent into the wavefolder  

## Current Status of Project  

All basic features are implemented except for implementing the planned Harmonics Knob in he wavefolder and adding CV inputs and their related attenuators/attenuverters.  These will require some hardware that I don't have on hand yet to translate Eurorack-level control voltages to the range used by the Daisy Seed.  

## Yet to Implement  
- Eurorack-compatible CV input is planned, and are documented in the wiki.  
- The Harmonics processor has not been implemented yet.  
- An overhaul of the Primary Oscillator's waveshape feature is planned, and is documented in the wiki.  
- Tuning knob ranges and responses to elicit musically-oriented/pleasing sounding results.  
