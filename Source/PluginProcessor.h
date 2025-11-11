/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

//==============================================================================
/**
*/
class BeastDistortionAudioProcessor  : public juce::AudioProcessor
{
public:
    //==============================================================================
    BeastDistortionAudioProcessor();
    ~BeastDistortionAudioProcessor() override;

    // Геттеры для параметров плагина
    float getGain() const { return gainParam->get(); }
    float getDrive() const { return driveParam->get(); };
    float getOutput() const { return outputParam->get(); };
    int getDistortionType() const { return typeParam->getIndex(); };
    bool getBypass() const { return bypassParam->get(); };

    // Публичные методы для доступа к параметрам
    juce::AudioParameterFloat* getGainParam() const { return gainParam; }
    juce::AudioParameterFloat* getDriveParam() const { return driveParam; }
    juce::AudioParameterFloat* getOutputParam() const { return outputParam; }
    juce::AudioParameterChoice* getTypeParam() const { return typeParam; }
    juce::AudioParameterBool* getBypassParam() const { return bypassParam; }

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

private:
    //==============================================================================
    // Параметры плагина
    juce::AudioParameterFloat* gainParam;  // Входной уровень сигнала (0-100)
    juce::AudioParameterFloat* driveParam; // Уровень обработки дисторшна (0-100)
    juce::AudioParameterFloat* outputParam; // Выходной уровень сигнала (0-100)
    juce::AudioParameterChoice* typeParam; // Тип обработки дисторшна
    juce::AudioParameterBool* bypassParam; // вкл/выкл обработку

    //функция обработки одного сэмпла
    float processSample(float input, int channel);

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (BeastDistortionAudioProcessor)
};
