#include "PluginProcessor.h"
#include "PluginEditor.h"

BasicCompressorAudioProcessorEditor::BasicCompressorAudioProcessorEditor(BasicCompressorAudioProcessor& p)
	: AudioProcessorEditor(&p), audioProcessor(p)
{
	setSize(620, 400);

	gainSlider.setSliderStyle(juce::Slider::LinearVertical);
	gainSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, true, 100, 25);
	addAndMakeVisible(gainSlider);

	thresholdLabel.setText("Threshold", juce::dontSendNotification);
	ratioLabel.setText("Ratio", juce::dontSendNotification);
	attackLabel.setText("Attack", juce::dontSendNotification);
	releaseLabel.setText("Release", juce::dontSendNotification);

	addAndMakeVisible(thresholdLabel);
	addAndMakeVisible(ratioLabel);
	addAndMakeVisible(attackLabel);
	addAndMakeVisible(releaseLabel);

	thresholdSlider.setSliderStyle(juce::Slider::Rotary);
	ratioSlider.setSliderStyle(juce::Slider::Rotary);
	attackSlider.setSliderStyle(juce::Slider::Rotary);
	releaseSlider.setSliderStyle(juce::Slider::Rotary);

	thresholdSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, true, 50, 20);
	ratioSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, true, 50, 20);
	attackSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, true, 50, 20);
	releaseSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, true, 50, 20);

	addAndMakeVisible(thresholdSlider);
	addAndMakeVisible(ratioSlider);
	addAndMakeVisible(attackSlider);
	addAndMakeVisible(releaseSlider);

	// ¡MAGIA! Aquí conectamos los Sliders a los parámetros del APVTS
	gainAttach = std::make_unique<SliderAttachment>(audioProcessor.apvts, "gain", gainSlider);
	threshAttach = std::make_unique<SliderAttachment>(audioProcessor.apvts, "threshold", thresholdSlider);
	ratioAttach = std::make_unique<SliderAttachment>(audioProcessor.apvts, "ratio", ratioSlider);
	attAttach = std::make_unique<SliderAttachment>(audioProcessor.apvts, "attack", attackSlider);
	relAttach = std::make_unique<SliderAttachment>(audioProcessor.apvts, "release", releaseSlider);

	addAndMakeVisible(compressorDisplay);
	addAndMakeVisible(inputLevelMeter);
	addAndMakeVisible(outputLevelMeter);

	startTimerHz(30);
}

BasicCompressorAudioProcessorEditor::~BasicCompressorAudioProcessorEditor() {}

void BasicCompressorAudioProcessorEditor::paint(juce::Graphics& g)
{
	g.fillAll(getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));
}

void BasicCompressorAudioProcessorEditor::resized()
{
	gainSlider.setBounds(0, 0, 120, getHeight());

	int knobSize = 70;
	int startX = 160;
	int startY = 35;
	int verticalSpacing = 90;

	auto centerTextX = startX + knobSize / 2 - 30;

	thresholdLabel.setBounds(centerTextX, startY - 20, 60, 20);
	thresholdSlider.setBounds(startX, startY, knobSize, knobSize);

	attackLabel.setBounds(centerTextX, startY + verticalSpacing - 20, 60, 20);
	attackSlider.setBounds(startX, startY + verticalSpacing, knobSize, knobSize);

	ratioLabel.setBounds(centerTextX, startY + 2 * verticalSpacing - 20, 60, 20);
	ratioSlider.setBounds(startX, startY + 2 * verticalSpacing, knobSize, knobSize);

	releaseLabel.setBounds(centerTextX, startY + 3 * verticalSpacing - 20, 60, 20);
	releaseSlider.setBounds(startX, startY + 3 * verticalSpacing, knobSize, knobSize);

	compressorDisplay.setBounds(280, 50, 280, 280);
	inputLevelMeter.setBounds(250, 50, 20, 280);
	outputLevelMeter.setBounds(570, 50, 20, 280);
}

void BasicCompressorAudioProcessorEditor::timerCallback()
{
	inputLevelMeter.setLevel(audioProcessor.getCurrentInputLevelDb());
	outputLevelMeter.setLevel(audioProcessor.getCurrentOutputLevelDb());

	// Actualizamos la gráfica leyendo los valores directamente de los knobs
	compressorDisplay.setParameters(thresholdSlider.getValue(), ratioSlider.getValue());
}

void BasicCompressorAudioProcessorEditor::CompressorDisplay::paint(juce::Graphics& g)
{
	auto bounds = getLocalBounds().toFloat();
	g.fillAll(juce::Colours::darkgrey);
	
	g.setColour(juce::Colours::grey);
	g.setFont(12.0f);
	g.drawText(" 0 dB", bounds.getX(), bounds.getY() + 2, 40, 15, juce::Justification::topLeft);
	g.drawText("-30 dB", bounds.getX(), bounds.getCentreY() - 7, 40, 15, juce::Justification::topLeft);
	g.drawLine(bounds.getX(), bounds.getCentreY(), bounds.getRight(), bounds.getCentreY(), 0.5f);
	
	g.drawText("0", bounds.getRight() - 15, bounds.getBottom() - 15, 15, 15, juce::Justification::bottomRight);
	g.drawText("-30", bounds.getCentreX() - 10, bounds.getBottom() - 15, 25, 15, juce::Justification::centredBottom);
	g.drawLine(bounds.getCentreX(), bounds.getY(), bounds.getCentreX(), bounds.getBottom(), 0.5f);

	g.setColour(juce::Colours::white);
	g.drawRect(bounds);

	constexpr float inputMin = -60.0f;
	constexpr float inputMax = 0.0f;
	constexpr int points = 200;

	juce::Path transferPath;
	for (int i = 0; i <= points; ++i)
	{
		float x = inputMin + (inputMax - inputMin) * i / points;
		float y = compressorTransferFunction(x);
		
		float px = juce::jmap<float>(x, inputMin, inputMax,
			static_cast<float>(bounds.getX()),
			static_cast<float>(bounds.getRight()));
		float py = juce::jmap<float>(y, inputMin, inputMax,
			static_cast<float>(bounds.getBottom()),
			static_cast<float>(bounds.getY()));

		if (i == 0)
			transferPath.startNewSubPath(px, py);
		else
			transferPath.lineTo(px, py);
	}
	g.strokePath(transferPath, juce::PathStrokeType(2.0f));
}

void BasicCompressorAudioProcessorEditor::LevelMeter::paint(juce::Graphics& g)
{
	auto bounds = getLocalBounds().toFloat();
	
	g.fillAll(juce::Colours::black);
	
	float safeLevel = juce::jlimit(-60.0f, 0.0f, levelDb);
	float heightNorm = juce::jmap<float>(safeLevel, -60.0f, 0.0f, 0.0f, 1.0f);
	float barHeight = heightNorm * bounds.getHeight();

	g.setColour(juce::Colours::green);
	if (barHeight > 0.0f)
	{
		g.fillRect(bounds.getX(), bounds.getBottom() - barHeight, bounds.getWidth(), barHeight);
	}

	g.setColour(juce::Colours::white);
	g.setFont(10.0f);

	for (int db = 0; db >= -60; db -= 12) 
	{
		float yPos = juce::jmap<float>((float)db, -60.0f, 0.0f, bounds.getBottom(), bounds.getY());
		g.drawLine(bounds.getX(), yPos, bounds.getX() + 6, yPos, 1.0f);
		
		if (db < 0 && db > -60) {
			g.drawText(juce::String(db), bounds.getX() + 2, yPos - 12, bounds.getWidth(), 12, juce::Justification::centredLeft, false);
		}
	}
	g.setColour(juce::Colours::grey);
	g.drawRect(bounds);
}