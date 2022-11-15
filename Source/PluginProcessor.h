/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "/Users/simonfay/Programming_Stuff/sjf_audio/sjf_chebyshevPolys.h"
#include "/Users/simonfay/Programming_Stuff/sjf_audio/sjf_lpf.h"
#include "/Users/simonfay/Programming_Stuff/sjf_audio/sjf_lpfFirst.h"
//==============================================================================
/**
*/
class Sjf_chebyShevAudioProcessor  : public juce::AudioProcessor
                            #if JucePlugin_Enable_ARA
                             , public juce::AudioProcessorARAExtension
                            #endif
{
public:
    //==============================================================================
    Sjf_chebyShevAudioProcessor();
    ~Sjf_chebyShevAudioProcessor() override;

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

   #ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
   #endif

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    //==============================================================================
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    //==============================================================================
    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    //==============================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;

    //==============================================================================
    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    
    void setLevels( int index, float newValue )
    {
        if ( index < m_levels.size() )
        {
            m_levels[ index ] = newValue;
        }
    }
    
    float getLevel( int index )
    {
        index = std::min( index, m_cheby.getNumOrders() );
        index = std::max( index, 0 );
        return m_levels[ index ];
    }
    
    int getNumChebyOrders()
    {
        return m_cheby.getNumOrders();
    }
    
    void setSmoothedValueTargets();
private:
    juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();
    void initialiseFilters();
    void initialiseSmoothedValues();
    float calculateFilterAlpha( float cutOffFrequency );
    void setLPFilterOrder();
    //==============================================================================
    sjf_chebyshev m_cheby;
    std::array< std::array< sjf_lpf, 2 >, 2 > m_outHPF;
    std::array< sjf_lpf, 2 > m_inLPF, m_outLPF;
    std::vector< float > m_levels;
    std::vector< juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> > m_smoothedChebyLevels;
    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> m_mixSmoothed, m_inputDriveSmoothed, m_inLPFSmoothed, m_outHPFSmoothed, m_outLPFSmoothed;
    float m_SR = 44100;
    bool m_firstOrderLowPass = true;
    
    juce::AudioProcessorValueTreeState parameters;
    
    std::atomic<float>* mixParameter = nullptr;
    std::atomic<float>* inDriveParameter = nullptr;
    std::atomic<float>* inLPFParameter = nullptr;
    std::atomic<float>* outHPFParameter = nullptr;
    std::atomic<float>* outLPFParameter = nullptr;
    std::atomic<float>* firstOrderLPParameter = nullptr;
    std::vector< std::atomic<float>* > chebyParameters;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Sjf_chebyShevAudioProcessor)
};
