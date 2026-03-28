#include "PluginProcessor.h"
#include "PluginEditor.h"

BasicCompressorAudioProcessorEditor::BasicCompressorAudioProcessorEditor(BasicCompressorAudioProcessor& p)
	: AudioProcessorEditor(&p), audioProcessor(p)
{
	setSize(620, 400);

	// Configuración del Slider de Ganancia
	gainSlider.setSliderStyle(juce::Slider::LinearVertical);
	gainSlider.setRange(-50.0f, 10.0f);
	gainSlider.setSkewFactorFromMidPoint(-10.0f);
	gainSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, true, 100, 25);
	gainSlider.setValue(0.0f);
	gainSlider.addListener(this);
	addAndMakeVisible(gainSlider);

	// Configuración de Textos (Labels)
	thresholdLabel.setText("Threshold", juce::dontSendNotification);
	ratioLabel.setText("Ratio", juce::dontSendNotification);
	attackLabel.setText("Attack", juce::dontSendNotification);
	releaseLabel.setText("Release", juce::dontSendNotification);

	// Centramos los textos de las etiquetas
	thresholdLabel.setJustificationType(juce::Justification::centred);
	ratioLabel.setJustificationType(juce::Justification::centred);
	attackLabel.setJustificationType(juce::Justification::centred);
	releaseLabel.setJustificationType(juce::Justification::centred);

	addAndMakeVisible(thresholdLabel);
	addAndMakeVisible(ratioLabel);
	addAndMakeVisible(attackLabel);
	addAndMakeVisible(releaseLabel);

	// Configuración de los Knobs (Rotary Sliders)
	thresholdSlider.setSliderStyle(juce::Slider::Rotary);
	ratioSlider.setSliderStyle(juce::Slider::Rotary);
	attackSlider.setSliderStyle(juce::Slider::Rotary);
	releaseSlider.setSliderStyle(juce::Slider::Rotary);

	thresholdSlider.setRange(-60.0f, 0.0f);
	thresholdSlider.setValue(-20.0f);
	ratioSlider.setRange(1.0f, 10.0f);
	ratioSlider.setValue(2.0f);
	attackSlider.setRange(0.1f, 100.0f);
	attackSlider.setValue(10.0f);
	releaseSlider.setRange(10.0f, 500.0f);
	releaseSlider.setValue(100.0f);

	gainSlider.setNumDecimalPlacesToDisplay(0);
	thresholdSlider.setNumDecimalPlacesToDisplay(0);
	ratioSlider.setNumDecimalPlacesToDisplay(0);
	releaseSlider.setNumDecimalPlacesToDisplay(0);

	// OJO con el Attack: como tu rango mínimo es 0.1, si le quitas todos los decimales 
	// se mostrará como "0". Te recomiendo dejarle al menos 1 decimal.
	attackSlider.setNumDecimalPlacesToDisplay(2);

	thresholdSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, true, 50, 20);
	ratioSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, true, 50, 20);
	attackSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, true, 50, 20);
	releaseSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, true, 50, 20);

	thresholdSlider.addListener(this);
	ratioSlider.addListener(this);
	attackSlider.addListener(this);
	releaseSlider.addListener(this);

	addAndMakeVisible(thresholdSlider);
	addAndMakeVisible(ratioSlider);
	addAndMakeVisible(attackSlider);
	addAndMakeVisible(releaseSlider);

	// Gráfica y Medidores
	addAndMakeVisible(compressorDisplay);
	addAndMakeVisible(inputLevelMeter);
	addAndMakeVisible(outputLevelMeter);

	// Iniciamos el timer a 30 FPS para los medidores
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

	// Etiquetas centradas encima de los knobs usando el mismo ancho y posición X
	thresholdLabel.setBounds(startX, startY - 20, knobSize, 20);
	thresholdSlider.setBounds(startX, startY, knobSize, knobSize);

	attackLabel.setBounds(startX, startY + verticalSpacing - 20, knobSize, 20);
	attackSlider.setBounds(startX, startY + verticalSpacing, knobSize, knobSize);

	ratioLabel.setBounds(startX, startY + 2 * verticalSpacing - 20, knobSize, 20);
	ratioSlider.setBounds(startX, startY + 2 * verticalSpacing, knobSize, knobSize);

	releaseLabel.setBounds(startX, startY + 3 * verticalSpacing - 20, knobSize, 20);
	releaseSlider.setBounds(startX, startY + 3 * verticalSpacing, knobSize, knobSize);

	compressorDisplay.setBounds(280, 50, 280, 280);
	inputLevelMeter.setBounds(250, 50, 20, 280);
	outputLevelMeter.setBounds(570, 50, 20, 280);
}

void BasicCompressorAudioProcessorEditor::sliderValueChanged(juce::Slider* slider)
{
	if (slider == &gainSlider)
		audioProcessor.volumeDb = gainSlider.getValue();
	else if (slider == &thresholdSlider)
		audioProcessor.compressor.setThreshold(thresholdSlider.getValue());
	else if (slider == &ratioSlider)
		audioProcessor.compressor.setRatio(ratioSlider.getValue());
	else if (slider == &attackSlider)
		audioProcessor.compressor.setAttack(attackSlider.getValue());
	else if (slider == &releaseSlider)
		audioProcessor.compressor.setRelease(releaseSlider.getValue());

	// Actualizamos la curva en tiempo real
	compressorDisplay.setParameters(thresholdSlider.getValue(), ratioSlider.getValue());
}

void BasicCompressorAudioProcessorEditor::timerCallback()
{
	inputLevelMeter.setLevel(audioProcessor.getCurrentInputLevelDb());
	outputLevelMeter.setLevel(audioProcessor.getCurrentOutputLevelDb());
}

// ==============================================================================
// Dibujo de la curva del compresor
// ==============================================================================
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

// ==============================================================================
// Dibujo de los Vúmetros
// ==============================================================================
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