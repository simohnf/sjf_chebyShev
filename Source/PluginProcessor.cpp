/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
Sjf_chebyShevAudioProcessor::Sjf_chebyShevAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       )
#endif
, parameters( *this, nullptr, juce::Identifier("sjf_verb"), createParameterLayout() )
{
    
    for ( int i = 0; i < m_cheby.getNumOrders(); i++ )
    {
        m_levels.push_back( 0 );
        m_smoothedChebyLevels.push_back( *new juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> );
        
        chebyParameters.push_back( nullptr );
        std::string name = "cheby" + std::to_string( i + 2 ) ;
        chebyParameters[ i ] = parameters.getRawParameterValue( name );
    }
    
    mixParameter = parameters.getRawParameterValue("mix");
    inDriveParameter = parameters.getRawParameterValue("inputDrive");
    
    inLPFParameter = parameters.getRawParameterValue( "inLPF" );
    outHPFParameter = parameters.getRawParameterValue( "outHPF" );
    outLPFParameter = parameters.getRawParameterValue( "outLPF" );
    
}

Sjf_chebyShevAudioProcessor::~Sjf_chebyShevAudioProcessor()
{
}

//==============================================================================
const juce::String Sjf_chebyShevAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool Sjf_chebyShevAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool Sjf_chebyShevAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool Sjf_chebyShevAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double Sjf_chebyShevAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int Sjf_chebyShevAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int Sjf_chebyShevAudioProcessor::getCurrentProgram()
{
    return 0;
}

void Sjf_chebyShevAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String Sjf_chebyShevAudioProcessor::getProgramName (int index)
{
    return {};
}

void Sjf_chebyShevAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void Sjf_chebyShevAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    m_SR = sampleRate;
    // set cutoff of filters to 20Hz
    auto Fc = 20.0f;
    auto alpha = calculateFilterAlpha( Fc );
    for ( int c = 0; c < m_outHPF.size(); c++ )
    {
        for ( int i = 0; i < m_outHPF[ c ].size(); i++ )
        {
            m_outHPF[ c ][ i ].setCutoff( alpha );
        }
    }
    initialiseSmoothedValues();
    setSmoothedValueTargets();
}

void Sjf_chebyShevAudioProcessor::releaseResources()
{
    
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool Sjf_chebyShevAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
  #else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    // Some plugin hosts, such as certain GarageBand versions, will only
    // load plugins that support stereo bus layouts.
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    // This checks if the input layout matches the output layout
   #if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
   #endif

    return true;
  #endif
}
#endif

void Sjf_chebyShevAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());

    setSmoothedValueTargets();
    auto bufferSize = buffer.getNumSamples();
    float distorted, val, inVal, dryAmp, wetAmp, inLPFAlpha, outHPFAlpha, outLPFAlpha, inDrive;

    auto numOrders = m_cheby.getNumOrders();
    for ( int i = 0; i < bufferSize; i++ )
    {
        inDrive = m_inputDriveSmoothed.getNextValue();
        inLPFAlpha = m_inLPFSmoothed.getNextValue();
        outHPFAlpha = m_outHPFSmoothed.getNextValue();
        outLPFAlpha = m_outLPFSmoothed.getNextValue();
        wetAmp = m_mixSmoothed.getNextValue();
        dryAmp = 1.0f - wetAmp;
        wetAmp *= wetAmp;
        dryAmp *= dryAmp;
        for ( int c = 0; c < totalNumOutputChannels; c ++ )
        {
            distorted = 0;
            // copy input sample
            val = buffer.getSample( c % totalNumInputChannels, i );
            inVal = val * inDrive;
            m_inLPF[ c ].setCutoff( inLPFAlpha );
            inVal = m_inLPF[ c ].filterInput( inVal );
            //distort through each orde of chebyshev polynomial
            for ( int i = 0; i < numOrders; i++ )
            {
                distorted += m_cheby.process( i, inVal ) * m_smoothedChebyLevels[ i ].getNextValue();
            }
            // filter out dc
            for ( int i = 0; i < m_outHPF[ c ].size(); i++ )
            {
                m_outHPF[ c ][ i ].setCutoff( outHPFAlpha );
                distorted -= m_outHPF[ c ][ i ].filterInput( distorted );
            }
            m_outLPF[ c ].setCutoff( outHPFAlpha );
            distorted = m_outLPF[ c ].filterInput( distorted );

            
            distorted *= wetAmp;
            
            // calculate dry signal
            val *= dryAmp;
            
            buffer.setSample( c , i , distorted + val );
        }
    }
}

//==============================================================================
bool Sjf_chebyShevAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* Sjf_chebyShevAudioProcessor::createEditor()
{
    return new Sjf_chebyShevAudioProcessorEditor (*this, parameters);
}

//==============================================================================
void Sjf_chebyShevAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
}

void Sjf_chebyShevAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
}

void Sjf_chebyShevAudioProcessor::setSmoothedValueTargets()
{
//    DBG("nOrders " << );
    for ( int i = 0; i < m_cheby.getNumOrders(); i++ )
    {
        m_smoothedChebyLevels[ i ].setTargetValue(  *chebyParameters[ i ] );
    }
    m_mixSmoothed.setTargetValue( *mixParameter * 0.01f );
    m_inputDriveSmoothed.setTargetValue( *inDriveParameter * 0.01f );
    
    m_inLPFSmoothed.setTargetValue( calculateFilterAlpha( *inLPFParameter ) );
    m_outHPFSmoothed.setTargetValue( calculateFilterAlpha( *outHPFParameter ) );
    m_outLPFSmoothed.setTargetValue( calculateFilterAlpha( *outLPFParameter ) );
}

void Sjf_chebyShevAudioProcessor::initialiseFilters()
{
//    m_inHPF, m_inLPF, m_outHPF, m_outLPF;
}

void Sjf_chebyShevAudioProcessor::initialiseSmoothedValues()
{
    for ( int i = 0; i < m_cheby.getNumOrders() + 1; i++ )
    {
        m_smoothedChebyLevels[ i ].reset( m_SR, 0.1f );
    }
    m_mixSmoothed.reset( m_SR, 0.1f );
    m_inputDriveSmoothed.reset( m_SR, 0.1f );
    m_inLPFSmoothed.reset( m_SR, 0.1f );
    m_outHPFSmoothed.reset( m_SR, 0.1f );
    m_outLPFSmoothed.reset( m_SR, 0.1f );
}

juce::AudioProcessorValueTreeState::ParameterLayout Sjf_chebyShevAudioProcessor::createParameterLayout()
{
    juce::AudioProcessorValueTreeState::ParameterLayout params;
    params.add ( std::make_unique<juce::AudioParameterFloat> ("mix", "Mix", 0.0f, 100.0f, 100.0f) );
    params.add ( std::make_unique<juce::AudioParameterFloat> ("inputDrive", "InputDrive", 0.0001f, 100.0f, 100.0f) );
    
    params.add ( std::make_unique<juce::AudioParameterFloat> ("inLPF", "InLPF", 20.0f, 20000.0f, 1000.0f) );
    params.add ( std::make_unique<juce::AudioParameterFloat> ("outHPF", "OutHPF", 20.0f, 20000.0f, 20.0f) );
    params.add ( std::make_unique<juce::AudioParameterFloat> ("outLPF", "OutLPF", 20.0f, 20000.0f, 2000.0f) );
    
    for ( int i = 0; i < m_cheby.getNumOrders(); i++ )
    {
        std::string name = "cheby" + std::to_string( i + 2 ) ;
        std::string Name = "Cheby" + std::to_string( i + 2 ) ;
        params.add (std::make_unique<juce::AudioParameterFloat> ( name, Name, 0.0f, 1.0f, 0.0f) );
    }
    
    return params;
}

float Sjf_chebyShevAudioProcessor::calculateFilterAlpha( float cutOffFrequency )
{
    return sin( cutOffFrequency * 2 * 3.14159265359 / m_SR );
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new Sjf_chebyShevAudioProcessor();
}


