
BEAST DISTORTION - аудиоплагин для обработки звука с различными типами дисторшн-эффектов. Плагин разработан в формате VST3 и совместим с большинством современных DAW.

Особенности

* 4 типа дисторшн: Hard Clip, Soft Clip, Overdrive, Foldback

* 3 параметра управления: Gain, Distortion, Output

* Режим Bypass: мгновенное сравнение обработанного и чистого сигнала

* Сброс настроек: одна кнопка для возврата к значениям по умолчанию

* GUI: интуитивно понятное управление

Параметры плагина:
* Gain: Регулирует уровень входного сигнала
* Distortion: Контролирует интенсивность эффекта дисторшн
* Output: Управляет уровнем выходного сигнала
* Type: Выбор алгоритма дисторшн:
  + Hard Clip: Резкое ограничение сигнала
  + Soft Clip: Плавное ограничение через гиперболический тангенс
  + Overdrive: Асимметричное ограничение, имитирующее ламповые усилители
  + Foldback: "Сворачивание" сигнала при превышении лимитов

Архитектура проекта.
- PluginProcessor.h/cpp  -  реализует логику логику обработки аудио
- PluginEditor.h/cpp     -   реализует графический интерфейс
- BeastDistortion.jucer        - Файл проекта Projucer

Ключевые компоненты:
* BeastDistortionAudioProcessor - обработка аудио, управление параметрами
* BeastDistortionAudioProcessorEditor - пользовательский интерфейс
* Алгоритмы дисторшн реализованы в методе processSample()

Основные параметры плагина:
- AudioParameterFloat* gainParam;      // Входной уровень сигнала (0-100)
- AudioParameterFloat* driveParam;     // Уровень дисторшн сигнала (0-100)
- AudioParameterFloat* outputParam;    // Выходной уровень сигнаоа (0-100)
- AudioParameterChoice* typeParam;     // Тип дисторшна
- AudioParameterBool* bypassParam;     // Вкл/Выкл обработку

UML диаграммы:
<img width="1548" height="1028" alt="image" src="https://github.com/user-attachments/assets/096a79e3-565b-453d-a799-d1d1c71465ae" />

Сделано с ❤️ и C++
