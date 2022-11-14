/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

#define potSize 80
#define textHeight 20
#define alph 0.5f
#define inputParamsBackgroundColour juce::Colour(25, 50, 25).withAlpha( alph )
#define outputParamsBackgroundColour juce::Colour(5, 25, 50).withAlpha( alph )
#define mixBackgroundColour juce::Colour(50, 5, 50).withAlpha( alph )

//==============================================================================
Sjf_chebyShevAudioProcessorEditor::Sjf_chebyShevAudioProcessorEditor (Sjf_chebyShevAudioProcessor& p, juce::AudioProcessorValueTreeState& vts)
: AudioProcessorEditor (&p), audioProcessor (p), valueTreeState (vts)
{
    setLookAndFeel( &otherLookandFeel );
    addAndMakeVisible( chebyMultiSlider );
    
    const int nSliders = audioProcessor.getNumChebyOrders();
    chebyMultiSlider.setNumSliders( nSliders );
//    std::vector< std::unique_ptr< juce::AudioProcessorValueTreeState::SliderAttachment > > chebySliderAttachment ( nSliders );
    chebySliderAttachment.resize( nSliders );
    auto chebySliders = chebyMultiSlider.getSliderArray();
    for ( int i = 0; i < nSliders; i++ )
    {
//        chebyMultiSlider.setSliderValue( i, audioProcessor.getLevel( i ) );
        
        std::string name = "cheby" + std::to_string( i + 2 ) ;
        chebySliderAttachment[ i ].reset (new juce::AudioProcessorValueTreeState::SliderAttachment (valueTreeState, name, *chebySliders[ i ] ) );
    }
    chebyMultiSlider.setTooltip( "Levels for each chebyshev polynomial..." );
    
    mixSliderAttachment.reset (new juce::AudioProcessorValueTreeState::SliderAttachment (valueTreeState, "mix", mixSlider));
    addAndMakeVisible( &mixSlider );
    mixSlider.setSliderStyle (juce::Slider::Rotary);
    mixSlider.setTextBoxStyle (juce::Slider::TextBoxBelow, false, potSize, textHeight);
    mixSlider.setNumDecimalPlacesToDisplay(3);
    mixSlider.setTextValueSuffix ("%");
    mixSlider.setTooltip( "Mix of dry and wet, 0% = all dry, 100% = all wet" );
    mixLabel.attachToComponent(&mixSlider, false);
    mixLabel.setText("Mix", juce::dontSendNotification);
    mixLabel.setJustificationType( juce::Justification::centred );
    
    inputDriveSliderAttachment.reset (new juce::AudioProcessorValueTreeState::SliderAttachment (valueTreeState, "inputDrive", inputDriveSlider));
    addAndMakeVisible( &inputDriveSlider );
    inputDriveSlider.setSliderStyle (juce::Slider::Rotary);
    inputDriveSlider.setTextBoxStyle (juce::Slider::TextBoxBelow, false, potSize, textHeight);
    inputDriveSlider.setNumDecimalPlacesToDisplay(3);
    inputDriveSlider.setTextValueSuffix ("%");
    inputDriveSlider.setTooltip( "Level of input to the chebyshev waveshapers" );
    inputDriveLabel.attachToComponent( &inputDriveSlider, false );
    inputDriveLabel.setText( "In Drive", juce::dontSendNotification);
    inputDriveLabel.setJustificationType( juce::Justification::centred );
    
    inputLPFSliderAttachment.reset (new juce::AudioProcessorValueTreeState::SliderAttachment (valueTreeState, "inLPF", inputLPFSlider));
    addAndMakeVisible( &inputLPFSlider );
    inputLPFSlider.setRange( 20.0f, 20000.0f );
    inputLPFSlider.setSliderStyle (juce::Slider::Rotary);
    inputLPFSlider.setTextBoxStyle (juce::Slider::TextBoxBelow, false, potSize, textHeight);
    inputLPFSlider.setNumDecimalPlacesToDisplay(3);
    inputLPFSlider.setTextValueSuffix ("Hz");
    inputLPFSlider.setSkewFactorFromMidPoint( 1000.0f );
    inputLPFSlider.setTooltip( "Cutoff frequency for lowpass filter on input to chebyshev waveshapers" );
    inputLPFLabel.attachToComponent( &inputLPFSlider, false );
    inputLPFLabel.setText( "In LPF", juce::dontSendNotification );
    inputLPFLabel.setJustificationType( juce::Justification::centred );
    
    outputHPFSliderAttachment.reset (new juce::AudioProcessorValueTreeState::SliderAttachment (valueTreeState, "outHPF", outputHPFSlider));
    addAndMakeVisible( &outputHPFSlider );
    outputHPFSlider.setRange( 20.0f, 20000.0f );
    outputHPFSlider.setSliderStyle (juce::Slider::Rotary);
    outputHPFSlider.setTextBoxStyle (juce::Slider::TextBoxBelow, false, potSize, textHeight);
    outputHPFSlider.setNumDecimalPlacesToDisplay(3);
    outputHPFSlider.setTextValueSuffix ("Hz");
    outputHPFSlider.setSkewFactorFromMidPoint( 1000.0f );
    outputHPFSlider.setTooltip( "Cutoff frequency for highpass filter on output of chebyshev waveshapers" );
    outputHPFLabel.attachToComponent( &outputHPFSlider, false );
    outputHPFLabel.setText( "Out HPF", juce::dontSendNotification );
    outputHPFLabel.setJustificationType( juce::Justification::centred );
    
    outputLPFSliderAttachment.reset (new juce::AudioProcessorValueTreeState::SliderAttachment (valueTreeState, "outLPF", outputLPFSlider));
    addAndMakeVisible( &outputLPFSlider );
    outputLPFSlider.setRange( 20.0f, 20000.0f );
    outputLPFSlider.setSliderStyle (juce::Slider::Rotary);
    outputLPFSlider.setTextBoxStyle (juce::Slider::TextBoxBelow, false, potSize, textHeight);
    outputLPFSlider.setNumDecimalPlacesToDisplay(3);
    outputLPFSlider.setTextValueSuffix ("Hz");
    outputLPFSlider.setSkewFactorFromMidPoint( 1000.0f );
    outputLPFSlider.setTooltip( "Cutoff frequency for lowpass filter on output of chebyshev waveshapers" );
    outputLPFLabel.attachToComponent( &outputLPFSlider, false);
    outputLPFLabel.setText( "Out LPF", juce::dontSendNotification );
    outputLPFLabel.setJustificationType( juce::Justification::centred );
    
    
    addAndMakeVisible(&tooltipsToggle);
    tooltipsToggle.setButtonText("Hints");
    tooltipsToggle.onStateChange = [this]
    {
        if (tooltipsToggle.getToggleState())
        {
            tooltipWindow.getObject().setAlpha(1.0f);
        }
        else
        {
            tooltipWindow.getObject().setAlpha(0.0f);
        }
    };
    tooltipWindow.getObject().setAlpha(0.0f);
    
//    startTimer(100);
    setSize (600, 300);
}

Sjf_chebyShevAudioProcessorEditor::~Sjf_chebyShevAudioProcessorEditor()
{
    setLookAndFeel( nullptr  );
}

//==============================================================================
void Sjf_chebyShevAudioProcessorEditor::paint (juce::Graphics& g)
{
    auto outlineColour = getLookAndFeel().findColour( mixSlider.textBoxOutlineColourId );
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));

    g.setColour (juce::Colours::white);
    g.setFont (15.0f);
    g.drawFittedText("sjf_chebyShev", 0, 0, getWidth(), textHeight, juce::Justification::centred, 1);
    
    g.setColour( inputParamsBackgroundColour );
    g.fillRect( inputDriveLabel.getX(), inputDriveLabel.getY(), inputDriveLabel.getWidth(), inputLPFSlider.getBottom() - inputDriveLabel.getY() );
    g.setColour( outlineColour );
    g.drawRect( inputDriveLabel.getX(), inputDriveLabel.getY(), inputDriveLabel.getWidth(), inputLPFSlider.getBottom() - inputDriveLabel.getY() );
    
    g.setColour( outputParamsBackgroundColour );
    g.fillRect( outputHPFLabel.getX(), outputHPFLabel.getY(), outputHPFLabel.getWidth(), outputLPFSlider.getBottom() - outputHPFLabel.getY() );
    g.setColour( outlineColour );
    g.drawRect( outputHPFLabel.getX(), outputHPFLabel.getY(), outputHPFLabel.getWidth(), outputLPFSlider.getBottom() - outputHPFLabel.getY() );
    
    g.setColour( mixBackgroundColour );
    g.fillRect( mixLabel.getX(), mixLabel.getY(), mixLabel.getWidth(), mixSlider.getBottom() - mixLabel.getY() );
    g.setColour( outlineColour );
    g.drawRect( mixLabel.getX(), mixLabel.getY(), mixLabel.getWidth(), mixSlider.getBottom() - mixLabel.getY() );
}

void Sjf_chebyShevAudioProcessorEditor::resized()
{
    chebyMultiSlider.setBounds( textHeight * 0.5f, textHeight + textHeight*0.5f, getWidth() * 0.5, getHeight() - textHeight*2.0f );
    
    inputDriveSlider.setBounds( chebyMultiSlider.getRight() + textHeight*0.5f, chebyMultiSlider.getY() + inputDriveLabel.getHeight(), potSize, potSize );
    inputLPFSlider.setBounds(inputDriveSlider.getX(), inputDriveSlider.getBottom() + textHeight + textHeight*0.5f, potSize, potSize );
    
    outputHPFSlider.setBounds( inputDriveSlider.getRight() + textHeight*0.5f, inputDriveSlider.getY(), potSize, potSize );
    outputLPFSlider.setBounds( outputHPFSlider.getX(), outputHPFSlider.getBottom() + textHeight + textHeight*0.5f, potSize, potSize );
    
    mixSlider.setBounds( outputLPFSlider.getRight() + textHeight*0.5f, outputHPFSlider.getY(), potSize, potSize );
    
    tooltipsToggle.setBounds(getWidth() - potSize, getHeight() - textHeight, potSize, textHeight );
}

//
//void Sjf_chebyShevAudioProcessorEditor::timerCallback()
//{
//
//}
