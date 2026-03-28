#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

// Ya no heredamos de Slider::Listener
class BasicCompressorAudioProcessorEditor : public juce::AudioProcessorEditor,
	public juce::Timer
{
public:
	BasicCompressorAudioProcessorEditor(BasicCompressorAudioProcessor& p);
	~BasicCompressorAudioProcessorEditor() noexcept override;

	void paint(juce::Graphics&) override;
	void resized() override;
	void timerCallback() override;

private:
	BasicCompressorAudioProcessor& audioProcessor;

	juce::Slider gainSlider;
	juce::Label thresholdLabel, ratioLabel, attackLabel, releaseLabel;
	juce::Slider thresholdSlider, ratioSlider, attackSlider, releaseSlider;

	// Adjuntos (Attachments) que conectan la UI con el APVTS automáticamente
	using SliderAttachment = juce::AudioProcessorValueTreeState::SliderAttachment;
	std::unique_ptr<SliderAttachment> gainAttach, threshAttach, ratioAttach, attAttach, relAttach;

	class CompressorDisplay : public juce::Component
	{
	public:
		CompressorDisplay() : threshold(-20.0f), ratio(2.0f) {}

		void setParameters(float newThreshold, float newRatio)
		{
			threshold = newThreshold;
			ratio = newRatio;
			repaint();
		}
		void paint(juce::Graphics& g) override;

	private:
		float threshold, ratio;
		float compressorTransferFunction(float inputDb)
		{
			if (inputDb < threshold) return inputDb;
			return threshold + (inputDb - threshold) / ratio;
		}
	};

	CompressorDisplay compressorDisplay;

	class LevelMeter : public juce::Component
	{
	public:
		LevelMeter() : levelDb(-100.0f) {}
		void setLevel(float newLevelDb)
		{
			levelDb = newLevelDb;
			repaint();
		}
		void paint(juce::Graphics& g) override;

	private:
		float levelDb;
	};

	LevelMeter inputLevelMeter;
	LevelMeter outputLevelMeter;

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(BasicCompressorAudioProcessorEditor)
};