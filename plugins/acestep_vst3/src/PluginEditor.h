#pragma once

#include <JuceHeader.h>

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
    void startMockGeneration();

    ACEStepVST3AudioProcessor& processor_;
    juce::Label titleLabel_;
    juce::Label subtitleLabel_;
    juce::Label backendUrlLabel_;
    juce::Label promptLabel_;
    juce::Label lyricsLabel_;
    juce::Label durationLabel_;
    juce::Label seedLabel_;
    juce::Label modelLabel_;
    juce::Label qualityLabel_;
    juce::Label backendStatusTitle_;
    juce::Label backendStatusValue_;
    juce::Label jobStatusTitle_;
    juce::Label jobStatusValue_;
    juce::Label errorTitle_;
    juce::Label errorValue_;
    juce::Label resultsLabel_;
    juce::TextEditor backendUrlEditor_;
    juce::TextEditor promptEditor_;
    juce::TextEditor lyricsEditor_;
    juce::TextEditor seedEditor_;
    juce::ComboBox durationBox_;
    juce::ComboBox modelBox_;
    juce::ComboBox qualityBox_;
    juce::ComboBox backendStatusBox_;
    juce::ComboBox resultSlotBox_;
    juce::TextButton generateButton_ {"Generate"};
    bool isSyncing_ = false;
    int mockGenerationPhase_ = -1;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ACEStepVST3AudioProcessorEditor)
};
}  // namespace acestep::vst3
