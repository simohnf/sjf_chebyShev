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
{
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
    auto alpha = sin(Fc * 2 * 3.14159265359 / m_SR);
    m_hpf.setCutoff( alpha );
}

void Sjf_chebyShevAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
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

    auto bufferSize = buffer.getNumSamples();
    float distorted, inVal;
//    for (int i = 0; i < m_levels.size(); i++ )
//    {
//        DBG( "levels " << i << m_levels[i] );
//    }
//    DBG("");
    for ( int i = 0; i < bufferSize; i++ )
    {
        for ( int c = 0; c < totalNumOutputChannels; c ++ )
        {
            distorted = 0;
            // copy input sample
            inVal = buffer.getSample( c % totalNumInputChannels, i );
            //distort through each orde of chebyshev polynomial
            distorted += m_cheby.process( 0, inVal );
            
            // filter out dc
            distorted = distorted - m_hpf.filterInput( distorted );
            buffer.setSample( c , i , distorted );
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
    return new Sjf_chebyShevAudioProcessorEditor (*this);
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

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new Sjf_chebyShevAudioProcessor();
}
