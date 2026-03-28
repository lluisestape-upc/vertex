#pragma once

#include <JuceHeader.h>
#include <juce_dsp/juce_dsp.h>

class BasicCompressorAudioProcessor : public juce::AudioProcessor
{
public:
	BasicCompressorAudioProcessor();
	~BasicCompressorAudioProcessor() override;

	// Tus variables de memoria (con los valores por defecto)
	float volumeDb = 0.0f;
	float threshValue = -20.0f;
	float ratioValue = 2.0f;
	float attackValue = 10.0f;
	float releaseValue = 100.0f;

	void prepareToPlay(double sampleRate, int samplesPerBlock) override;
	void releaseResources() override;
	bool isBusesLayoutSupported(const BusesLayout& layouts) const override;
	void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

	juce::AudioProcessorEditor* createEditor() override;
	bool hasEditor() const override;

	const juce::String getName() const override;

	bool acceptsMidi() const override;
	bool producesMidi() const override;
	bool isMidiEffect() const override;
	double getTailLengthSeconds() const override;

	int getNumPrograms() override;
	int getCurrentProgram() override;
	void setCurrentProgram(int index) override;
	const juce::String getProgramName(int index) override;
	void changeProgramName(int index, const juce::String& newName) override;

	// Funciones para guardar/cargar el estado (El "cerebro")
	void getStateInformation(juce::MemoryBlock& destData) override;
	void setStateInformation(const void* data, int sizeInBytes) override;

	float getCurrentInputLevelDb() const { return currentInputLevelDb; }
	float getCurrentOutputLevelDb() const { return currentOutputLevelDb; }

	juce::dsp::Compressor<float> compressor;

	// Nuestro APVTS y la función que define los parámetros
	juce::AudioProcessorValueTreeState apvts;
	juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();

private:
	float currentInputLevelDb = -100.0f;
	float currentOutputLevelDb = -100.0f;

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(BasicCompressorAudioProcessor)
};