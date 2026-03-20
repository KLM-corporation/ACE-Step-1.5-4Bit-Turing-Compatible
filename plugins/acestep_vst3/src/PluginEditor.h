#pragma once

#include <JuceHeader.h>

#include "PreviewDeckComponent.h"
#include "ResultDeckComponent.h"
#include "StatusStripComponent.h"
#include "SynthPanelComponent.h"
#include "TapeTransportComponent.h"
#include "V2LookAndFeel.h"

namespace acestep::vst3
{
class ACEStepVST3AudioProcessor;

class ACEStepVST3AudioProcessorEditor final : public juce::AudioProcessorEditor,
                                              private juce::Timer
{
public:
    explicit ACEStepVST3AudioProcessorEditor(ACEStepVST3AudioProcessor& processor);
    ~ACEStepVST3AudioProcessorEditor() override;

    void paint(juce::Graphics& g) override;
    void resized() override;

private:
    void timerCallback() override;
    void configureLabels();
    void configureEditors();
    void configureSelectors();
    void syncFromProcessor();
    void persistTextFields();
    void refreshResultSelector();
    void refreshStatusViews();
    void choosePreviewFile();
    void chooseReferenceFile();
    void clearReferenceFile();
    void chooseSourceFile();
    void clearSourceFile();
    void playPreviewFile();
    void stopPreviewFile();
    void clearPreviewFile();
    void revealPreviewFile();

    ACEStepVST3AudioProcessor& processor_;
    std::unique_ptr<V2LookAndFeel> lookAndFeel_;
    StatusStripComponent statusStrip_;
    SynthPanelComponent synthPanel_;
    TapeTransportComponent transport_;
    ResultDeckComponent resultDeck_;
    PreviewDeckComponent previewDeck_;
    std::unique_ptr<juce::FileChooser> previewChooser_;
    std::unique_ptr<juce::FileChooser> referenceChooser_;
    std::unique_ptr<juce::FileChooser> sourceChooser_;
    bool isSyncing_ = false;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ACEStepVST3AudioProcessorEditor)
};
}  // namespace acestep::vst3
