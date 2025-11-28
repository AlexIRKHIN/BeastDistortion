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
    //Инициализация параметра Drive (интенсивность дисторшна)
    addParameter(driveParam = new juce::AudioParameterFloat(
        "drive",                                                // Идентификатор параметра
        "Drive",                                                // Название параметра (для DAW)
        juce::NormalisableRange<float>(0.f, 100.0f),            // Диапазон значений (0-100)
        50.0f,                                                  // Значение по умолчанию (50%)
        "Drive",                                                // Единица измерения (отображаемая)
        juce::AudioProcessorParameter::genericParameter,        // Тип параметра
        [](float value, int) {return juce::String(value, 1); }  // Форматирование значение (1 знак после запятой)
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

    // Создание списка доступных типов дисторшна
    juce::StringArray distortionTypes;
    distortionTypes.add("Hard Clip");       // Жёсткое ограничение
    distortionTypes.add("Soft Clip");       // Мягкое ограничение
    distortionTypes.add("Overdrive");       // Овердрайв
    distortionTypes.add("Foldback");        // Фолдбэк

    // Инициализация параметра выбора типа дисторшна
    addParameter(typeParam = new juce::AudioParameterChoice(
        "type",                 // Идентификатор
        "Type",                 // Название
        distortionTypes,        // Список вариантов
        0                       // Индекс по умолчанию (Hard Clip)
    ));

    // Инициализация параметра Bypass (включение/выключение эффекта)
    addParameter(bypassParam = new juce::AudioParameterBool(
        "bypass",               // Идентификатор
        "Bypass",               // Название
        false                   // Значение по умолчанию (выключен)
    ));

}

BeastDistortionAudioProcessor::~BeastDistortionAudioProcessor()
{
    // Деструктор - автоматическое освобождение ресурсов через умные указатели JUCE
}

//==============================================================================
float BeastDistortionAudioProcessor::processSample(float input, int channel)
{
    // Если bypass включен - пропускаем сигнал без изменений
    if (bypassParam->get())
        return input;

    // Получение текущих значений параметров
    float gain = gainParam->get();
    float drive = driveParam->get();
    float output = outputParam->get();
    int distortionType = typeParam->getIndex();

    // Преобразование параметров в коэффициенты усиления:

    // Gain: преобразуем 0-100 в диапазон 0.5x - 2.0x
    // При 50 = 1.25x (нейтральное положение)
    float gainFactor = 0.5f + (gain / 100.0f) * 1.5f;

    // Drive: преобразуем 0-100 в диапазон 1.0x - 6.0x  
    // При 50 = 3.5x (умеренный эффект дистошна)
    float driveGain = 1.0f + (drive / 100.0f) * 5.0f;

    // Output: преобразуем 0-100 в диапазон 0.25x - 1.5x
    // При 50 = 0.6x (компенсация усиления от предыдущих обработок)
    float outputGain = 0.2f + (output / 100.0f) * 0.8f;

    // Применение входного усиления
    float processed = input * gainFactor;

    // Применение усиления дисторшна
    processed *= driveGain;

    // Применение выбранного алгоритма дисторшна
    switch (distortionType)
    {
    case 0:     // Hard Clipping - жесткое ограничение
        processed = juce::jlimit(-1.0f, 1.0f, processed);
        break;

    case 1:     // Soft Clipping - мягкое ограничение через гиперболический тангенс
        processed = std::tanh(processed);
        break;

    case 2:     // Overdrive - асимметричное ограничение с экспоненциальным затуханием
        if (processed > 1.0f)
            processed = 1.0f - std::exp(-processed);
        else if (processed < -1.0f)
            processed = -1.0f + std::exp(processed);
        break;

    case 3:     // Foldback - "отражение" сигнала при превышении порога
        if (processed > 1.0f)
            processed = 1.0f - (processed - 1.0f);
        else if (processed < -1.0f)
            processed = -1.0f - (processed + 1.0f);
        break;

    default:    // Резервный вариант - жесткое ограничение
        processed = juce::jlimit(-1.0f, 1.0f, processed);
        break;
    }

    // Применение выходного усиления и возврат обработанного сэмпла
    return processed * outputGain;
}

//==============================================================================
const juce::String BeastDistortionAudioProcessor::getName() const
{
    return JucePlugin_Name;     // Возвращает имя плагина, заданное в настройках Projucer
}

bool BeastDistortionAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;                // Плагин принимает MIDI-вход
   #else
    return false;               // Плагин не принимает MIDI (чисто аудио)
   #endif
}

bool BeastDistortionAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;                // Плагин генерирует MIDI-выход
   #else
    return false;               // Плагин не генерирует MIDI
   #endif
}

bool BeastDistortionAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;                // Плагин является MIDI-эффектом
   #else
    return false;               // Плагин является аудио-эффектом
   #endif
}

double BeastDistortionAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;                 // Нет "хвоста" (эффект мгновенный)
}

int BeastDistortionAudioProcessor::getNumPrograms()
{
    return 1;                   // Поддержка одной программы - минимальное требование для хостов
}

int BeastDistortionAudioProcessor::getCurrentProgram()
{
    return 0;                   // Всегда возвращает 0 (только одна программа)
}

void BeastDistortionAudioProcessor::setCurrentProgram (int index)
{
    // Пустая реализация - одна программа, нечего менять
}

const juce::String BeastDistortionAudioProcessor::getProgramName (int index)
{
    return {};                   // Пустое имя программы
}

void BeastDistortionAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
    // Пустая реализация - программы не поддерживают переименование
}

//==============================================================================
void BeastDistortionAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    // Метод для подготовки к воспроизведению
    // В данном плагине не требует дополнительной инициализации
}

void BeastDistortionAudioProcessor::releaseResources()
{
    // Метод освобождения ресурсов
    // В данном плагине не требует дополнительной очистки
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool BeastDistortionAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
  #else
    // Проверка поддерживаемых конфигураций аудиоканалов
    // Поддерживаются только моно и стерео конфигурации
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    // Проверка соответствия входных и выходных конфигураций
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
    juce::ScopedNoDenormals noDenormals;    // Отключение денормализованных чисел для производительности
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    // Очистка выходных каналов, которые не имеют входных данных
    // (предотвращение воспроизведения "мусора" в неиспользуемых каналах)
    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());

    // Основной цикл обработки аудио
    // Обработка каждого канала независимо
    for (int channel = 0; channel < totalNumInputChannels; ++channel)
    {
        // Получение указателя на данные канала
        auto* channelData = buffer.getWritePointer (channel);

        // Обработка каждого сэмпла в канале
        for (int sample = 0; sample < buffer.getNumSamples(); ++sample)
        {   
            // Применение дисторшна к каждому сэмплу
            channelData[sample] = processSample(channelData[sample], channel);
        }
    }
}

//==============================================================================
bool BeastDistortionAudioProcessor::hasEditor() const
{
    return true; // Плагин имеет собственный редактор
}

juce::AudioProcessorEditor* BeastDistortionAudioProcessor::createEditor()
{   
    // Создание экземпляра редактора
    return new BeastDistortionAudioProcessorEditor (*this);
}

//==============================================================================
void BeastDistortionAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // Сохранение состояния плагина в бинарные данные
    
    // Создание XML-структуры для хранения настроек
    std::unique_ptr<juce::XmlElement> xml(new juce::XmlElement("BeastDistortionSettings"));

    // Сохранение значений всех параметров в XML-атрибуты
    xml->setAttribute("gain", gainParam->get());
    xml->setAttribute("drive", driveParam->get());
    xml->setAttribute("output", outputParam->get());
    xml->setAttribute("type", typeParam->getIndex());
    xml->setAttribute("bypass", bypassParam->get());

    // Копирование XML в бинарный блок данных
    copyXmlToBinary(*xml, destData);
}

void BeastDistortionAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // Восстановление состояния плагина из бинарных данных

    // Извлечение XML из бинарных данных
    std::unique_ptr<juce::XmlElement> xml(getXmlFromBinary(data, sizeInBytes));

    // Проверка корректности XML и тега
    if (xml != nullptr && xml->hasTagName("BeastDistortionSettings"))
    {
        // Восстановление значений параметров с нормализацией в диапазон 0-1
        gainParam->setValueNotifyingHost(xml->getDoubleAttribute("gain", 50.0) / 100.0f);
        driveParam->setValueNotifyingHost(xml->getDoubleAttribute("drive", 50.0) / 100.0f);
        outputParam->setValueNotifyingHost(xml->getDoubleAttribute("output", 50.0) / 100.0f);
        typeParam->setValueNotifyingHost(xml->getIntAttribute("type", 0) / 3.0f);
        bypassParam->setValueNotifyingHost(xml->getBoolAttribute("bypass", false) ? 1.0f : 0.0f);
    }
}

//==============================================================================
// Функция создания экземпляра плагина (требуется JUCE)
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new BeastDistortionAudioProcessor();
}
