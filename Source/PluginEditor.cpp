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
    
    addAndMakeVisible( &inputLabel );
    inputLabel.setText( "INPUT", juce::dontSendNotification );
    inputLabel.setJustificationType( juce::Justification::centred );
    
    
    inputDriveSliderAttachment.reset (new juce::AudioProcessorValueTreeState::SliderAttachment (valueTreeState, "inputDrive", inputDriveSlider));
    addAndMakeVisible( &inputDriveSlider );
    inputDriveSlider.setSliderStyle (juce::Slider::Rotary);
    inputDriveSlider.setTextBoxStyle (juce::Slider::TextBoxBelow, false, potSize, textHeight);
    inputDriveSlider.setNumDecimalPlacesToDisplay(3);
    inputDriveSlider.setTextValueSuffix ("%");
    inputDriveSlider.setTooltip( "Level of input to the chebyshev waveshapers" );
    inputDriveLabel.attachToComponent( &inputDriveSlider, false );
    inputDriveLabel.setText( "Drive", juce::dontSendNotification);
    inputDriveLabel.setJustificationType( juce::Justification::centred );
    
    inputLPFSliderAttachment.reset (new juce::AudioProcessorValueTreeState::SliderAttachment (valueTreeState, "inLPF", inputLPFSlider));
    addAndMakeVisible( &inputLPFSlider );
    inputLPFSlider.setRange( 20.0f, 10000.0f );
    inputLPFSlider.setSliderStyle (juce::Slider::Rotary);
    inputLPFSlider.setTextBoxStyle (juce::Slider::TextBoxBelow, false, potSize, textHeight);
    inputLPFSlider.setNumDecimalPlacesToDisplay(3);
    inputLPFSlider.setTextValueSuffix ("Hz");
    inputLPFSlider.setSkewFactorFromMidPoint( 500.0f );
    inputLPFSlider.setTooltip( "Cutoff frequency for lowpass filter on input to chebyshev waveshapers" );
    inputLPFLabel.attachToComponent( &inputLPFSlider, false );
    inputLPFLabel.setText( "LPF", juce::dontSendNotification );
    inputLPFLabel.setJustificationType( juce::Justification::centred );
    
    addAndMakeVisible( &outputLabel );
    outputLabel.setText( "OUTPUT", juce::dontSendNotification );
    outputLabel.setJustificationType( juce::Justification::centred );
    
    outputHPFSliderAttachment.reset (new juce::AudioProcessorValueTreeState::SliderAttachment (valueTreeState, "outHPF", outputHPFSlider));
    addAndMakeVisible( &outputHPFSlider );
    outputHPFSlider.setRange( 20.0f, 10000.0f );
    outputHPFSlider.setSliderStyle (juce::Slider::Rotary);
    outputHPFSlider.setTextBoxStyle (juce::Slider::TextBoxBelow, false, potSize, textHeight);
    outputHPFSlider.setNumDecimalPlacesToDisplay(3);
    outputHPFSlider.setTextValueSuffix ("Hz");
    outputHPFSlider.setSkewFactorFromMidPoint( 500.0f );
    outputHPFSlider.setTooltip( "Cutoff frequency for highpass filter on output of chebyshev waveshapers" );
    outputHPFLabel.attachToComponent( &outputHPFSlider, false );
    outputHPFLabel.setText( "HPF", juce::dontSendNotification );
    outputHPFLabel.setJustificationType( juce::Justification::centred );
    
    outputLPFSliderAttachment.reset (new juce::AudioProcessorValueTreeState::SliderAttachment (valueTreeState, "outLPF", outputLPFSlider));
    addAndMakeVisible( &outputLPFSlider ); 
    outputLPFSlider.setRange( 20.0f, 10000.0f );
    outputLPFSlider.setSliderStyle (juce::Slider::Rotary);
    outputLPFSlider.setTextBoxStyle (juce::Slider::TextBoxBelow, false, potSize, textHeight);
    outputLPFSlider.setNumDecimalPlacesToDisplay(3);
    outputLPFSlider.setTextValueSuffix ("Hz");
    outputLPFSlider.setSkewFactorFromMidPoint( 500.0f );
    outputLPFSlider.setTooltip( "Cutoff frequency for lowpass filter on output of chebyshev waveshapers" );
    outputLPFLabel.attachToComponent( &outputLPFSlider, false);
    outputLPFLabel.setText( "LPF", juce::dontSendNotification );
    outputLPFLabel.setJustificationType( juce::Justification::centred );
    
    
    addAndMakeVisible( &firstOrderLPFToggle );
    firstOrderLPFToggleAttachment.reset( new juce::AudioProcessorValueTreeState::ButtonAttachment(valueTreeState, "firstOrderLPF", firstOrderLPFToggle));
    firstOrderLPFToggle.setTooltip("Order for low pass filters");
    firstOrderLPFToggle.onClick = [ this ]
    {
        if ( firstOrderLPFToggle.getToggleState() )
        {
            firstOrderLPFToggle.setButtonText("1st order");
        }
        else
        {
            firstOrderLPFToggle.setButtonText("2nd order");
        }
    };
    if ( audioProcessor.lpfIsFirstOrder() )
    {
//        firstOrderLPFToggle.setToggleState( true, juce::dontSendNotification );
        firstOrderLPFToggle.setButtonText("1st order");
    }
    else
    {
        firstOrderLPFToggle.setButtonText("2nd order");
    }
    
    
    addAndMakeVisible(&tooltipsToggle);
    tooltipsToggle.setButtonText("Hints");
    tooltipsToggle.setColour(tooltipsToggle.tickColourId, juce::Colours::white.withAlpha(1.0f));
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
    
    auto w = potSize * 6 + textHeight * 2.5;
    auto h = potSize * 2 + textHeight * 5.5 - 2;
    setSize (w, h);
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
    g.fillRect( inputLabel.getX(), inputLabel.getY(), inputLabel.getWidth(), inputLPFSlider.getBottom() - inputLabel.getY() );
    g.setColour( outlineColour );
    g.drawRect( inputLabel.getX(), inputLabel.getY(), inputLabel.getWidth(), inputLPFSlider.getBottom() - inputLabel.getY() );
    g.drawLine( inputLabel.getX(), inputLabel.getBottom(), inputLabel.getRight(), inputLabel.getBottom() );
    
    g.setColour( outputParamsBackgroundColour );
    g.fillRect( outputLabel.getX(), outputLabel.getY(), outputLabel.getWidth(), outputLPFSlider.getBottom() - outputLabel.getY() );
    g.setColour( outlineColour );
    g.drawRect( outputLabel.getX(), outputLabel.getY(), outputLabel.getWidth(), outputLPFSlider.getBottom() - outputLabel.getY() );
    g.drawLine( outputLabel.getX(), outputLabel.getBottom(), outputLabel.getRight(), outputLabel.getBottom() );
    
    g.setColour( mixBackgroundColour );
    g.fillRect( mixLabel.getX(), mixLabel.getY(), mixLabel.getWidth(), mixSlider.getBottom() - mixLabel.getY() );
    g.setColour( outlineColour );
    g.drawRect( mixLabel.getX(), mixLabel.getY(), mixLabel.getWidth(), mixSlider.getBottom() - mixLabel.getY() );
    
}

void Sjf_chebyShevAudioProcessorEditor::resized()
{
    chebyMultiSlider.setBounds( textHeight * 0.5f, textHeight + textHeight*0.5f, potSize * 3, getHeight() - textHeight*2.0f );

    inputLabel.setBounds( chebyMultiSlider.getRight() + textHeight*0.5f, chebyMultiSlider.getY(), potSize, textHeight );
    inputDriveSlider.setBounds( inputLabel.getX(), inputLabel.getBottom() + inputDriveLabel.getHeight(), potSize, potSize );
    inputLPFSlider.setBounds(inputDriveSlider.getX(), inputDriveSlider.getBottom() + inputDriveLabel.getHeight(), potSize, potSize );
    
    outputLabel.setBounds( inputLabel.getRight() + textHeight*0.5f, inputLabel.getY(), potSize, textHeight );
    outputHPFSlider.setBounds( outputLabel.getX(), outputLabel.getBottom() + outputHPFLabel.getHeight(), potSize, potSize );
    outputLPFSlider.setBounds( outputHPFSlider.getX(), outputHPFSlider.getBottom() + outputHPFLabel.getHeight(), potSize, potSize );
    
    auto mixTop = outputLabel.getY() + ( outputLPFSlider.getBottom() - outputLabel.getY() )*0.5f - ( mixLabel.getHeight() + potSize ) * 0.5f;
    
    mixSlider.setBounds( outputLabel.getRight() + textHeight*0.5f, mixTop, potSize, potSize );
    firstOrderLPFToggle.setBounds( mixSlider.getX(), mixSlider.getBottom() + textHeight*0.5f, potSize, textHeight );
    
    tooltipsToggle.setBounds(mixSlider.getX(), chebyMultiSlider.getBottom() - textHeight, potSize, textHeight );
}

