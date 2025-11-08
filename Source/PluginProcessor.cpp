/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
BeastDistortionAudioProcessor::BeastDistortionAudioProcessor()
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
    //Инициализация параметра Drive
    addParameter(driveParam = new juce::AudioParameterFloat(
        "drive",  // ID параметра
        "Drive",  // Название параметра
        juce::NormalisableRange<float>(0.f, 100.0f),  // Диапазон значений
        50.0f, // Значение по умолчанию
        "Drive", // Единица измерения
        juce::AudioProcessorParameter::genericParameter, // Тип параметра
        [](float value, int) {return juce::String(value, 1); } // Форматирование значение
    ));

    // Инициализация параметра Gain (Входной уровень)
    addParameter(gainParam = new juce::AudioParameterFloat(
        "gain",
        "Gain",
        juce::NormalisableRange<float>(0.0f, 100.0f),
        50.0f,
        "Gain",
        juce::AudioProcessorParameter::genericParameter,
        [](float value, int) { return juce::String(value, 1); }
    ));

    // Инициализация параметра Output (Выходной уровень)
    addParameter(outputParam = new juce::AudioParameterFloat(
        "output",
        "Output",
        juce::NormalisableRange<float>(0.0f, 100.0f),
        50.0f,
        "Output",
        juce::AudioProcessorParameter::genericParameter,
        [](float value, int) { return juce::String(value, 1); }
    ));

    // Список типов дисторшна
    juce::StringArray distortionTypes;
    distortionTypes.add("Hard Clip");
    distortionTypes.add("Soft Clip");
    distortionTypes.add("Overdrive");
    distortionTypes.add("Foldback");

    // Инициализация параметра выбора типа дисторшна
    addParameter(typeParam = new juce::AudioParameterChoice(
        "type",
        "Type",
        distortionTypes,
        0  // Индекс выбранного по умолчанию (Hard Clip)
    ));

    // Инициализация параметра Bypass
    addParameter(bypassParam = new juce::AudioParameterBool(
        "bypass",
        "Bypass",
        false  // По умолчанию выключен
    ));

}

BeastDistortionAudioProcessor::~BeastDistortionAudioProcessor()
{
}

//==============================================================================
// Обработка одного сэмпла
float BeastDistortionAudioProcessor::processSample(float input, int channel)
{
    // Если bypass включен - пропускаем сигнал без изменений
    if (bypassParam->get())
        return input;

    // Получаем текущие значения параметров
    float gain = gainParam->get();
    float drive = driveParam->get();
    float output = outputParam->get();
    int distortionType = typeParam->getIndex();

    // Преобразуем значения параметров в рабочие диапазоны
    float gainFactor = 1.0f + (gain / 100.0f) * 4.0f;   // 1.0x - 5.0x усиление входа
    float driveGain = 1.0f + (drive / 100.0f) * 9.0f;   // 1.0x - 10.0x усиление дисторшна
    float outputGain = output / 100.0f * 2.0f;          // 0.0 - 2.0 выходное усиление

    //  Применяем входное усиление (Gain)
    float processed = input * gainFactor;

    // Применяем усиление дисторшна (Drive)
    processed *= driveGain;

    // Применяем выбранный тип дисторшна
    switch (distortionType)
    {
    case 0: // Hard Clipping
        processed = juce::jlimit(-1.0f, 1.0f, processed);
        break;

    case 1: // Soft Clipping
        processed = std::tanh(processed);
        break;

    case 2: // Overdrive
        if (processed > 1.0f)
            processed = 1.0f - std::exp(-processed);
        else if (processed < -1.0f)
            processed = -1.0f + std::exp(processed);
        break;

    case 3: // Foldback
        if (processed > 1.0f)
            processed = 1.0f - (processed - 1.0f);
        else if (processed < -1.0f)
            processed = -1.0f - (processed + 1.0f);
        break;

    default:
        processed = juce::jlimit(-1.0f, 1.0f, processed);
        break;
    }

    // Применяем выходное усиление
    return processed * outputGain;
}

//==============================================================================
const juce::String BeastDistortionAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool BeastDistortionAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool BeastDistortionAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool BeastDistortionAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double BeastDistortionAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int BeastDistortionAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int BeastDistortionAudioProcessor::getCurrentProgram()
{
    return 0;
}

void BeastDistortionAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String BeastDistortionAudioProcessor::getProgramName (int index)
{
    return {};
}

void BeastDistortionAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void BeastDistortionAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    // Use this method as the place to do any pre-playback
    // initialisation that you need..
}

void BeastDistortionAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool BeastDistortionAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
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

void BeastDistortionAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    // In case we have more outputs than inputs, this code clears any output
    // channels that didn't contain input data, (because these aren't
    // guaranteed to be empty - they may contain garbage).
    // This is here to avoid people getting screaming feedback
    // when they first compile a plugin, but obviously you don't need to keep
    // this code if your algorithm always overwrites all the output channels.
    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());

    // This is the place where you'd normally do the guts of your plugin's
    // audio processing...
    // Make sure to reset the state if your inner loop is processing
    // the samples and the outer loop is handling the channels.
    // Alternatively, you can process the samples with the channels
    // interleaved by keeping the same state.
    for (int channel = 0; channel < totalNumInputChannels; ++channel)
    {
        auto* channelData = buffer.getWritePointer (channel);

        // Обрабатываем каждый сэмпл в канале
        for (int sample = 0; sample < buffer.getNumSamples(); ++sample)
        {
            channelData[sample] = processSample(channelData[sample], channel);
        }
    }
}

//==============================================================================
bool BeastDistortionAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* BeastDistortionAudioProcessor::createEditor()
{
    return new BeastDistortionAudioProcessorEditor (*this);
}

//==============================================================================
void BeastDistortionAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
}

void BeastDistortionAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new BeastDistortionAudioProcessor();
}
