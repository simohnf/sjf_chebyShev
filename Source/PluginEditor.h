/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "/Users/simonfay/Programming_Stuff/sjf_audio/sjf_LookAndFeel.h"
#include "/Users/simonfay/Programming_Stuff/sjf_audio/sjf_multislider.h"
//==============================================================================
/**
*/
class Sjf_chebyShevAudioProcessorEditor  : public juce::AudioProcessorEditor /*, public juce::Timer */
{
public:
    Sjf_chebyShevAudioProcessorEditor (Sjf_chebyShevAudioProcessor&, juce::AudioProcessorValueTreeState& vts);
    ~Sjf_chebyShevAudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;

private:
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    Sjf_chebyShevAudioProcessor& audioProcessor;
    juce::AudioProcessorValueTreeState& valueTreeState;
    sjf_lookAndFeel otherLookandFeel; 
    
    
    sjf_multislider chebyMultiSlider;
//
//    void timerCallback() override;
    
    juce::Slider mixSlider, inputDriveSlider, inputLPFSlider, outputHPFSlider, outputLPFSlider;
    juce::Label mixLabel, inputDriveLabel, inputLPFLabel, outputLPFLabel, outputHPFLabel;
    
    
    std::unique_ptr< juce::AudioProcessorValueTreeState::SliderAttachment > mixSliderAttachment, inputDriveSliderAttachment, inputLPFSliderAttachment, outputHPFSliderAttachment, outputLPFSliderAttachment;
    std::vector< std::unique_ptr< juce::AudioProcessorValueTreeState::SliderAttachment > > chebySliderAttachment;
    
    
    juce::SharedResourcePointer<juce::TooltipWindow> tooltipWindow;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Sjf_chebyShevAudioProcessorEditor)
};
