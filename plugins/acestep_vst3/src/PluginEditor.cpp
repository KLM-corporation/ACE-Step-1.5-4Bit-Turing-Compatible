#include "PluginEditor.h"

#include "PluginProcessor.h"

namespace acestep::vst3
{
namespace
{
constexpr int kEditorWidth = 940;
constexpr int kEditorHeight = 700;

const juce::Colour kShellOuter = juce::Colour::fromRGB(27, 25, 24);
const juce::Colour kShellInner = juce::Colour::fromRGB(60, 56, 52);
const juce::Colour kShellHighlight = juce::Colour::fromRGB(88, 81, 74);
const juce::Colour kPanelFill = juce::Colour::fromRGB(29, 28, 27);
const juce::Colour kPanelInset = juce::Colour::fromRGB(22, 21, 20);
const juce::Colour kPanelStroke = juce::Colour::fromRGB(109, 99, 90);
const juce::Colour kTextPrimary = juce::Colour::fromRGB(234, 224, 208);
const juce::Colour kTextMuted = juce::Colour::fromRGB(176, 163, 147);
const juce::Colour kTextDim = juce::Colour::fromRGB(130, 119, 107);
const juce::Colour kAccent = juce::Colour::fromRGB(235, 164, 92);
const juce::Colour kAccentWarm = juce::Colour::fromRGB(214, 108, 94);
const juce::Colour kAccentOlive = juce::Colour::fromRGB(126, 139, 99);
const juce::Colour kDisplayFill = juce::Colour::fromRGB(34, 41, 30);
const juce::Colour kDisplayGlow = juce::Colour::fromRGB(188, 214, 156);

class PluginLookAndFeel final : public juce::LookAndFeel_V4
{
public:
    PluginLookAndFeel()
    {
        setColour(juce::Label::textColourId, kTextPrimary);
        setColour(juce::TextEditor::backgroundColourId, kPanelInset);
        setColour(juce::TextEditor::outlineColourId, juce::Colour::fromRGB(87, 79, 71));
        setColour(juce::TextEditor::focusedOutlineColourId, kAccent);
        setColour(juce::TextEditor::textColourId, kTextPrimary);
        setColour(juce::TextEditor::highlightColourId, kAccent.withAlpha(0.22f));
        setColour(juce::CaretComponent::caretColourId, kAccent);
        setColour(juce::ComboBox::backgroundColourId, kPanelInset);
        setColour(juce::ComboBox::outlineColourId, juce::Colour::fromRGB(89, 80, 73));
        setColour(juce::ComboBox::arrowColourId, kTextMuted);
        setColour(juce::ComboBox::textColourId, kTextPrimary);
        setColour(juce::PopupMenu::backgroundColourId, juce::Colour::fromRGB(30, 28, 27));
        setColour(juce::PopupMenu::textColourId, kTextPrimary);
    }

    juce::Font getTextButtonFont(juce::TextButton&, int buttonHeight) override
    {
        return juce::Font(juce::FontOptions(static_cast<float>(juce::jmin(18, buttonHeight / 2 + 3)),
                                            juce::Font::bold));
    }

    void drawButtonBackground(juce::Graphics& g,
                              juce::Button& button,
                              const juce::Colour&,
                              bool isMouseOverButton,
                              bool isButtonDown) override
    {
        auto bounds = button.getLocalBounds().toFloat().reduced(0.5f);
        auto base = button.getToggleState() ? kAccentOlive : juce::Colour::fromRGB(78, 69, 60);
        if (button.getButtonText().containsIgnoreCase("Generate")
            || button.getButtonText().containsIgnoreCase("Rendering"))
        {
            base = kAccent;
        }
        if (!button.isEnabled())
        {
            base = base.darker(0.6f);
        }
        if (isMouseOverButton)
        {
            base = base.brighter(0.18f);
        }
        if (isButtonDown)
        {
            base = base.darker(0.28f);
        }

        g.setGradientFill(juce::ColourGradient(base.brighter(0.1f),
                                               bounds.getTopLeft(),
                                               base.darker(0.25f),
                                               bounds.getBottomLeft(),
                                               false));
        g.fillRoundedRectangle(bounds, 8.0f);
        g.setColour(juce::Colours::black.withAlpha(0.35f));
        g.drawRoundedRectangle(bounds.translated(0.0f, 1.0f), 8.0f, 2.0f);
        g.setColour(button.getButtonText().containsIgnoreCase("Generate") ? kAccentWarm.withAlpha(0.45f)
                                                                          : kPanelStroke.withAlpha(0.65f));
        g.drawRoundedRectangle(bounds, 8.0f, 1.0f);
    }

    void drawComboBox(juce::Graphics& g,
                      int width,
                      int height,
                      bool,
                      int,
                      int,
                      int,
                      int,
                      juce::ComboBox& box) override
    {
        auto bounds = juce::Rectangle<float>(0.0f, 0.0f, static_cast<float>(width), static_cast<float>(height))
                          .reduced(0.5f);
        g.setGradientFill(juce::ColourGradient(kPanelInset.brighter(0.06f),
                                               bounds.getTopLeft(),
                                               kPanelInset,
                                               bounds.getBottomLeft(),
                                               false));
        g.fillRoundedRectangle(bounds, 7.0f);
        g.setColour(juce::Colours::black.withAlpha(0.42f));
        g.drawRoundedRectangle(bounds.translated(0.0f, 1.0f), 7.0f, 1.2f);
        g.setColour(box.isEnabled() ? kPanelStroke : kTextDim);
        g.drawRoundedRectangle(bounds, 7.0f, 1.0f);

        auto arrowZone = bounds.removeFromRight(26.0f).reduced(4.0f, 7.0f);
        juce::Path arrow;
        arrow.startNewSubPath(arrowZone.getX(), arrowZone.getY());
        arrow.lineTo(arrowZone.getCentreX(), arrowZone.getBottom());
        arrow.lineTo(arrowZone.getRight(), arrowZone.getY());
        g.setColour(kTextMuted);
        g.strokePath(arrow, juce::PathStrokeType(1.6f));
    }

    juce::Font getComboBoxFont(juce::ComboBox& box) override
    {
        return juce::Font(juce::FontOptions(static_cast<float>(juce::jmin(18, box.getHeight() / 2 + 2)),
                                            juce::Font::plain));
    }

    void drawTextEditorOutline(juce::Graphics& g,
                               int width,
                               int height,
                               juce::TextEditor& textEditor) override
    {
        auto bounds = juce::Rectangle<float>(0.0f, 0.0f, static_cast<float>(width), static_cast<float>(height))
                          .reduced(0.5f);
        g.setColour(juce::Colours::black.withAlpha(0.45f));
        g.drawRoundedRectangle(bounds.translated(0.0f, 1.0f), 7.0f, 1.5f);
        g.setColour(textEditor.hasKeyboardFocus(true) ? kAccent : kPanelStroke);
        g.drawRoundedRectangle(bounds, 7.0f, textEditor.hasKeyboardFocus(true) ? 1.3f : 1.0f);
    }
};

void drawScrew(juce::Graphics& g, juce::Point<float> centre)
{
    g.setColour(juce::Colour::fromRGB(98, 91, 84));
    g.fillEllipse(juce::Rectangle<float>(10.0f, 10.0f).withCentre(centre));
    g.setColour(juce::Colour::fromRGB(44, 40, 38));
    g.drawEllipse(juce::Rectangle<float>(10.0f, 10.0f).withCentre(centre), 1.0f);
    g.drawLine(centre.x - 2.5f, centre.y, centre.x + 2.5f, centre.y, 1.0f);
    g.drawLine(centre.x, centre.y - 2.5f, centre.x, centre.y + 2.5f, 1.0f);
}

void drawStatusLamp(juce::Graphics& g, juce::Rectangle<float> bounds, juce::Colour colour, bool active)
{
    const auto base = active ? colour : juce::Colour::fromRGB(66, 59, 52);
    g.setColour(base.darker(0.35f));
    g.fillEllipse(bounds);
    g.setColour(base.brighter(active ? 0.3f : 0.05f));
    g.fillEllipse(bounds.reduced(2.0f));
    if (active)
    {
        g.setColour(base.withAlpha(0.2f));
        g.fillEllipse(bounds.expanded(5.0f));
    }
}

void drawPanel(juce::Graphics& g,
               juce::Rectangle<int> bounds,
               const juce::String& title,
               const juce::Colour& accent,
               float glowAlpha = 0.08f)
{
    auto area = bounds.toFloat();
    g.setColour(accent.withAlpha(glowAlpha));
    g.fillRoundedRectangle(area.expanded(1.0f), 11.0f);
    g.setGradientFill(juce::ColourGradient(kPanelFill.brighter(0.06f),
                                           area.getTopLeft(),
                                           kPanelFill,
                                           area.getBottomLeft(),
                                           false));
    g.fillRoundedRectangle(area, 10.0f);
    g.setColour(juce::Colours::black.withAlpha(0.28f));
    g.drawRoundedRectangle(area.translated(0.0f, 1.2f), 10.0f, 2.0f);
    g.setColour(kPanelStroke);
    g.drawRoundedRectangle(area, 10.0f, 1.0f);

    auto titleBar = area.removeFromTop(26.0f);
    g.setColour(accent.withAlpha(0.14f));
    g.fillRoundedRectangle(titleBar.reduced(12.0f, 4.0f), 6.0f);
    g.setColour(kTextPrimary);
    g.setFont(juce::Font(juce::FontOptions(12.5f, juce::Font::bold)));
    g.drawText(title.toUpperCase(), titleBar.reduced(18.0f, 0.0f), juce::Justification::centredLeft);
}
}  // namespace

ACEStepVST3AudioProcessorEditor::ACEStepVST3AudioProcessorEditor(
    ACEStepVST3AudioProcessor& processor)
    : juce::AudioProcessorEditor(&processor), processor_(processor)
{
    lookAndFeel_ = std::make_unique<PluginLookAndFeel>();
    setLookAndFeel(lookAndFeel_.get());
    setSize(kEditorWidth, kEditorHeight);
    configureLabels();
    configureEditors();
    configureSelectors();
    syncFromProcessor();
    refreshResultSelector();
    refreshStatusViews();
    startTimerHz(4);
}

ACEStepVST3AudioProcessorEditor::~ACEStepVST3AudioProcessorEditor()
{
    setLookAndFeel(nullptr);
}

void ACEStepVST3AudioProcessorEditor::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat();
    g.fillAll(kShellOuter);

    g.setGradientFill(juce::ColourGradient(kShellHighlight,
                                           bounds.getTopLeft(),
                                           kShellOuter,
                                           bounds.getBottomRight(),
                                           false));
    g.fillRoundedRectangle(bounds.reduced(8.0f), 24.0f);

    for (int x = 20; x < getWidth() - 20; x += 6)
    {
        g.setColour(((x / 6) % 2 == 0 ? juce::Colours::white : juce::Colours::black).withAlpha(0.015f));
        g.drawVerticalLine(x, 10.0f, static_cast<float>(getHeight() - 10));
    }

    g.setColour(juce::Colours::black.withAlpha(0.32f));
    g.drawRoundedRectangle(bounds.reduced(8.0f), 24.0f, 2.0f);

    auto layout = getLocalBounds().reduced(20);
    auto header = layout.removeFromTop(76);
    auto meterStrip = header.removeFromTop(24).reduced(10, 3);
    g.setColour(kDisplayFill);
    g.fillRoundedRectangle(meterStrip.toFloat(), 8.0f);
    g.setColour(kDisplayGlow.withAlpha(0.15f));
    g.fillRoundedRectangle(meterStrip.removeFromLeft(190).toFloat(), 8.0f);
    g.setColour(kDisplayGlow);
    g.setFont(juce::Font(juce::FontOptions(11.5f, juce::Font::bold)));
    g.drawText("ACE-STEP 1.5   TEXT-TO-MUSIC INSTRUMENT", meterStrip.reduced(12, 0),
               juce::Justification::centredLeft);

    auto brandLine = header.removeFromBottom(18);
    g.setColour(kTextDim);
    g.setFont(juce::Font(juce::FontOptions(11.0f, juce::Font::bold)));
    g.drawText("INSPIRED BY VINTAGE DESKTOP SYNTHS", brandLine, juce::Justification::centredRight);
    drawStatusLamp(g, juce::Rectangle<float>(12.0f, 12.0f).withCentre(
                          juce::Point<float>(static_cast<float>(getWidth() - 42), 49.0f)),
                   kAccentOlive, true);

    auto rowOne = layout.removeFromTop(244);
    auto leftPanel = rowOne.removeFromLeft(408);
    auto rightPanel = rowOne;
    auto rowTwo = layout.removeFromTop(176);
    auto monitorPanel = rowTwo.removeFromLeft(430);
    auto actionPanel = rowTwo;
    auto rowThree = layout;

    drawPanel(g, leftPanel.reduced(6), "Prompt programmer", kAccentOlive);
    drawPanel(g, rightPanel.reduced(6), "Voice architecture", kAccent);
    drawPanel(g, monitorPanel.reduced(6), "Machine monitor", kAccentOlive);
    drawPanel(g, actionPanel.reduced(6), "Render transport", kAccentWarm);
    drawPanel(g, rowThree.reduced(6), "Preview deck", kAccent);

    drawScrew(g, {26.0f, 26.0f});
    drawScrew(g, {static_cast<float>(getWidth() - 26), 26.0f});
    drawScrew(g, {26.0f, static_cast<float>(getHeight() - 26)});
    drawScrew(g, {static_cast<float>(getWidth() - 26), static_cast<float>(getHeight() - 26)});
}

void ACEStepVST3AudioProcessorEditor::resized()
{
    auto bounds = getLocalBounds().reduced(26);

    auto header = bounds.removeFromTop(76);
    titleLabel_.setBounds(header.removeFromTop(36));
    subtitleLabel_.setBounds(header.removeFromTop(22));

    auto topRow = bounds.removeFromTop(244);
    auto promptPanel = topRow.removeFromLeft(408).reduced(18, 18);
    topRow.removeFromLeft(8);
    auto controlPanel = topRow.reduced(18, 18);

    auto monitorRow = bounds.removeFromTop(176);
    auto statusPanel = monitorRow.removeFromLeft(430).reduced(18, 18);
    monitorRow.removeFromLeft(8);
    auto renderPanel = monitorRow.reduced(18, 18);

    auto previewPanel = bounds.reduced(18, 18);

    auto promptTop = promptPanel;
    backendUrlLabel_.setBounds(promptTop.removeFromTop(18));
    promptTop.removeFromTop(6);
    backendUrlEditor_.setBounds(promptTop.removeFromTop(32));
    promptTop.removeFromTop(12);
    promptLabel_.setBounds(promptTop.removeFromTop(18));
    promptTop.removeFromTop(6);
    promptEditor_.setBounds(promptTop.removeFromTop(46));
    promptTop.removeFromTop(12);
    lyricsLabel_.setBounds(promptTop.removeFromTop(18));
    promptTop.removeFromTop(6);
    lyricsEditor_.setBounds(promptTop.removeFromTop(78));

    auto controlsTop = controlPanel;
    auto controlGridTop = controlsTop.removeFromTop(112);
    auto knobRowA = controlGridTop.removeFromTop(50);
    auto knobA1 = knobRowA.removeFromLeft((knobRowA.getWidth() - 12) / 2);
    knobRowA.removeFromLeft(12);
    auto knobA2 = knobRowA;
    durationLabel_.setBounds(knobA1.removeFromTop(16));
    knobA1.removeFromTop(6);
    durationBox_.setBounds(knobA1);
    seedLabel_.setBounds(knobA2.removeFromTop(16));
    knobA2.removeFromTop(6);
    seedEditor_.setBounds(knobA2);

    controlsTop.removeFromTop(10);
    auto knobRowB = controlsTop.removeFromTop(50);
    auto knobB1 = knobRowB.removeFromLeft((knobRowB.getWidth() - 12) / 2);
    knobRowB.removeFromLeft(12);
    auto knobB2 = knobRowB;
    modelLabel_.setBounds(knobB1.removeFromTop(16));
    knobB1.removeFromTop(6);
    modelBox_.setBounds(knobB1);
    qualityLabel_.setBounds(knobB2.removeFromTop(16));
    knobB2.removeFromTop(6);
    qualityBox_.setBounds(knobB2);

    controlsTop.removeFromTop(12);
    resultsLabel_.setBounds(controlsTop.removeFromTop(18));
    controlsTop.removeFromTop(6);
    resultSlotBox_.setBounds(controlsTop.removeFromTop(32));

    auto statusLeft = statusPanel;
    backendStatusTitle_.setBounds(statusLeft.removeFromTop(18));
    statusLeft.removeFromTop(6);
    backendStatusBox_.setBounds(statusLeft.removeFromTop(30));
    statusLeft.removeFromTop(10);
    backendStatusValue_.setBounds(statusLeft.removeFromTop(48));
    statusLeft.removeFromTop(10);
    jobStatusTitle_.setBounds(statusLeft.removeFromTop(18));
    statusLeft.removeFromTop(6);
    jobStatusValue_.setBounds(statusLeft.removeFromTop(54));

    auto actionTop = renderPanel;
    errorTitle_.setBounds(actionTop.removeFromTop(18));
    actionTop.removeFromTop(6);
    errorValue_.setBounds(actionTop.removeFromTop(70));
    actionTop.removeFromTop(12);
    generateButton_.setBounds(actionTop.removeFromTop(42).withTrimmedLeft(20).withTrimmedRight(110));

    auto previewTop = previewPanel;
    previewTitle_.setBounds(previewTop.removeFromTop(18));
    previewTop.removeFromTop(8);
    previewValue_.setBounds(previewTop.removeFromTop(78));
    previewTop.removeFromTop(12);

    auto wideButtons = previewTop.removeFromTop(42);
    choosePreviewButton_.setBounds(wideButtons.removeFromLeft(210));
    wideButtons.removeFromLeft(12);
    revealPreviewButton_.setBounds(wideButtons.removeFromLeft(160));

    previewTop.removeFromTop(12);
    auto transportButtons = previewTop.removeFromTop(42);
    playPreviewButton_.setBounds(transportButtons.removeFromLeft(110));
    transportButtons.removeFromLeft(12);
    stopPreviewButton_.setBounds(transportButtons.removeFromLeft(110));
    transportButtons.removeFromLeft(12);
    clearPreviewButton_.setBounds(transportButtons.removeFromLeft(110));
}

void ACEStepVST3AudioProcessorEditor::timerCallback()
{
    processor_.pumpBackendWorkflow();
    refreshResultSelector();
    refreshStatusViews();
}
}  // namespace acestep::vst3
