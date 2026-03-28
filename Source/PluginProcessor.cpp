#include "PluginProcessor.h"
#include "PluginEditor.h"

// Inicializamos el APVTS en el constructor
BasicCompressorAudioProcessor::BasicCompressorAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
	: AudioProcessor(BusesProperties()
#if ! JucePlugin_IsMidiEffect
#if ! JucePlugin_IsSynth
		.withInput("Input", juce::AudioChannelSet::stereo(), true)
#endif
		.withOutput("Output", juce::AudioChannelSet::stereo(), true)
#endif
	), apvts(*this, nullptr, "Parameters", createParameterLayout())
#endif
{
}

BasicCompressorAudioProcessor::~BasicCompressorAudioProcessor() {}

// Creamos los parámetros para que el DAW los vea
juce::AudioProcessorValueTreeState::ParameterLayout BasicCompressorAudioProcessor::createParameterLayout()
{
	std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;

	params.push_back(std::make_unique<juce::AudioParameterFloat>("threshold", "Threshold", -60.0f, 0.0f, -20.0f));
	params.push_back(std::make_unique<juce::AudioParameterFloat>("ratio", "Ratio", 1.0f, 10.0f, 2.0f));
	params.push_back(std::make_unique<juce::AudioParameterFloat>("attack", "Attack", 0.1f, 100.0f, 10.0f));
	params.push_back(std::make_unique<juce::AudioParameterFloat>("release", "Release", 10.0f, 500.0f, 100.0f));
	params.push_back(std::make_unique<juce::AudioParameterFloat>("gain", "Gain", -50.0f, 10.0f, 0.0f));

	return { params.begin(), params.end() };
}

const juce::String BasicCompressorAudioProcessor::getName() const { return JucePlugin_Name; }
bool BasicCompressorAudioProcessor::acceptsMidi() const { return false; }
bool BasicCompressorAudioProcessor::producesMidi() const { return false; }
bool BasicCompressorAudioProcessor::isMidiEffect() const { return false; }
double BasicCompressorAudioProcessor::getTailLengthSeconds() const { return 0.0; }
int BasicCompressorAudioProcessor::getNumPrograms() { return 1; }
int BasicCompressorAudioProcessor::getCurrentProgram() { return 0; }
void BasicCompressorAudioProcessor::setCurrentProgram(int index) {}
const juce::String BasicCompressorAudioProcessor::getProgramName(int index) { return {}; }
void BasicCompressorAudioProcessor::changeProgramName(int index, const juce::String& newName) {}

void BasicCompressorAudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
	juce::dsp::ProcessSpec spec;
	spec.sampleRate = sampleRate;
	spec.maximumBlockSize = samplesPerBlock;
	spec.numChannels = getTotalNumInputChannels();

	compressor.prepare(spec);
}

void BasicCompressorAudioProcessor::releaseResources() {}

#ifndef JucePlugin_PreferredChannelConfigurations
bool BasicCompressorAudioProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const
{
	if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
		&& layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
		return false;
	if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
		return false;
	return true;
}
#endif

void BasicCompressorAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
	juce::ScopedNoDenormals noDenormals;
	auto totalNumInputChannels = getTotalNumInputChannels();
	auto totalNumOutputChannels = getTotalNumOutputChannels();

	// 1. LEER VALORES DEL APVTS
	compressor.setThreshold(*apvts.getRawParameterValue("threshold"));
	compressor.setRatio(*apvts.getRawParameterValue("ratio"));
	compressor.setAttack(*apvts.getRawParameterValue("attack"));
	compressor.setRelease(*apvts.getRawParameterValue("release"));
	volumeDb = *apvts.getRawParameterValue("gain");

	float linearGain = std::pow(10.0f, volumeDb / 20.0f);

	for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
		buffer.clear(i, 0, buffer.getNumSamples());

	int numSamples = buffer.getNumSamples();

	// 2. MEDIR ENTRADA (Pre-Compresor)
	const float* inData = buffer.getReadPointer(0);
	float inputSumSquares = 0.0f;
	for (int i = 0; i < numSamples; ++i)
		inputSumSquares += inData[i] * inData[i];
	float inputRms = std::sqrt(inputSumSquares / numSamples);
	currentInputLevelDb = 20.0f * std::log10(inputRms + 1e-9f);

	// 3. PROCESAR COMPRESOR
	juce::dsp::AudioBlock<float> block(buffer);
	compressor.process(juce::dsp::ProcessContextReplacing<float>(block));

	// 4. APLICAR GAIN AL OUTPUT
	for (int channel = 0; channel < totalNumInputChannels; ++channel)
	{
		auto* channelData = buffer.getWritePointer(channel);
		for (int sample = 0; sample < numSamples; ++sample)
			channelData[sample] *= linearGain;
	}

	// 5. MEDIR SALIDA (Post-Compresor y Post-Gain)
	const float* outData = buffer.getReadPointer(0);
	float outputSumSquares = 0.0f;
	for (int i = 0; i < numSamples; ++i)
		outputSumSquares += outData[i] * outData[i];
	float outputRms = std::sqrt(outputSumSquares / numSamples);
	currentOutputLevelDb = 20.0f * std::log10(outputRms + 1e-9f);
}

bool BasicCompressorAudioProcessor::hasEditor() const { return true; }
juce::AudioProcessorEditor* BasicCompressorAudioProcessor::createEditor() { return new BasicCompressorAudioProcessorEditor(*this); }

// GUARDAR Y CARGAR DEL DAW
void BasicCompressorAudioProcessor::getStateInformation(juce::MemoryBlock& destData)
{
	auto state = apvts.copyState();
	std::unique_ptr<juce::XmlElement> xml(state.createXml());
	copyXmlToBinary(*xml, destData);
}

void BasicCompressorAudioProcessor::setStateInformation(const void* data, int sizeInBytes)
{
	std::unique_ptr<juce::XmlElement> xmlState(getXmlFromBinary(data, sizeInBytes));
	if (xmlState.get() != nullptr)
		if (xmlState->hasTagName(apvts.state.getType()))
			apvts.replaceState(juce::ValueTree::fromXml(*xmlState));
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter() { return new BasicCompressorAudioProcessor(); }