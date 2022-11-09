/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
Sjf_chebyShevAudioProcessorEditor::Sjf_chebyShevAudioProcessorEditor (Sjf_chebyShevAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    addAndMakeVisible( polyAmounts );
    
    polyAmounts.setNumSliders( 5 );
    
    startTimer(100);
    setSize (400, 300);
}

Sjf_chebyShevAudioProcessorEditor::~Sjf_chebyShevAudioProcessorEditor()
{
}

//==============================================================================
void Sjf_chebyShevAudioProcessorEditor::paint (juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));

    g.setColour (juce::Colours::white);
    g.setFont (15.0f);
    g.drawFittedText ("Hello World!", getLocalBounds(), juce::Justification::centred, 1);
}

void Sjf_chebyShevAudioProcessorEditor::resized()
{
    polyAmounts.setBounds( 10, 10, getWidth() - 20, getHeight() - 20 );
}


void Sjf_chebyShevAudioProcessorEditor::timerCallback()
{
    for (int i = 0; i < 5; i++ )
    {
        DBG( polyAmounts.fetch( i ) );
        audioProcessor.setLevels( i, polyAmounts.fetch( i ) );
    }
//
}
