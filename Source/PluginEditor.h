/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

//==============================================================================
/**
*/
class BeastDistortionAudioProcessorEditor  : public juce::AudioProcessorEditor,
    public juce::Slider::Listener,
    public juce::ComboBox::Listener,
    public juce::Button::Listener

{
public:
    BeastDistortionAudioProcessorEditor (BeastDistortionAudioProcessor&);
    ~BeastDistortionAudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;

    // Наследуем методы слушателей
    void sliderValueChanged(juce::Slider* slider) override;
    void comboBoxChanged(juce::ComboBox* comboBox) override;
    void buttonClicked(juce::Button* button) override;

private:
    BeastDistortionAudioProcessor& audioProcessor;

    // Слайдеры (крутилки)
    juce::Slider gainSlider;
    juce::Slider distortionSlider;
    juce::Slider outputSlider;

    // Лейблы для слайдеров
    juce::Label gainLabel;
    juce::Label distortionLabel;
    juce::Label outputLabel;

    // Лейблы значений слайдеров
    juce::Label gainValueLabel;
    juce::Label distortionValueLabel;
    juce::Label outputValueLabel;

    // Выбор типа дисторшна
    juce::ComboBox typeComboBox;
    juce::Label typeLabel;

    // Кнопки
    juce::TextButton resetButton;
    juce::TextButton bypassButton;

    // Пресеты (пока заглушка)
    juce::ComboBox presetComboBox;
    juce::Label presetLabel;

    // Заголовок
    juce::Label titleLabel;

    // Цвета
    juce::Colour backgroundColour;
    juce::Colour textColour;
    juce::Colour sliderColour;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (BeastDistortionAudioProcessorEditor)
};
