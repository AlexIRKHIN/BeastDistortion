/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/


#include "PluginProcessor.h"
#include "PluginEditor.h"


//==============================================================================
BeastDistortionAudioProcessorEditor::BeastDistortionAudioProcessorEditor(BeastDistortionAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p), needsDelayedSync(true)
{
    // Размер окна 800x600 
    setSize(800, 600);

    // Настройка цветов
    backgroundColour = juce::Colour(40, 40, 40);
    textColour = juce::Colour(200, 200, 200);
    sliderColour = juce::Colour(255, 80, 0);

    // === НАСТРОЙКА СЛАЙДЕРОВ ===

    auto setupSlider = [this](juce::Slider& slider, juce::Label& valueLabel) {
        slider.setSliderStyle(juce::Slider::RotaryVerticalDrag);
        slider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
        slider.setRange(0.0, 100.0, 1.0);
        slider.setColour(juce::Slider::rotarySliderFillColourId, sliderColour);
        slider.setColour(juce::Slider::rotarySliderOutlineColourId, juce::Colour(100, 100, 100));
        slider.setColour(juce::Slider::thumbColourId, juce::Colour(255, 255, 255));
        slider.setSliderSnapsToMousePosition(false);
        slider.addListener(this);
        addAndMakeVisible(slider);

        // Настраиваем лейбл значения
        valueLabel.setJustificationType(juce::Justification::centred);
        valueLabel.setColour(juce::Label::textColourId, juce::Colour(255, 255, 255));
        valueLabel.setFont(juce::Font("Impact", 54.0f, juce::Font::bold));
        valueLabel.setInterceptsMouseClicks(false, false);
        addAndMakeVisible(valueLabel);
        };

    setupSlider(gainSlider, gainValueLabel);
    setupSlider(distortionSlider, distortionValueLabel);
    setupSlider(outputSlider, outputValueLabel);

    // === НАСТРОЙКА ЛЕЙБЛОВ СЛАЙДЕРОВ ===

    auto setupSliderLabel = [this](juce::Label& label, const juce::String& text) {
        label.setText(text, juce::dontSendNotification);
        label.setJustificationType(juce::Justification::centred);
        label.setColour(juce::Label::textColourId, textColour);
        label.setFont(juce::Font("Tahoma", 24.0f, juce::Font::bold));
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
    typeComboBox.addListener(this);
    typeComboBox.setColour(juce::ComboBox::backgroundColourId, juce::Colour(70, 70, 70));
    typeComboBox.setColour(juce::ComboBox::textColourId, textColour);
    typeComboBox.setColour(juce::ComboBox::arrowColourId, sliderColour);
    addAndMakeVisible(typeComboBox);

    typeLabel.setText("DISTORTION TYPE", juce::dontSendNotification);
    typeLabel.setJustificationType(juce::Justification::centred);
    typeLabel.setColour(juce::Label::textColourId, textColour);
    typeLabel.setFont(juce::Font("Tahoma", 16.0f, juce::Font::bold));
    addAndMakeVisible(typeLabel);

    // === НАСТРОЙКА КНОПОК ===

    resetButton.setButtonText("RESET");
    resetButton.addListener(this);
    resetButton.setColour(juce::TextButton::buttonColourId, juce::Colour(80, 80, 80));
    resetButton.setColour(juce::TextButton::textColourOffId, sliderColour);
    addAndMakeVisible(resetButton);

    bypassButton.setButtonText("ON/OFF");
    bypassButton.setClickingTogglesState(true);
    bypassButton.addListener(this);
    bypassButton.setColour(juce::TextButton::buttonColourId, juce::Colour(80, 80, 80));
    bypassButton.setColour(juce::TextButton::textColourOffId, juce::Colours::limegreen);
    bypassButton.setColour(juce::TextButton::textColourOnId, sliderColour);
    addAndMakeVisible(bypassButton);

    // === НАСТРОЙКА ЗАГОЛОВКА ===

    titleLabel.setText("BEAST DISTORTION", juce::dontSendNotification);
    titleLabel.setJustificationType(juce::Justification::centred);
    titleLabel.setColour(juce::Label::textColourId, juce::Colour(255, 80, 0));
    titleLabel.setFont(juce::Font("Impact", 42.0f, juce::Font::plain));
    addAndMakeVisible(titleLabel);

    // === СИНХРОНИЗАЦИЯ С ПАРАМЕТРАМИ ПРОЦЕССОРА ===

    // Синхронизируем слайдеры с текущими значениями параметров
    gainSlider.setValue(audioProcessor.getGainParam()->get(), juce::dontSendNotification);
    distortionSlider.setValue(audioProcessor.getDriveParam()->get(), juce::dontSendNotification);
    outputSlider.setValue(audioProcessor.getOutputParam()->get(), juce::dontSendNotification);

    // Синхронизируем комбобокс типа дисторшна
    typeComboBox.setSelectedId(audioProcessor.getTypeParam()->getIndex() + 1, juce::dontSendNotification);

    // Синхронизируем кнопку bypass
    bypassButton.setToggleState(audioProcessor.getBypassParam()->get(), juce::dontSendNotification);

    // Обновляем лейблы значений
    gainValueLabel.setText(juce::String(gainSlider.getValue(), 0), juce::dontSendNotification);
    distortionValueLabel.setText(juce::String(distortionSlider.getValue(), 0), juce::dontSendNotification);
    outputValueLabel.setText(juce::String(outputSlider.getValue(), 0), juce::dontSendNotification);

    // Запускаем таймер для отложенной синхронизации
    startTimer(100);  // 100ms задержка
}

BeastDistortionAudioProcessorEditor::~BeastDistortionAudioProcessorEditor()
{
    // Останавливаем таймер при уничтожении редактора
    stopTimer();
}

void BeastDistortionAudioProcessorEditor::timerCallback()
{
    if (needsDelayedSync)
    {
        // Останавливаем таймер после первой синхронизации
        stopTimer();
        needsDelayedSync = false;

        // Повторная синхронизация с параметрами процессора
        gainSlider.setValue(audioProcessor.getGainParam()->get(), juce::dontSendNotification);
        distortionSlider.setValue(audioProcessor.getDriveParam()->get(), juce::dontSendNotification);
        outputSlider.setValue(audioProcessor.getOutputParam()->get(), juce::dontSendNotification);
        typeComboBox.setSelectedId(audioProcessor.getTypeParam()->getIndex() + 1, juce::dontSendNotification);
        bypassButton.setToggleState(audioProcessor.getBypassParam()->get(), juce::dontSendNotification);

        // Обновляем лейблы значений
        gainValueLabel.setText(juce::String(gainSlider.getValue(), 0), juce::dontSendNotification);
        distortionValueLabel.setText(juce::String(distortionSlider.getValue(), 0), juce::dontSendNotification);
        outputValueLabel.setText(juce::String(outputSlider.getValue(), 0), juce::dontSendNotification);
    }
}

//==============================================================================
void BeastDistortionAudioProcessorEditor::paint(juce::Graphics& g)
{
    // Заливаем фон
    g.fillAll(backgroundColour);

    // Устанавливаем белый цвет для прямоугольников
    g.setColour(juce::Colours::white);

    // Получаем размеры окна
    auto width = getWidth();
    auto height = getHeight();
    int borderThickness = 10; // Толщина рамки

    // 1. Верхняя рамка
    g.fillRect(0, 0, width, borderThickness);

    // 2. Нижняя рамка
    g.fillRect(0, height - borderThickness, width, borderThickness);

    // 3. Левая рамка
    g.fillRect(0, 0, borderThickness, height);

    // 4. Правая рамка
    g.fillRect(width - borderThickness, 0, borderThickness, height);

    // 5. Разделительная линия под заголовком
    int headerBottom = 10 + 80; // Нижняя граница заголовка
    int separatorY = headerBottom + 10; // Положение разделителя (10px ниже заголовка)
    g.fillRect(0, separatorY, width, borderThickness);
}

void BeastDistortionAudioProcessorEditor::resized()
{
    auto area = getLocalBounds();

    // Учитываем рамки - оставляем отступ от краев
    area.reduce(10, 10); // Отступ 10px от каждой стороны для рамки

    // Заголовок - верхняя часть
    titleLabel.setBounds(0, 10, getWidth(), 80);

    // === ОБЛАСТЬ СЛАЙДЕРОВ ===

    // Высота области слайдеров (от разделителя до комбобокса)
    int sliderAreaHeight = 400;
    auto sliderArea = area.withTrimmedTop(100).withHeight(sliderAreaHeight);

    int sliderSize = 200; // Размер слайдера
    int labelHeight = 40;
    int valueLabelHeight = 120;
    int verticalOffset = 70; 
    int horizontalSpacing = 150; // Расстояние между слайдерами
    int sliderRaise = 70; // Поднимаем все слайдеры на 70 пикселей
    int distortionLower = 60;

    // CENTER - DISTORTION SLIDER (главный слайдер)
    auto centerX = getWidth() / 2;
    auto distortionY = sliderArea.getY() + (sliderArea.getHeight() / 2) - (sliderSize / 2) - sliderRaise + distortionLower;

    distortionSlider.setBounds(centerX - sliderSize / 2, distortionY, sliderSize, sliderSize);
    // Значение внутри слайдера
    distortionValueLabel.setBounds(centerX - 50, distortionY + sliderSize / 2 - valueLabelHeight / 2, 100, valueLabelHeight);
    distortionLabel.setBounds(centerX - 90, distortionY + sliderSize - 25, 180, labelHeight);

    // LEFT - GAIN SLIDER (слева и выше)
    auto gainX = centerX - sliderSize - horizontalSpacing;
    auto gainY = distortionY - verticalOffset;

    gainSlider.setBounds(gainX, gainY, sliderSize, sliderSize);
    // Значение внутри слайдера
    gainValueLabel.setBounds(gainX + sliderSize / 2 - 50, gainY + sliderSize / 2 - valueLabelHeight / 2, 100, valueLabelHeight);
    // Подпись под слайдером
    gainLabel.setBounds(gainX + sliderSize / 2 - 60, gainY + sliderSize - 25, 120, labelHeight);

    // RIGHT - OUTPUT SLIDER (справа и выше)
    auto outputX = centerX + horizontalSpacing;
    auto outputY = distortionY - verticalOffset;

    outputSlider.setBounds(outputX, outputY, sliderSize, sliderSize);
    // Значение внутри слайдера
    outputValueLabel.setBounds(outputX + sliderSize / 2 - 50, outputY + sliderSize / 2 - valueLabelHeight / 2, 100, valueLabelHeight);
    // Подпись под слайдером
    outputLabel.setBounds(outputX + sliderSize / 2 - 60, outputY + sliderSize - 25, 120, labelHeight);

    // === КОМБОБОКС И КНОПКИ ===

    // TYPE COMBOBOX
    int comboBoxWidth = 220;
    int comboBoxHeight = 50;
    int typeY = getHeight() - 170;

    typeComboBox.setBounds(centerX - comboBoxWidth / 2, typeY, comboBoxWidth, comboBoxHeight);

    // TYPE LABEL
    typeLabel.setText("DISTORTION TYPE", juce::dontSendNotification);
    typeLabel.setJustificationType(juce::Justification::centred);
    typeLabel.setBounds(centerX - 100, typeY + comboBoxHeight + 2, 200, 25);

    // === КНОПКИ ===

    int buttonWidth = 240;
    int buttonHeight = 40;
    int buttonY = getHeight() - 10 - 30 - buttonHeight;

    // RESET button - слева
    resetButton.setBounds(50, buttonY, buttonWidth, buttonHeight);

    // ON/OFF button - справа
    bypassButton.setBounds(getWidth() - 50 - buttonWidth, buttonY, buttonWidth, buttonHeight);
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