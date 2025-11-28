/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

//==============================================================================
// Аудиопроцессор плагина BeastDistortion.
// Отвечает за всю аудиообработку, управление параметрами и сохранение состояния.
// Реализует различные алгоритмы дисторшна и управление сигналом.

class BeastDistortionAudioProcessor  : public juce::AudioProcessor
{
public:
    //==============================================================================
    // Конструктор - инициализирует все параметры плагина
    BeastDistortionAudioProcessor();

    // Деструктор
    ~BeastDistortionAudioProcessor() override;

    //==============================================================================
    // Геттеры для прямого доступа к значениям параметров
    float getGain() const { return gainParam->get(); }                 // Текущее значение Gain (0-100)
    float getDrive() const { return driveParam->get(); }               // Текущее значение Drive (0-100)
    float getOutput() const { return outputParam->get(); }             // Текущее значение Output (0-100)
    int getDistortionType() const { return typeParam->getIndex(); }    // Текущий тип дисторшна (0-3)
    bool getBypass() const { return bypassParam->get(); }              // Текущее состояние ON/OFF

    // Публичные методы для доступа к объектам параметров (используются редактором)
    juce::AudioParameterFloat* getGainParam() const { return gainParam; }
    juce::AudioParameterFloat* getDriveParam() const { return driveParam; }
    juce::AudioParameterFloat* getOutputParam() const { return outputParam; }
    juce::AudioParameterChoice* getTypeParam() const { return typeParam; }
    juce::AudioParameterBool* getBypassParam() const { return bypassParam; }

    //==============================================================================
    // Методы аудиообработки JUCE
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

   #ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
   #endif

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    //==============================================================================
    // Методы редактора

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    //==============================================================================
    // Информационные методы

    const juce::String getName() const override;
    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    //==============================================================================
    // Методы программ (пресетов)
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;

    //==============================================================================
    // Методы сохранения и восстановления состояния
    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

private:
    //==============================================================================
    // Параметры плагина
    juce::AudioParameterFloat* gainParam;       // Входной уровень сигнала (0-100)
    juce::AudioParameterFloat* driveParam;      // Уровень обработки дисторшна (0-100)
    juce::AudioParameterFloat* outputParam;     // Выходной уровень сигнала (0-100)
    juce::AudioParameterChoice* typeParam;      // Тип обработки дисторшна
    juce::AudioParameterBool* bypassParam;      // вкл/выкл обработку

    //функция обработки одного аудиосэмпла через выбранный алгоритм дисторшна
    float processSample(float input, int channel);

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (BeastDistortionAudioProcessor)
};
