#include "PluginEditor.h"

#include "PluginProcessor.h"
#include "V2Chrome.h"

namespace acestep::vst3
{
namespace
{
constexpr int kEditorWidth = 1080;
constexpr int kEditorHeight = 900;
}  // namespace

ACEStepVST3AudioProcessorEditor::ACEStepVST3AudioProcessorEditor(
    ACEStepVST3AudioProcessor& processor)
    : juce::AudioProcessorEditor(&processor), processor_(processor)
{
    lookAndFeel_ = std::make_unique<V2LookAndFeel>();
    setLookAndFeel(lookAndFeel_.get());
    setSize(kEditorWidth, kEditorHeight);

    for (auto* component : {static_cast<juce::Component*>(&statusStrip_),
                            static_cast<juce::Component*>(&synthPanel_),
                            static_cast<juce::Component*>(&transport_),
                            static_cast<juce::Component*>(&resultDeck_),
                            static_cast<juce::Component*>(&previewDeck_)})
    {
        addAndMakeVisible(*component);
    }

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
    v2::drawChassis(g, getLocalBounds());
}

void ACEStepVST3AudioProcessorEditor::resized()
{
    auto bounds = getLocalBounds().reduced(26);
    statusStrip_.setBounds(bounds.removeFromTop(96));
    bounds.removeFromTop(14);

    auto upper = bounds.removeFromTop(480);
    auto lower = bounds;

    auto synthBounds = upper.removeFromLeft((upper.getWidth() * 11) / 20);
    synthPanel_.setBounds(synthBounds.reduced(0, 2));
    upper.removeFromLeft(14);
    transport_.setBounds(upper.reduced(0, 2));

    auto resultBounds = lower.removeFromTop(150);
    resultDeck_.setBounds(resultBounds);
    lower.removeFromTop(14);
    previewDeck_.setBounds(lower);
}

void ACEStepVST3AudioProcessorEditor::timerCallback()
{
    processor_.pumpBackendWorkflow();
    refreshResultSelector();
    refreshStatusViews();
}
}  // namespace acestep::vst3
