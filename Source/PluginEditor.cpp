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
    // Установка размера окна редактора
    setSize(800, 600);

    // Инициализация цветовой схемы
    backgroundColour = juce::Colour(40, 40, 40);        // Темно-серый фон
    textColour = juce::Colour(200, 200, 200);           // Светло-серый текст
    sliderColour = juce::Colour(255, 80, 0);            // Оранжевый акцент

    // === НАСТРОЙКА СЛАЙДЕРОВ ===

    // Лямбда-функция для настройки слайдеров и их числовых лейблов
    auto setupSlider = [this](juce::Slider& slider, juce::Label& valueLabel) {

        // Настройка внешнего вида и поведения слайдера
        slider.setSliderStyle(juce::Slider::RotaryVerticalDrag);                            // Круговой слайдер
        slider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);                       // Скрываем встроенное текстовое поле
        slider.setRange(0.0, 100.0, 1.0);                                                   // Диапазон значений 0-100 с шагом 1
        slider.setColour(juce::Slider::rotarySliderFillColourId, sliderColour);             // Оранжевое заполнение
        slider.setColour(juce::Slider::rotarySliderOutlineColourId, juce::Colours::grey);   // Серый контур
        slider.setColour(juce::Slider::thumbColourId, juce::Colours::white);                // Белый указатель
        slider.setSliderSnapsToMousePosition(false);                                        // Плавное перемещение (без привязки)
        slider.addListener(this);                                                           // Подписываемся на события изменения
        addAndMakeVisible(slider);                                                          // Делаем видимым

        // Настройка лейбла для отображения числового значения
        valueLabel.setJustificationType(juce::Justification::centred);                      // Выравнивание по центру
        valueLabel.setColour(juce::Label::textColourId, juce::Colours::white);              // Белый текст
        valueLabel.setFont(juce::Font("Century Gothic", 54.0f, juce::Font::plain));         // Шрифт Century Gothic для цифр
        valueLabel.setInterceptsMouseClicks(false, false);                                  // Отключаем взаимодействие с мышью
        addAndMakeVisible(valueLabel);                                                      // Делаем видимым
        };

    // Инициализация всех слайдеров
    setupSlider(gainSlider, gainValueLabel);                // Слайдер входного уровня
    setupSlider(distortionSlider, distortionValueLabel);    // Слайдер интенсивности дисторшна
    setupSlider(outputSlider, outputValueLabel);            // Слайдер выходного уровня

    // === НАСТРОЙКА ТЕКСТОВЫХ ЛЕЙБЛОВ СЛАЙДЕРОВ ===

    // Лямбда-функция для настройки подписей слайдеров
    auto setupSliderLabel = [this](juce::Label& label, const juce::String& text) {
        label.setText(text, juce::dontSendNotification);                // Установка текста
        label.setJustificationType(juce::Justification::centred);       // Выравнивание по центру
        label.setColour(juce::Label::textColourId, textColour);         // Светло-серый цвет
        label.setFont(juce::Font("Tahoma", 24.0f, juce::Font::bold));   // Жирный шрифт Tahoma
        addAndMakeVisible(label);                                       // Делаем видимым
        };

    // Инициализация всех подписей
    setupSliderLabel(gainLabel, "GAIN");                    // Подпись для Gain
    setupSliderLabel(distortionLabel, "DISTORTION");        // Подпись для Distortion
    setupSliderLabel(outputLabel, "OUTPUT");                // Подпись для Output

    // === НАСТРОЙКА ВЫБОРА ТИПА ДИСТОРШНА ===

    typeComboBox.addItem("HARD CLIP", 1);                   // Жесткое ограничение
    typeComboBox.addItem("SOFT CLIP", 2);                   // Мягкое ограничение
    typeComboBox.addItem("OVERDRIVE", 3);                   // Овердрайв
    typeComboBox.addItem("FOLDBACK", 4);                    // Фолдбэк
    
    typeComboBox.addListener(this);                         // Подписываемся на события изменения

    typeComboBox.setColour(juce::ComboBox::backgroundColourId, juce::Colour(70, 70, 70));   // Темно-серый фон
    typeComboBox.setColour(juce::ComboBox::textColourId, textColour);                       // Светло-серый текст
    typeComboBox.setColour(juce::ComboBox::arrowColourId, sliderColour);                    // Оранжевая стрелка
    addAndMakeVisible(typeComboBox);                                                        // Делаем видимым

    // Настройка подписи для комбобокса
    typeLabel.setText("DISTORTION TYPE", juce::dontSendNotification);           // Установка текста
    typeLabel.setJustificationType(juce::Justification::centred);               // Выравнивание по центру
    typeLabel.setColour(juce::Label::textColourId, textColour);                 // Светло-серый цвет
    typeLabel.setFont(juce::Font("Tahoma", 16.0f, juce::Font::bold));           // Жирный шрифт Tahoma
    addAndMakeVisible(typeLabel);                                               // Делаем видимым

    // === НАСТРОЙКА КНОПОК ===

    // Кнопка сброса параметров
    resetButton.setButtonText("RESET");
    resetButton.addListener(this);
    resetButton.setColour(juce::TextButton::buttonColourId, juce::Colour(80, 80, 80));      // Темно-серый фон
    resetButton.setColour(juce::TextButton::textColourOffId, sliderColour);                 // Оранжевый текст
    addAndMakeVisible(resetButton);                                                         // Делаем видимым

    // Кнопка включения/выключения эффекта
    bypassButton.setButtonText("ON/OFF");                                                   // Установка текста
    bypassButton.setClickingTogglesState(true);                                             // Переключатель (вкл/выкл)
    bypassButton.addListener(this);                                                         // Подписываемся на события изменения
    bypassButton.setColour(juce::TextButton::buttonColourId, juce::Colour(80, 80, 80));     // Темно-серый фон
    bypassButton.setColour(juce::TextButton::textColourOffId, juce::Colours::limegreen);    // Зеленый текст при выключенном
    bypassButton.setColour(juce::TextButton::textColourOnId, sliderColour);                 // Оранжевый текст при включенном
    addAndMakeVisible(bypassButton);                                                        // Делаем видимым

    // === НАСТРОЙКА ЗАГОЛОВКА С ЛОГОТИПОМ ===

   // Левый текст "BEAST"
    leftTitleLabel.setText("BEAST", juce::dontSendNotification);                // Установка текста
    leftTitleLabel.setJustificationType(juce::Justification::centredRight);     // Выравнивание по правому краю
    leftTitleLabel.setColour(juce::Label::textColourId, juce::Colours::white);  // Белый текст
    leftTitleLabel.setFont(juce::Font("Impact", 42.0f, juce::Font::plain));     // Шрифт Impact
    addAndMakeVisible(leftTitleLabel);                                          // Делаем видимым

    // Загрузка и настройка логотипа из бинарных данных
    auto logoImage = juce::ImageCache::getFromMemory(BinaryData::beast_png, BinaryData::beast_pngSize);

    // Если логотип не загрузился - просто не показываем его, но плагин продолжает работать
    if (logoImage.isValid())
    {
        logoComponent.setImage(logoImage, juce::RectanglePlacement::centred);   // Центрирование изображения
    }
    logoComponent.setInterceptsMouseClicks(false, false);                       // Отключаем взаимодействие с мышью
    addAndMakeVisible(logoComponent);                                           // Делаем видимым

    // Правый текст "DISTORTION"
    rightTitleLabel.setText("DISTORTION", juce::dontSendNotification);          // Установка текста
    rightTitleLabel.setJustificationType(juce::Justification::centredLeft);     // Выравнивание по левому краю
    rightTitleLabel.setColour(juce::Label::textColourId, juce::Colours::white); // Белый текст
    rightTitleLabel.setFont(juce::Font("Impact", 42.0f, juce::Font::plain));    // Шрифт Impact
    addAndMakeVisible(rightTitleLabel);                                         // Делаем видимым

    // === СИНХРОНИЗАЦИЯ ИНТЕРФЕЙСА С ПАРАМЕТРАМИ ПРОЦЕССОРА ===

    // Начальная синхронизация слайдеров с текущими значениями параметров
    gainSlider.setValue(audioProcessor.getGainParam()->get(), juce::dontSendNotification);
    distortionSlider.setValue(audioProcessor.getDriveParam()->get(), juce::dontSendNotification);
    outputSlider.setValue(audioProcessor.getOutputParam()->get(), juce::dontSendNotification);

    // Синхронизация комбобокса с текущим типом дисторшна
    typeComboBox.setSelectedId(audioProcessor.getTypeParam()->getIndex() + 1, juce::dontSendNotification);

    // Синхронизация кнопки ON/OFF
    bypassButton.setToggleState(audioProcessor.getBypassParam()->get(), juce::dontSendNotification);

    // Обновление числовых лейблов текущими значениями
    gainValueLabel.setText(juce::String(gainSlider.getValue(), 0), juce::dontSendNotification);
    distortionValueLabel.setText(juce::String(distortionSlider.getValue(), 0), juce::dontSendNotification);
    outputValueLabel.setText(juce::String(outputSlider.getValue(), 0), juce::dontSendNotification);

    // Запуск таймера для отложенной синхронизации (решает проблемы инициализации)
    startTimer(100);  // 100ms задержка
}

BeastDistortionAudioProcessorEditor::~BeastDistortionAudioProcessorEditor()
{
    // Остановка таймера при уничтожении редактора
    stopTimer();
}

//==============================================================================
void BeastDistortionAudioProcessorEditor::timerCallback()
{
    // Обработчик таймера для отложенной синхронизации
    if (needsDelayedSync)
    {
        
        stopTimer();                    // Останавка таймера после первой синхронизации
        needsDelayedSync = false;        // Сброс флага

        // Повторная синхронизация с параметрами процессора (для надежности)
        gainSlider.setValue(audioProcessor.getGainParam()->get(), juce::dontSendNotification);
        distortionSlider.setValue(audioProcessor.getDriveParam()->get(), juce::dontSendNotification);
        outputSlider.setValue(audioProcessor.getOutputParam()->get(), juce::dontSendNotification);
        typeComboBox.setSelectedId(audioProcessor.getTypeParam()->getIndex() + 1, juce::dontSendNotification);
        bypassButton.setToggleState(audioProcessor.getBypassParam()->get(), juce::dontSendNotification);

        // Обновление числовых лейблов
        gainValueLabel.setText(juce::String(gainSlider.getValue(), 0), juce::dontSendNotification);
        distortionValueLabel.setText(juce::String(distortionSlider.getValue(), 0), juce::dontSendNotification);
        outputValueLabel.setText(juce::String(outputSlider.getValue(), 0), juce::dontSendNotification);
    }
}

//==============================================================================
void BeastDistortionAudioProcessorEditor::paint(juce::Graphics& g)
{
    // Отрисовка фона
    g.fillAll(backgroundColour);                            // Заливка темно-серым цветом

    // Установка белого цвета для элементов оформления
    g.setColour(juce::Colours::white);                      // Белый цыет

    // Получение размеров окна
    auto width = getWidth();
    auto height = getHeight();
    int borderThickness = 10;                               // Толщина рамки

    // Отрисовка рамки вокруг всего интерфейса
    
    g.fillRect(0, 0, width, borderThickness);                           // Верхняя рамка
    g.fillRect(0, height - borderThickness, width, borderThickness);    // Нижняя рамка
    g.fillRect(0, 0, borderThickness, height);                          // Левая рамка
    g.fillRect(width - borderThickness, 0, borderThickness, height);    // Правая рамка

    // Отрисовка разделительной линии под заголовком
    int headerBottom = 10 + 80;                                         // Нижняя граница области заголовка
    int separatorY = headerBottom + 10;                                 // Позиция разделителя (10px ниже заголовка)    
    g.fillRect(0, separatorY, width, borderThickness);                  // Горизонтальная линия
}

//==============================================================================
void BeastDistortionAudioProcessorEditor::resized()
{
    auto area = getLocalBounds();

    // Создание отступов от краев для учета рамки
    area.reduce(10, 10);                            // Создание отступов от краев для учета рамки

    // === КОМПОНОВКА ЗАГОЛОВКА С ЛОГОТИПОМ ===

    int headerHeight = 80;                          // Высота области заголовка
    int logoSize = 120;                             // Размер логотипа
    int textWidth = 200;                            // Ширина каждой текстовой части

    // Область заголовка
    auto headerArea = juce::Rectangle<int>(0, 10, getWidth(), headerHeight);

    // Расчет общей ширины и начальной позиции для центрирования
    int totalWidth = textWidth * 2 + logoSize;
    int startX = (getWidth() - totalWidth) / 2;

    // Левая часть заголовка "BEAST" (приподнята на 15px для эффекта лесенки)
    leftTitleLabel.setBounds(startX,
        headerArea.getY() - 15,                     // Смещение вверх
        textWidth,
        headerHeight);

    // Центральный логотип
    logoComponent.setBounds(startX + textWidth,
        headerArea.getY() + (headerHeight - logoSize) / 2 + 5,  // Вертикальное центрирование + смещение
        logoSize,
        logoSize);

    // Правая часть заголовка "DISTORTION" (опущена на 15px для эффекта лесенки)
    rightTitleLabel.setBounds(startX + textWidth + logoSize,
        headerArea.getY() + 15,                     // Смещение вниз
        textWidth,
        headerHeight);

    // === КОМПОНОВКА ОБЛАСТИ СЛАЙДЕРОВ ===

    int sliderAreaHeight = 400;                                                 // Высота области слайдеров
    auto sliderArea = area.withTrimmedTop(100).withHeight(sliderAreaHeight);    // Область под слайдеры

    // Размеры и отступы для слайдеров
    int sliderSize = 200;           // Диаметр круговых слайдеров
    int labelHeight = 40;           // Высота подписей
    int valueLabelHeight = 120;     // Высота числовых лейблов
    int verticalOffset = 70;        // Вертикальное смещение боковых слайдеров
    int horizontalSpacing = 150;    // Горизонтальное расстояние между слайдерами
    int sliderRaise = 70;           // Общее поднятие всех слайдеров
    int distortionLower = 60;       // Дополнительное опускание центрального слайдера

    // Центральный слайдер DISTORTION (главный, расположен ниже)
    auto centerX = getWidth() / 2;      // Расположение по горизонтали
    auto distortionY = sliderArea.getY() + (sliderArea.getHeight() / 2) - (sliderSize / 2) - sliderRaise + distortionLower;     // Расположение по вертикали

    // Позиционирование центрального слайдера и его элементов   
    distortionSlider.setBounds(centerX - sliderSize / 2, distortionY, sliderSize, sliderSize);                                  // Позиционирование слайдера
    distortionValueLabel.setBounds(centerX - 50, distortionY + sliderSize / 2 - valueLabelHeight / 2, 100, valueLabelHeight);   // Позиционирование значения внутри слайдера
    distortionLabel.setBounds(centerX - 90, distortionY + sliderSize - 25, 180, labelHeight);                                   // Позиционирование подписи под слайдером

    // Левый слайдер GAIN (расположен выше и слева)
    auto gainX = centerX - sliderSize - horizontalSpacing;
    auto gainY = distortionY - verticalOffset;

    gainSlider.setBounds(gainX, gainY, sliderSize, sliderSize);
    gainValueLabel.setBounds(gainX + sliderSize / 2 - 50, gainY + sliderSize / 2 - valueLabelHeight / 2, 100, valueLabelHeight);
    gainLabel.setBounds(gainX + sliderSize / 2 - 60, gainY + sliderSize - 25, 120, labelHeight);

    // Правый слайдер OUTPUT (расположен выше и справа)
    auto outputX = centerX + horizontalSpacing;
    auto outputY = distortionY - verticalOffset;

    outputSlider.setBounds(outputX, outputY, sliderSize, sliderSize);
    outputValueLabel.setBounds(outputX + sliderSize / 2 - 50, outputY + sliderSize / 2 - valueLabelHeight / 2, 100, valueLabelHeight);
    outputLabel.setBounds(outputX + sliderSize / 2 - 60, outputY + sliderSize - 25, 120, labelHeight);

    // === КОМПОНОВКА КОМБОБОКСА И КНОПОК ===

    // Выпадающий список типа дисторшна
    int comboBoxWidth = 220;
    int comboBoxHeight = 50;
    int typeY = getHeight() - 170;      // Позиция по Y

    typeComboBox.setBounds(centerX - comboBoxWidth / 2, typeY, comboBoxWidth, comboBoxHeight);

    // Подпись для комбобокса
    typeLabel.setBounds(centerX - 100, typeY + comboBoxHeight + 2, 200, 25);

    // === КНОПКИ ===

    int buttonWidth = 240;
    int buttonHeight = 40;
    int buttonY = getHeight() - 10 - 30 - buttonHeight;             // Позиция по вертикали

    // Кнопка RESET слева
    resetButton.setBounds(50, buttonY, buttonWidth, buttonHeight);

    // Кнопка ON/OFF справа
    bypassButton.setBounds(getWidth() - 50 - buttonWidth, buttonY, buttonWidth, buttonHeight);
}

//==============================================================================
void BeastDistortionAudioProcessorEditor::sliderValueChanged(juce::Slider* slider)
{
    // Обработчик изменения значений слайдеров
    if (slider == &gainSlider)
    {
        // Обновление параметра Gain в процессоре (нормализация к диапазону 0-1)
        audioProcessor.getGainParam()->setValueNotifyingHost(gainSlider.getValue() / 100.0f);

        // Обновление числового лейбла
        gainValueLabel.setText(juce::String(gainSlider.getValue(), 0), juce::dontSendNotification);
    }
    else if (slider == &distortionSlider)
    { 
        audioProcessor.getDriveParam()->setValueNotifyingHost(distortionSlider.getValue() / 100.0f);
        distortionValueLabel.setText(juce::String(distortionSlider.getValue(), 0), juce::dontSendNotification);
    }
    else if (slider == &outputSlider)
    {
        audioProcessor.getOutputParam()->setValueNotifyingHost(outputSlider.getValue() / 100.0f);
        outputValueLabel.setText(juce::String(outputSlider.getValue(), 0), juce::dontSendNotification);
    }
}

void BeastDistortionAudioProcessorEditor::comboBoxChanged(juce::ComboBox* comboBox)
{
    // Обработчик изменения выбора типа дисторшна
    if (comboBox == &typeComboBox)
    {
        // Обновление параметра типа дисторшна (нормализация к диапазону 0-1)
        audioProcessor.getTypeParam()->setValueNotifyingHost((typeComboBox.getSelectedId() - 1) / 3.0f);
    }
}

void BeastDistortionAudioProcessorEditor::buttonClicked(juce::Button* button)
{
    // Обработчик нажатия кнопок
    if (button == &resetButton)
    {
        // Сброс всех параметров к значениям по умолчанию
        gainSlider.setValue(50.0);                                      // Gain: 50%
        distortionSlider.setValue(50.0);                                // Drive: 50%
        outputSlider.setValue(50.0);                                    // Output: 50%
        typeComboBox.setSelectedId(1);                                  // Тип: Hard Clip
        bypassButton.setToggleState(false, juce::sendNotification);     // Bypass: выключен
    }
    else if (button == &bypassButton)
    {
        // Переключение состояния байпаса 
        audioProcessor.getBypassParam()->setValueNotifyingHost(bypassButton.getToggleState() ? 1.0f : 0.0f);
    }
}