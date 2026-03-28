#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

class BasicCompressorAudioProcessorEditor : public juce::AudioProcessorEditor,
    public juce::Slider::Listener,
    public juce::Timer
{
public:
    BasicCompressorAudioProcessorEditor(BasicCompressorAudioProcessor&);
    ~BasicCompressorAudioProcessorEditor() override;

    void paint(juce::Graphics&) override;
    void resized() override;

    // Funciones requeridas por Listener y Timer
    void sliderValueChanged(juce::Slider* slider) override;
    void timerCallback() override;

private:
    BasicCompressorAudioProcessor& audioProcessor;

    // Declaración de los sliders y etiquetas
    juce::Slider gainSlider;
    juce::Slider thresholdSlider, ratioSlider, attackSlider, releaseSlider;
    juce::Label thresholdLabel, ratioLabel, attackLabel, releaseLabel;

    // Clase interna para la gráfica del compresor
    class CompressorDisplay : public juce::Component
    {
    public:
        void paint(juce::Graphics& g) override;
        void setParameters(float thresh, float r) { threshold = thresh; ratio = r; repaint(); }
    private:
        float threshold = -20.0f;
        float ratio = 2.0f;
        float compressorTransferFunction(float inputDb)
        {
            if (inputDb < threshold) return inputDb;
            return threshold + (inputDb - threshold) / ratio;
        }
    };

    // Clase interna para los vúmetros
    class LevelMeter : public juce::Component
    {
    public:
        void paint(juce::Graphics& g) override;
        void setLevel(float newLevelDb) { levelDb = newLevelDb; repaint(); }
    private:
        float levelDb = -60.0f;
    };

    // Instancias de nuestras clases visuales
    CompressorDisplay compressorDisplay;
    LevelMeter inputLevelMeter;
    LevelMeter outputLevelMeter;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(BasicCompressorAudioProcessorEditor)
};