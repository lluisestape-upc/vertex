#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
// VertexLookAndFeel — Polar knob design
// Translated from polar.jsx (plugin-gui-design handoff):
//   white cap, soft drop shadow, double ring frame, 25-tick scale,
//   hairline blue progress arc, thin pin + ringed dot indicator.
// SVG reference viewBox: 200×200, centre 100,100, outer frame r=94.
// All distances use S = min(w,h)*0.45 / 94  as a pixel-per-SVG-unit scale.
//==============================================================================

VertexLookAndFeel::VertexLookAndFeel() {}

void VertexLookAndFeel::drawRotarySlider(juce::Graphics& g, int x, int y, int w, int h,
                                          float sliderPos, float startAngle, float endAngle,
                                          juce::Slider& /*slider*/)
{
    const float cx = x + w * 0.5f;
    const float cy = y + h * 0.5f;
    const float S  = juce::jmin(w, h) * 0.45f / 94.0f; // SVG-to-pixel scale

    const float currentAngle = startAngle + sliderPos * (endAngle - startAngle);
    const float sinA = std::sin(currentAngle);
    const float cosA = std::cos(currentAngle);

    // Pre-computed radii (SVG units × S)
    const float frameR  = 94.0f * S;
    const float trackR  = 70.0f * S;
    const float capR    = 58.0f * S;
    const float innerR  = 44.0f * S;
    const float dotDist = 52.0f * S;   // indicator dot distance from centre
    const float pinOutR = 50.0f * S;   // pin outer end
    const float pinInR  = 30.0f * S;   // pin inner end (inside cap)

    // ---- 1. Outer frame circle ----------------------------------------
    g.setColour(juce::Colour(0xffeef1f6));
    g.drawEllipse(cx - frameR, cy - frameR, frameR * 2.0f, frameR * 2.0f, 0.8f);

    // ---- 2. Tick marks (25, covering the full -135°→+135° sweep) ------
    for (int i = 0; i < 25; ++i)
    {
        const float ta     = startAngle + (i / 24.0f) * (endAngle - startAngle);
        const bool  major  = (i % 6 == 0);
        const float sinT   = std::sin(ta);
        const float cosT   = std::cos(ta);
        const float irTick = major ? 82.0f * S : 86.0f * S;
        g.setColour(major ? juce::Colour(0xff9aa3b8) : juce::Colour(0xffcdd4e0));
        g.drawLine(cx + sinT * 90.0f * S, cy - cosT * 90.0f * S,
                   cx + sinT * irTick,    cy - cosT * irTick,
                   major ? 1.2f : 0.6f);
    }

    // ---- 3. Background track arc --------------------------------------
    {
        juce::Path track;
        track.addArc(cx - trackR, cy - trackR, trackR * 2.0f, trackR * 2.0f,
                     startAngle, endAngle, true);
        g.setColour(juce::Colour(0xffe4e8f0));
        g.strokePath(track, juce::PathStrokeType(2.5f, juce::PathStrokeType::curved,
                                                  juce::PathStrokeType::rounded));
    }

    // ---- 4. Progress arc (electric blue) ------------------------------
    {
        juce::Path active;
        active.addArc(cx - trackR, cy - trackR, trackR * 2.0f, trackR * 2.0f,
                      startAngle, currentAngle, true);
        g.setColour(VertexColours::primary);
        g.strokePath(active, juce::PathStrokeType(2.5f, juce::PathStrokeType::curved,
                                                   juce::PathStrokeType::rounded));
    }

    // ---- 5. Drop shadow under cap (3 soft offset rings) ---------------
    for (int layer = 3; layer >= 1; --layer)
    {
        const float expand = layer * 1.8f;
        g.setColour(juce::Colour(0x0a1530ff).withAlpha(0.025f * layer));
        g.fillEllipse(cx - capR - expand + 2.0f, cy - capR - expand + 3.5f,
                      (capR + expand) * 2.0f, (capR + expand) * 2.0f);
    }

    // ---- 6. Cap: radial-like gradient (white top-left → #e4e8f0) ------
    {
        juce::ColourGradient capGrad(
            juce::Colours::white,        cx - capR * 0.35f, cy - capR * 0.35f,
            juce::Colour(0xffe4e8f0),    cx + capR * 0.35f, cy + capR * 0.35f,
            false);
        capGrad.addColour(0.6, juce::Colour(0xfff8fafd));
        g.setGradientFill(capGrad);
        g.fillEllipse(cx - capR, cy - capR, capR * 2.0f, capR * 2.0f);
    }

    // ---- 7. Bottom-shade overlay (subtle blue-grey tint downward) -----
    {
        juce::ColourGradient shade(
            juce::Colours::transparentWhite,    cx, cy - capR,
            juce::Colour(0x1a788cb4),            cx, cy + capR,
            false);
        g.setGradientFill(shade);
        g.fillEllipse(cx - capR, cy - capR, capR * 2.0f, capR * 2.0f);
    }

    // ---- 8. Cap ring --------------------------------------------------
    g.setColour(juce::Colour(0xffdde2eb));
    g.drawEllipse(cx - capR, cy - capR, capR * 2.0f, capR * 2.0f, 1.0f);

    // ---- 9. Inner secondary ring --------------------------------------
    g.setColour(juce::Colour(0xffeef1f6));
    g.drawEllipse(cx - innerR, cy - innerR, innerR * 2.0f, innerR * 2.0f, 0.8f);

    // ---- 10. Crosshair at centre --------------------------------------
    g.setColour(juce::Colour(0xffdde2eb));
    g.drawLine(cx - 4.0f * S, cy, cx + 4.0f * S, cy, 0.8f);
    g.drawLine(cx, cy - 4.0f * S, cx, cy + 4.0f * S, 0.8f);

    // ---- 11. Indicator: pin + ringed dot ------------------------------
    g.setColour(VertexColours::primary);
    // Thin pin from inner end to outer end (inside cap)
    g.drawLine(cx + sinA * pinInR,  cy - cosA * pinInR,
               cx + sinA * pinOutR, cy - cosA * pinOutR, 1.5f);
    // Dot outer (blue)
    const float dCx = cx + sinA * dotDist;
    const float dCy = cy - cosA * dotDist;
    const float dotR = 4.5f * S;
    g.fillEllipse(dCx - dotR, dCy - dotR, dotR * 2.0f, dotR * 2.0f);
    // Dot inner (white)
    g.setColour(juce::Colours::white);
    const float dotIn = 2.0f * S;
    g.fillEllipse(dCx - dotIn, dCy - dotIn, dotIn * 2.0f, dotIn * 2.0f);
}

juce::Label* VertexLookAndFeel::createSliderTextBox(juce::Slider& slider)
{
    auto* l = LookAndFeel_V4::createSliderTextBox(slider);
    l->setColour(juce::Label::backgroundColourId,      juce::Colours::transparentBlack);
    l->setColour(juce::Label::outlineColourId,          juce::Colours::transparentBlack);
    l->setColour(juce::Label::textColourId,             VertexColours::textDark);
    l->setColour(juce::TextEditor::backgroundColourId,  juce::Colours::white);
    l->setColour(juce::TextEditor::outlineColourId,     VertexColours::primary.withAlpha(0.5f));
    l->setFont(juce::Font(juce::Font::getDefaultMonospacedFontName(), 11.0f, juce::Font::plain));
    return l;
}

//==============================================================================
// Editor constructor
//==============================================================================

BasicCompressorAudioProcessorEditor::BasicCompressorAudioProcessorEditor(BasicCompressorAudioProcessor& p)
    : AudioProcessorEditor(&p), audioProcessor(p)
{
    setSize(500, 470);
    setLookAndFeel(&vertexLF);

    auto setupKnob = [&](juce::Slider& s, int decimals)
    {
        s.setSliderStyle(juce::Slider::Rotary);
        s.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 80, 18);
        s.setNumDecimalPlacesToDisplay(decimals);
        s.setColour(juce::Slider::textBoxBackgroundColourId, juce::Colours::transparentBlack);
        s.setColour(juce::Slider::textBoxOutlineColourId,    juce::Colours::transparentBlack);
        s.setColour(juce::Slider::textBoxTextColourId,       VertexColours::textDark);
        addAndMakeVisible(s);
    };

    setupKnob(gainSlider,      1);
    setupKnob(thresholdSlider, 1);
    setupKnob(ratioSlider,     2);
    setupKnob(attackSlider,    1);
    setupKnob(releaseSlider,   1);

    auto setupLabel = [&](juce::Label& l, const juce::String& text)
    {
        l.setText(text, juce::dontSendNotification);
        l.setJustificationType(juce::Justification::centred);
        l.setColour(juce::Label::textColourId, VertexColours::textMid);
        l.setFont(juce::Font(juce::Font::getDefaultMonospacedFontName(), 9.5f, juce::Font::bold));
        addAndMakeVisible(l);
    };

    setupLabel(gainLabel,      "GAIN");
    setupLabel(thresholdLabel, "THRESH");
    setupLabel(ratioLabel,     "RATIO");
    setupLabel(attackLabel,    "ATTACK");
    setupLabel(releaseLabel,   "RELEASE");

    addAndMakeVisible(compressorDisplay);
    addAndMakeVisible(inputLevelMeter);
    addAndMakeVisible(outputLevelMeter);

    gainAttach    = std::make_unique<SliderAttachment>(audioProcessor.apvts, "gain",      gainSlider);
    threshAttach  = std::make_unique<SliderAttachment>(audioProcessor.apvts, "threshold", thresholdSlider);
    ratioAttach   = std::make_unique<SliderAttachment>(audioProcessor.apvts, "ratio",     ratioSlider);
    attackAttach  = std::make_unique<SliderAttachment>(audioProcessor.apvts, "attack",    attackSlider);
    releaseAttach = std::make_unique<SliderAttachment>(audioProcessor.apvts, "release",   releaseSlider);

    startTimerHz(30);
}

BasicCompressorAudioProcessorEditor::~BasicCompressorAudioProcessorEditor()
{
    setLookAndFeel(nullptr);
}

//==============================================================================
// paint
//==============================================================================

void BasicCompressorAudioProcessorEditor::paint(juce::Graphics& g)
{
    const int W = getWidth();
    const int H = getHeight();

    // ---- Base background ----
    g.fillAll(juce::Colours::white);

    // ---- Subtle Polar coordinate grid reticle ----
    // 20px grid, very faint — matches the Polar design background pattern
    {
        const int step = 20;
        g.setColour(juce::Colour(0xffeef1f6).withAlpha(0.55f));
        for (int gx = 0; gx < W; gx += step)
            g.drawVerticalLine(gx, 0.0f, (float)H);
        for (int gy = 0; gy < H; gy += step)
            g.drawHorizontalLine(gy, 0.0f, (float)W);
    }

    // ---- Knob area tinted background (top section) ----
    g.setColour(VertexColours::knobArea);
    g.fillRect(0, 36, W, 118);

    // ---- Bottom graph background ----
    g.setColour(VertexColours::background);
    g.fillRect(0, 154, W, H - 154);

    // ---- Title bar ----
    g.setFont(juce::Font(juce::Font::getDefaultMonospacedFontName(), 20.0f, juce::Font::bold));
    g.setColour(VertexColours::textDark);
    g.drawText("VERTEX", 14, 4, 160, 28, juce::Justification::centredLeft, false);

    // Blue dot accent
    g.setColour(VertexColours::primary);
    g.fillEllipse(90.0f, 14.0f, 6.0f, 6.0f);

    // Version tag
    g.setFont(juce::Font(juce::Font::getDefaultMonospacedFontName(), 8.0f, juce::Font::plain));
    g.setColour(VertexColours::textMid.withAlpha(0.6f));
    g.drawText("v1.0", W - 40, 8, 34, 16, juce::Justification::centredRight, false);

    // Separator line: title → knob area
    g.setColour(VertexColours::primary);
    g.fillRect(0, 35, W, 1);

    // Thin separator below knob area
    g.setColour(VertexColours::panelLine);
    g.fillRect(0, 153, W, 1);

    // ---- Corner marks on graph panel ----
    const int gx = 30, gy = 158, gw = 440, gh = 268;
    const int markLen = 8;
    g.setColour(VertexColours::primary.withAlpha(0.5f));
    g.drawLine(gx,               gy,           gx + markLen, gy,           1.5f);
    g.drawLine(gx,               gy,           gx,           gy + markLen, 1.5f);
    g.drawLine(gx + gw - markLen,gy,           gx + gw,      gy,           1.5f);
    g.drawLine(gx + gw,          gy,           gx + gw,      gy + markLen, 1.5f);
    g.drawLine(gx,               gy + gh - markLen, gx, gy + gh,           1.5f);
    g.drawLine(gx,               gy + gh,      gx + markLen, gy + gh,      1.5f);
    g.drawLine(gx + gw,          gy + gh - markLen, gx + gw, gy + gh,      1.5f);
    g.drawLine(gx + gw - markLen,gy + gh,      gx + gw,      gy + gh,      1.5f);

    // ---- IN / OUT labels ----
    g.setFont(juce::Font(juce::Font::getDefaultMonospacedFontName(), 9.0f, juce::Font::bold));
    g.setColour(VertexColours::textMid);
    g.drawText("IN",  0,       428, 28, 14, juce::Justification::centred, false);
    g.drawText("OUT", 472, 428, 28, 14, juce::Justification::centred, false);
}

//==============================================================================
// resized
//==============================================================================

void BasicCompressorAudioProcessorEditor::resized()
{
    const int knobSlotW = getWidth() / 5;
    const int labelY    = 38;
    const int labelH    = 13;
    const int knobY     = labelY + labelH;
    const int knobH     = 103;

    for (int i = 0; i < 5; ++i)
    {
        int slotX = i * knobSlotW;
        juce::Label*  lbl = nullptr;
        juce::Slider* sld = nullptr;
        switch (i)
        {
            case 0: lbl = &gainLabel;      sld = &gainSlider;      break;
            case 1: lbl = &thresholdLabel; sld = &thresholdSlider; break;
            case 2: lbl = &ratioLabel;     sld = &ratioSlider;     break;
            case 3: lbl = &attackLabel;    sld = &attackSlider;    break;
            case 4: lbl = &releaseLabel;   sld = &releaseSlider;   break;
            default: break;
        }
        if (lbl) lbl->setBounds(slotX, labelY, knobSlotW, labelH);
        if (sld) sld->setBounds(slotX, knobY,  knobSlotW, knobH);
    }

    const int graphY = 158;
    const int graphH = 268;
    const int meterW = 28;

    inputLevelMeter .setBounds(0,                   graphY, meterW,  graphH);
    compressorDisplay.setBounds(meterW + 2,          graphY, 440,     graphH);
    outputLevelMeter .setBounds(getWidth() - meterW, graphY, meterW,  graphH);
}

//==============================================================================
// timerCallback
//==============================================================================

void BasicCompressorAudioProcessorEditor::timerCallback()
{
    inputLevelMeter .setLevel(audioProcessor.getCurrentInputLevelDb());
    outputLevelMeter.setLevel(audioProcessor.getCurrentOutputLevelDb());
    compressorDisplay.setParameters(thresholdSlider.getValue(), ratioSlider.getValue());
}

//==============================================================================
// CompressorDisplay::paint
//==============================================================================

void BasicCompressorAudioProcessorEditor::CompressorDisplay::paint(juce::Graphics& g)
{
    auto b = getLocalBounds().toFloat();

    // Panel shadow
    g.setColour(juce::Colour(0x1a003088));
    g.fillRoundedRectangle(b.translated(1.5f, 2.5f), 4.0f);

    // White panel
    g.setColour(juce::Colours::white);
    g.fillRoundedRectangle(b, 4.0f);

    // Subtle grid (4×4)
    g.setColour(VertexColours::trackBg);
    for (int i = 1; i < 4; ++i)
    {
        float gx = b.getX() + b.getWidth()  * i / 4.0f;
        float gy = b.getY() + b.getHeight() * i / 4.0f;
        g.drawVerticalLine  (juce::roundToInt(gx), b.getY(),    b.getBottom());
        g.drawHorizontalLine(juce::roundToInt(gy), b.getX(), b.getRight());
    }

    // 1:1 reference diagonal
    g.setColour(VertexColours::light.withAlpha(0.8f));
    g.drawLine(b.getX(), b.getBottom(), b.getRight(), b.getY(), 1.0f);

    constexpr float inputMin = -60.0f, inputMax = 0.0f;

    // Threshold marker
    float threshX = juce::jmap<float>(threshold, inputMin, inputMax, b.getX(), b.getRight());
    g.setColour(VertexColours::mid.withAlpha(0.25f));
    g.drawVerticalLine(juce::roundToInt(threshX), b.getY(), b.getBottom());

    // Transfer curve
    constexpr int pts = 300;
    juce::Path path;
    for (int i = 0; i <= pts; ++i)
    {
        float inDb  = inputMin + (inputMax - inputMin) * i / (float)pts;
        float outDb = compressorTransferFunction(inDb);
        float px = juce::jmap<float>(inDb,  inputMin, inputMax, b.getX(), b.getRight());
        float py = juce::jmap<float>(outDb, inputMin, inputMax, b.getBottom(), b.getY());
        if (i == 0) path.startNewSubPath(px, py);
        else        path.lineTo(px, py);
    }

    // Fill below curve
    {
        juce::Path fill = path;
        fill.lineTo(b.getRight(), b.getBottom());
        fill.lineTo(b.getX(), b.getBottom());
        fill.closeSubPath();
        g.setColour(VertexColours::primary.withAlpha(0.05f));
        g.fillPath(fill);
    }

    // Glow pass
    g.setColour(VertexColours::primary.withAlpha(0.15f));
    g.strokePath(path, juce::PathStrokeType(5.0f, juce::PathStrokeType::curved,
                                             juce::PathStrokeType::rounded));
    // Main curve
    g.setColour(VertexColours::primary);
    g.strokePath(path, juce::PathStrokeType(1.8f, juce::PathStrokeType::curved,
                                             juce::PathStrokeType::rounded));

    // Axis labels
    g.setFont(juce::Font(juce::Font::getDefaultMonospacedFontName(), 9.0f, juce::Font::plain));
    g.setColour(VertexColours::textMid);
    g.drawText("-60", b.getX() + 3,         b.getY() + 3, 28, 11, juce::Justification::centredLeft);
    g.drawText("-30", b.getCentreX() - 14,  b.getY() + 3, 28, 11, juce::Justification::centred);
    g.drawText("0",   b.getRight() - 18,    b.getY() + 3, 16, 11, juce::Justification::centredRight);

    // Panel border
    g.setColour(VertexColours::panelLine);
    g.drawRoundedRectangle(b, 4.0f, 1.0f);
}

//==============================================================================
// LevelMeter::paint
//==============================================================================

void BasicCompressorAudioProcessorEditor::LevelMeter::paint(juce::Graphics& g)
{
    auto b = getLocalBounds().toFloat();

    // Panel shadow
    g.setColour(juce::Colour(0x1a003088));
    g.fillRoundedRectangle(b.translated(1.5f, 2.5f), 3.0f);

    // White panel
    g.setColour(juce::Colours::white);
    g.fillRoundedRectangle(b, 3.0f);

    float innerX = b.getX() + 2.0f;
    float innerW = b.getWidth() - 4.0f;
    float innerTop  = b.getY() + 2.0f;
    float innerBot  = b.getBottom() - 2.0f;
    float innerH    = innerBot - innerTop;

    float norm  = juce::jmap<float>(displayDb, -60.0f, 0.0f, 0.0f, 1.0f);
    float barH  = norm * innerH;
    float barTop = innerBot - barH;

    // Blue gradient bar
    if (barH > 0.5f)
    {
        juce::ColourGradient grad(VertexColours::mid,    innerX + innerW * 0.5f, innerBot,
                                   VertexColours::primary, innerX + innerW * 0.5f, innerTop, false);
        g.setGradientFill(grad);
        g.fillRoundedRectangle(innerX, barTop, innerW, barH, 2.0f);
    }

    // dB tick marks (every 12 dB)
    g.setColour(VertexColours::panelLine.darker(0.4f));
    for (int db = -12; db > -60; db -= 12)
    {
        float yPos = juce::jmap<float>((float)db, -60.0f, 0.0f, innerBot, innerTop);
        g.drawLine(innerX, yPos, innerX + innerW, yPos, 0.7f);
    }

    // 0 dB reference line
    g.setColour(VertexColours::primary.withAlpha(0.6f));
    g.drawLine(innerX, innerTop, innerX + innerW, innerTop, 1.5f);

    // Panel border
    g.setColour(VertexColours::panelLine);
    g.drawRoundedRectangle(b, 3.0f, 1.0f);
}
