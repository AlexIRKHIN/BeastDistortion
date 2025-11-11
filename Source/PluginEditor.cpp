/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/


#include "PluginProcessor.h"
#include "PluginEditor.h"


//==============================================================================
BeastDistortionAudioProcessorEditor::BeastDistortionAudioProcessorEditor (BeastDistortionAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    // Размер окна 800x600 
    setSize(800, 600);

    // Настройка цветов
    backgroundColour = juce::Colour(40, 40, 40);
    textColour = juce::Colour(200, 200, 200);
    sliderColour = juce::Colour(255, 80, 0);

    // === НАСТРОЙКА СЛАЙДЕРОВ ===

    auto setupSlider = [this](juce::Slider& slider, juce::Label& valueLabel, double defaultValue) {
        slider.setSliderStyle(juce::Slider::RotaryVerticalDrag);
        slider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
        slider.setRange(0.0, 100.0, 1.0);
        slider.setValue(defaultValue);
        slider.setColour(juce::Slider::rotarySliderFillColourId, sliderColour);
        slider.setColour(juce::Slider::rotarySliderOutlineColourId, juce::Colour(100, 100, 100));
        slider.setColour(juce::Slider::thumbColourId, juce::Colour(255, 255, 255));
        slider.setSliderSnapsToMousePosition(false);
        slider.addListener(this);
        addAndMakeVisible(slider);

        // Настраиваем лейбл значения
        valueLabel.setJustificationType(juce::Justification::centred);
        valueLabel.setColour(juce::Label::textColourId, juce::Colour(255, 255, 255));
        valueLabel.setFont(juce::Font(18.0f, juce::Font::bold));
        valueLabel.setText(juce::String(defaultValue, 0), juce::dontSendNotification);
        addAndMakeVisible(valueLabel);
        };

    setupSlider(gainSlider, gainValueLabel, 50.0);
    setupSlider(distortionSlider, distortionValueLabel, 50.0);
    setupSlider(outputSlider, outputValueLabel, 50.0);

    // === НАСТРОЙКА ЛЕЙБЛОВ СЛАЙДЕРОВ ===

    auto setupSliderLabel = [this](juce::Label& label, const juce::String& text) {
        label.setText(text, juce::dontSendNotification);
        label.setJustificationType(juce::Justification::centred);
        label.setColour(juce::Label::textColourId, textColour);
        label.setFont(juce::Font(16.0f, juce::Font::bold));
        addAndMakeVisible(label);
        };

    setupSliderLabel(gainLabel, "GAIN");
    setupSliderLabel(distortionLabel, "DISTORTION");
    setupSliderLabel(outputLabel, "OUTPUT");

    // === НАСТРОЙКА ВЫБОРА ТИПА ДИСТОРШНА ===

    typeComboBox.addItem("HARD CLIP", 1);
    typeComboBox.addItem("SOFT CLIP", 2);
    typeComboBox.addItem("OVERDRIVE", 3);
    typeComboBox.addItem("FOLDBACK", 4);
    typeComboBox.setSelectedId(1);
    typeComboBox.addListener(this);
    typeComboBox.setColour(juce::ComboBox::backgroundColourId, juce::Colour(70, 70, 70));
    typeComboBox.setColour(juce::ComboBox::textColourId, textColour);
    typeComboBox.setColour(juce::ComboBox::arrowColourId, sliderColour);
    addAndMakeVisible(typeComboBox);

    typeLabel.setText("TYPE:", juce::dontSendNotification);
    typeLabel.setJustificationType(juce::Justification::centredLeft);
    typeLabel.setColour(juce::Label::textColourId, textColour);
    typeLabel.setFont(juce::Font(16.0f, juce::Font::bold));
    addAndMakeVisible(typeLabel);

    // === НАСТРОЙКА ПРЕСЕТОВ ===

    presetComboBox.addItem("DEFAULT", 1);
    presetComboBox.setSelectedId(1);
    presetComboBox.setColour(juce::ComboBox::backgroundColourId, juce::Colour(70, 70, 70));
    presetComboBox.setColour(juce::ComboBox::textColourId, textColour);
    presetComboBox.setColour(juce::ComboBox::arrowColourId, sliderColour);
    addAndMakeVisible(presetComboBox);

    presetLabel.setText("PRESET:", juce::dontSendNotification);
    presetLabel.setJustificationType(juce::Justification::centredLeft);
    presetLabel.setColour(juce::Label::textColourId, textColour);
    presetLabel.setFont(juce::Font(16.0f, juce::Font::bold));
    addAndMakeVisible(presetLabel);

    // === НАСТРОЙКА КНОПОК ===

    resetButton.setButtonText("RESET");
    resetButton.addListener(this);
    resetButton.setColour(juce::TextButton::buttonColourId, juce::Colour(80, 80, 80));
    resetButton.setColour(juce::TextButton::textColourOffId, textColour);
    addAndMakeVisible(resetButton);

    bypassButton.setButtonText("ON/OFF");
    bypassButton.setClickingTogglesState(true);
    bypassButton.addListener(this);
    bypassButton.setColour(juce::TextButton::buttonColourId, juce::Colour(80, 80, 80));
    bypassButton.setColour(juce::TextButton::textColourOffId, textColour);
    bypassButton.setColour(juce::TextButton::textColourOnId, sliderColour);
    addAndMakeVisible(bypassButton);

    // === НАСТРОЙКА ЗАГОЛОВКА ===

    titleLabel.setText("BEAST DISTORTION", juce::dontSendNotification);
    titleLabel.setJustificationType(juce::Justification::centred);
    titleLabel.setColour(juce::Label::textColourId, juce::Colour(255, 80, 0));
    titleLabel.setFont(juce::Font(42.0f, juce::Font::bold));
    addAndMakeVisible(titleLabel);

}

BeastDistortionAudioProcessorEditor::~BeastDistortionAudioProcessorEditor()
{
}

//==============================================================================
void BeastDistortionAudioProcessorEditor::paint (juce::Graphics& g)
{
    g.fillAll(backgroundColour);

}

void BeastDistortionAudioProcessorEditor::resized()
{
    auto area = getLocalBounds();

    // Заголовок - верхняя часть
    titleLabel.setBounds(0, 10, 800, 80);

    // Основная область с слайдерами
    auto sliderArea = area.removeFromTop(350); // Высота области со слайдерами
    sliderArea.removeFromTop(80); // Отступаем от заголовка

    // Распределяем пространство для 3 слайдеров
    auto sliderWidth = sliderArea.getWidth() / 3;
    int sliderSize = 180; // Размер слайдера (диаметр)
    int labelHeight = 30;
    int valueLabelHeight = 40;

    // GAIN SLIDER
    auto gainArea = sliderArea.removeFromLeft(sliderWidth);
    gainSlider.setBounds(gainArea.withSizeKeepingCentre(sliderSize, sliderSize));
    // Значение внутри слайдера
    gainValueLabel.setBounds(gainArea.withSizeKeepingCentre(60, valueLabelHeight)
        .withY(gainSlider.getBounds().getCentreY() - valueLabelHeight / 2));
    // Подпись под слайдером
    gainLabel.setBounds(gainArea.withTrimmedTop(sliderSize + 10)
        .withHeight(labelHeight));

    // DISTORTION SLIDER
    auto distortionArea = sliderArea.removeFromLeft(sliderWidth);
    distortionSlider.setBounds(distortionArea.withSizeKeepingCentre(sliderSize, sliderSize));
    // Значение внутри слайдера
    distortionValueLabel.setBounds(distortionArea.withSizeKeepingCentre(60, valueLabelHeight)
        .withY(distortionSlider.getBounds().getCentreY() - valueLabelHeight / 2));
    // Подпись под слайдером
    distortionLabel.setBounds(distortionArea.withTrimmedTop(sliderSize + 10)
        .withHeight(labelHeight));

    // OUTPUT SLIDER
    auto outputArea = sliderArea;
    outputSlider.setBounds(outputArea.withSizeKeepingCentre(sliderSize, sliderSize));
    // Значение внутри слайдера
    outputValueLabel.setBounds(outputArea.withSizeKeepingCentre(60, valueLabelHeight)
        .withY(outputSlider.getBounds().getCentreY() - valueLabelHeight / 2));
    // Подпись под слайдером
    outputLabel.setBounds(outputArea.withTrimmedTop(sliderSize + 10)
        .withHeight(labelHeight));

    // === НИЖНЯЯ ПАНЕЛЬ КОНТРОЛЕВ ===
    auto controlArea = area.reduced(20);
    controlArea.removeFromTop(20); // Отступ сверху

    // Верхний ряд: TYPE и PRESET
    auto topRow = controlArea.removeFromTop(40);
    auto controlWidth = topRow.getWidth() / 2;

    // TYPE
    auto typeArea = topRow.removeFromLeft(controlWidth).reduced(5);
    typeLabel.setBounds(typeArea.removeFromLeft(60));
    typeComboBox.setBounds(typeArea.reduced(5, 0));

    // PRESET
    auto presetArea = topRow.reduced(5);
    presetLabel.setBounds(presetArea.removeFromLeft(60));
    presetComboBox.setBounds(presetArea.reduced(5, 0));

    // Нижний ряд: кнопки
    auto bottomRow = controlArea.removeFromTop(50);
    auto buttonWidth = bottomRow.getWidth() / 2;

    resetButton.setBounds(bottomRow.removeFromLeft(buttonWidth).reduced(20, 5));
    bypassButton.setBounds(bottomRow.reduced(20, 5));
}

//==============================================================================
void BeastDistortionAudioProcessorEditor::sliderValueChanged(juce::Slider* slider)
{
    if (slider == &gainSlider)
    {
        // используем getGainParam() 
        audioProcessor.getGainParam()->setValueNotifyingHost(gainSlider.getValue() / 100.0f);
        gainValueLabel.setText(juce::String(gainSlider.getValue(), 0), juce::dontSendNotification);
    }
    else if (slider == &distortionSlider)
    {
        // используем getDriveParam() 
        audioProcessor.getDriveParam()->setValueNotifyingHost(distortionSlider.getValue() / 100.0f);
        distortionValueLabel.setText(juce::String(distortionSlider.getValue(), 0), juce::dontSendNotification);
    }
    else if (slider == &outputSlider)
    {
        // используем getOutputParam()
        audioProcessor.getOutputParam()->setValueNotifyingHost(outputSlider.getValue() / 100.0f);
        outputValueLabel.setText(juce::String(outputSlider.getValue(), 0), juce::dontSendNotification);
    }
}

void BeastDistortionAudioProcessorEditor::comboBoxChanged(juce::ComboBox* comboBox)
{
    if (comboBox == &typeComboBox)
    {
        // используем getTypeParam() 
        audioProcessor.getTypeParam()->setValueNotifyingHost((typeComboBox.getSelectedId() - 1) / 3.0f);
    }
}

void BeastDistortionAudioProcessorEditor::buttonClicked(juce::Button* button)
{
    if (button == &resetButton)
    {
        // Сброс всех параметров к значениям по умолчанию
        gainSlider.setValue(50.0);
        distortionSlider.setValue(50.0);
        outputSlider.setValue(50.0);
        typeComboBox.setSelectedId(1);
        bypassButton.setToggleState(false, juce::sendNotification);
    }
    else if (button == &bypassButton)
    {
        // используем getBypassParam() 
        audioProcessor.getBypassParam()->setValueNotifyingHost(bypassButton.getToggleState() ? 1.0f : 0.0f);
        
    }
}