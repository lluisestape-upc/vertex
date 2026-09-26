#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

//==============================================================================
namespace VertexColours
{
    const juce::Colour background { 0xfff4f7ff };
    const juce::Colour knobArea   { 0xffedf1fb };
    const juce::Colour panelBg    { 0xffffffff };
    const juce::Colour primary    { 0xff1144ee };
    const juce::Colour mid        { 0xff5577dd };
    const juce::Colour light      { 0xffaabbee };
    const juce::Colour textDark   { 0xff0a1a48 };
    const juce::Colour textMid    { 0xff5568a0 };
    const juce::Colour panelLine  { 0xffccd5ef };
    const juce::Colour trackBg    { 0xffe2e9f8 };
}

//==============================================================================
class VertexLookAndFeel : public juce::LookAndFeel_V4
{
public:
    VertexLookAndFeel();

    void drawRotarySlider(juce::Graphics&, int x, int y, int w, int h,
                          float sliderPos, float startAngle, float endAngle,
                          juce::Slider&) override;

    juce::Label* createSliderTextBox(juce::Slider&) override;
};

//==============================================================================
class BasicCompressorAudioProcessorEditor : public juce::AudioProcessorEditor,
                                            public juce::Timer
{
public:
    BasicCompressorAudioProcessorEditor(BasicCompressorAudioProcessor&);
    ~BasicCompressorAudioProcessorEditor() override;

    void paint(juce::Graphics&) override;
    void resized() override;
    void timerCallback() override;

private:
    BasicCompressorAudioProcessor& audioProcessor;
    VertexLookAndFeel vertexLF;

    // All five parameters as rotary knobs
    juce::Slider gainSlider, thresholdSlider, ratioSlider, attackSlider, releaseSlider;
    juce::Label  gainLabel, thresholdLabel, ratioLabel, attackLabel, releaseLabel;

    using SliderAttachment = juce::AudioProcessorValueTreeState::SliderAttachment;
    std::unique_ptr<SliderAttachment> gainAttach, threshAttach, ratioAttach,
                                      attackAttach, releaseAttach;

    // --- Compressor transfer curve ---
    class CompressorDisplay : public juce::Component
    {
    public:
        void paint(juce::Graphics& g) override;
        void setParameters(float thresh, float r) { threshold = thresh; ratio = r; repaint(); }
    private:
        float threshold = -20.0f, ratio = 2.0f;
        float compressorTransferFunction(float inputDb)
        {
            if (inputDb < threshold) return inputDb;
            return threshold + (inputDb - threshold) / ratio;
        }
    };

    // --- Level meter with slow ballistics ---
    class LevelMeter : public juce::Component
    {
    public:
        void paint(juce::Graphics& g) override;
        void setLevel(float newLevelDb)
        {
            float clamped = juce::jmax(newLevelDb, -60.0f);
            if (clamped >= displayDb)
                displayDb = clamped;                              // instant attack
            else
                displayDb = juce::jmax(displayDb - 1.2f, clamped); // ~36 dB/s decay at 30 Hz
            repaint();
        }
    private:
        float displayDb = -60.0f;
    };

    CompressorDisplay compressorDisplay;
    LevelMeter inputLevelMeter;
    LevelMeter outputLevelMeter;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(BasicCompressorAudioProcessorEditor)
};
