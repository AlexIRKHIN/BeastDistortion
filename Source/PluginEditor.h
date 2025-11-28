/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

//==============================================================================
// Редактор аудиоплагина Beast Distortion
// Отвечает за весь пользовательский интерфейс и обработку взаимодействий
class BeastDistortionAudioProcessorEditor  : public juce::AudioProcessorEditor,
    public juce::Slider::Listener,
    public juce::ComboBox::Listener,
    public juce::Button::Listener,
    public juce::Timer
{
public:

    // Конструктор - создает редактор и инициализирует все компоненты интерфейса 
    BeastDistortionAudioProcessorEditor (BeastDistortionAudioProcessor&);

    // Деструктор
    ~BeastDistortionAudioProcessorEditor() override;

    //==============================================================================
    // Основной метод отрисовки - рисует фон и рамки
    void paint (juce::Graphics&) override;

    // Метод компоновки - размещает все компоненты интерфейса
    void resized() override;

    //==============================================================================
    // Методы обратного вызова слушателей

    // Вызывается при изменении значения любого слайдера - обновляет соответствующий параметр
    void sliderValueChanged(juce::Slider* slider) override;

    // Вызывается при изменении выбора типа дисторшна
    void comboBoxChanged(juce::ComboBox* comboBox) override;

    // Вызывается при нажатии кнопок
    void buttonClicked(juce::Button* button) override;

    // Callback таймера для отложенной синхронизации с параметрами процессора
    void timerCallback() override;

private:
    // Ссылка на аудиопроцессор
    BeastDistortionAudioProcessor& audioProcessor;

    //==============================================================================
    // Компоненты интерфейса
    
    // Слайдеры (Фейдеры)
    juce::Slider gainSlider;        // Регулятор входного уровня сигнала (0-100)
    juce::Slider distortionSlider;  // Регулятор интенсивности эффекта дисторшна (0-100)
    juce::Slider outputSlider;      // Регулятор выходного уровня сигнала (0-100)

    // Подписи слайдеров
    juce::Label gainLabel;          // Подпись "GAIN"
    juce::Label distortionLabel;    // Подпись "DISTORTION"        
    juce::Label outputLabel;        // Подпись "OUTPUT"

    // Лейблы значений слайдеров
    juce::Label gainValueLabel;         // Отображает текущее значение Gain
    juce::Label distortionValueLabel;   // Отображает текущее значение Distortion
    juce::Label outputValueLabel;       // Отображает текущее значение Output

    // Выбор типа эффекта дисторшна
    juce::ComboBox typeComboBox;        // Выпадающий список для выбора типа дисторшна
    juce::Label typeLabel;              // Подпись "DISTORTION TYPE"

    // Кнопки
    juce::TextButton resetButton;       // Сбрасывает все параметры к значениям по умолчанию
    juce::TextButton bypassButton;      // Включает/выключает обработку плагина

    // Компоненты заголовка с логотипом
    juce::Label leftTitleLabel;          // Текст "BEAST"
    juce::ImageComponent logoComponent;  // Изображение логотипа Beast
    juce::Label rightTitleLabel;         // Текст "DISTORTION"

    // Цвета
    juce::Colour backgroundColour;      // Темно-серый фон (#282828)
    juce::Colour textColour;            // Светло-серый текст (#C8C8C8)
    juce::Colour sliderColour;          // Оранжевый акцентный цвет для слайдеров (#FF5000)

    //==============================================================================
    // Управление состоянием
    bool needsDelayedSync;              // Флаг для первоначальной синхронизации параметров

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (BeastDistortionAudioProcessorEditor)
};