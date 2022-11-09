/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "/Users/simonfay/Programming_Stuff/sjf_audio/sjf_lookAndFeel.h"
#include "/Users/simonfay/Programming_Stuff/sjf_audio/sjf_multislider.h"
//==============================================================================
/**
*/
class Sjf_chebyShevAudioProcessorEditor  : public juce::AudioProcessorEditor, public juce::Timer
{
public:
    Sjf_chebyShevAudioProcessorEditor (Sjf_chebyShevAudioProcessor&);
    ~Sjf_chebyShevAudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;

private:
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    Sjf_chebyShevAudioProcessor& audioProcessor;

    
    sjf_multislider polyAmounts;
    
    void timerCallback() override;
    
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Sjf_chebyShevAudioProcessorEditor)
};
