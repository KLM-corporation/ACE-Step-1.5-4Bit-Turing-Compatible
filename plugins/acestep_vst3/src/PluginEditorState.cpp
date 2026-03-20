#include "PluginEditor.h"

#include "PluginEnums.h"
#include "PluginProcessor.h"

namespace acestep::vst3
{
void ACEStepVST3AudioProcessorEditor::configureLabels()
{
    titleLabel_.setText("ACE-STEP", juce::dontSendNotification);
    titleLabel_.setFont(juce::Font(juce::FontOptions(32.0f, juce::Font::bold)));
    subtitleLabel_.setText("Generative instrument for DAW composition and preview playback.",
                           juce::dontSendNotification);
    subtitleLabel_.setFont(juce::Font(juce::FontOptions(14.0f, juce::Font::plain)));

    for (auto* label : {&titleLabel_, &subtitleLabel_, &backendUrlLabel_, &promptLabel_,
                        &lyricsLabel_, &durationLabel_, &seedLabel_, &modelLabel_,
                        &qualityLabel_, &backendStatusTitle_, &backendStatusValue_,
                        &jobStatusTitle_, &jobStatusValue_, &errorTitle_, &errorValue_,
                        &resultsLabel_, &previewTitle_, &previewValue_})
    {
        label->setJustificationType(juce::Justification::centredLeft);
        addAndMakeVisible(*label);
    }

    titleLabel_.setColour(juce::Label::textColourId, juce::Colour::fromRGB(236, 227, 212));
    subtitleLabel_.setColour(juce::Label::textColourId, juce::Colour::fromRGB(176, 163, 147));

    for (auto* label : {&backendUrlLabel_, &promptLabel_, &lyricsLabel_, &durationLabel_,
                        &seedLabel_, &modelLabel_, &qualityLabel_, &backendStatusTitle_,
                        &jobStatusTitle_, &errorTitle_, &resultsLabel_, &previewTitle_})
    {
        label->setFont(juce::Font(juce::FontOptions(13.0f, juce::Font::bold)));
        label->setColour(juce::Label::textColourId, juce::Colour::fromRGB(197, 185, 167));
    }

    for (auto* label : {&backendStatusValue_, &jobStatusValue_, &previewValue_})
    {
        label->setFont(juce::Font(juce::FontOptions(14.0f, juce::Font::plain)));
        label->setColour(juce::Label::textColourId, juce::Colour::fromRGB(228, 219, 207));
    }

    backendUrlLabel_.setText("Signal path", juce::dontSendNotification);
    promptLabel_.setText("Prompt", juce::dontSendNotification);
    lyricsLabel_.setText("Lyrics", juce::dontSendNotification);
    durationLabel_.setText("Duration", juce::dontSendNotification);
    seedLabel_.setText("Seed", juce::dontSendNotification);
    modelLabel_.setText("Engine", juce::dontSendNotification);
    qualityLabel_.setText("Quality", juce::dontSendNotification);
    backendStatusTitle_.setText("Backend", juce::dontSendNotification);
    jobStatusTitle_.setText("Render status", juce::dontSendNotification);
    errorTitle_.setText("Warnings", juce::dontSendNotification);
    resultsLabel_.setText("Selected take", juce::dontSendNotification);
    previewTitle_.setText("Preview deck", juce::dontSendNotification);
    errorValue_.setColour(juce::Label::textColourId, juce::Colour::fromRGB(232, 160, 112));
    errorValue_.setFont(juce::Font(juce::FontOptions(14.0f, juce::Font::plain)));
}

void ACEStepVST3AudioProcessorEditor::configureEditors()
{
    backendUrlEditor_.setFont(juce::Font(juce::FontOptions(15.0f, juce::Font::plain)));
    backendUrlEditor_.setTextToShowWhenEmpty(kDefaultBackendBaseUrl, juce::Colours::grey);
    backendUrlEditor_.setIndents(12, 10);
    backendUrlEditor_.onTextChange = [this] { persistTextFields(); };

    promptEditor_.setFont(juce::Font(juce::FontOptions(15.0f, juce::Font::plain)));
    promptEditor_.setTextToShowWhenEmpty("Describe the song idea", juce::Colours::grey);
    promptEditor_.setIndents(12, 10);
    promptEditor_.onTextChange = [this] { persistTextFields(); };

    lyricsEditor_.setFont(juce::Font(juce::FontOptions(15.0f, juce::Font::plain)));
    lyricsEditor_.setTextToShowWhenEmpty("Optional lyric sketch", juce::Colours::grey);
    lyricsEditor_.setMultiLine(true);
    lyricsEditor_.setIndents(12, 10);
    lyricsEditor_.onTextChange = [this] { persistTextFields(); };

    seedEditor_.setFont(juce::Font(juce::FontOptions(15.0f, juce::Font::plain)));
    seedEditor_.setInputRestrictions(10, "0123456789");
    seedEditor_.setIndents(12, 10);
    seedEditor_.onTextChange = [this] { persistTextFields(); };

    for (auto* editor : {&backendUrlEditor_, &promptEditor_, &lyricsEditor_, &seedEditor_})
    {
        addAndMakeVisible(*editor);
    }
}

void ACEStepVST3AudioProcessorEditor::configureSelectors()
{
    for (const auto duration : {10, 30, 60, 120})
    {
        durationBox_.addItem(juce::String(duration) + " SEC", duration);
    }
    modelBox_.addItem("Turbo", 1);
    modelBox_.addItem("Standard", 2);
    modelBox_.addItem("Quality", 3);
    qualityBox_.addItem("Fast", 1);
    qualityBox_.addItem("Balanced", 2);
    qualityBox_.addItem("Detailed", 3);
    backendStatusBox_.addItem(toString(BackendStatus::ready), 1);
    backendStatusBox_.addItem(toString(BackendStatus::offline), 2);
    backendStatusBox_.addItem(toString(BackendStatus::degraded), 3);
    backendStatusBox_.setEnabled(false);
    generateButton_.setClickingTogglesState(false);

    durationBox_.onChange = [this] { persistTextFields(); };
    modelBox_.onChange = [this] { persistTextFields(); };
    qualityBox_.onChange = [this] { persistTextFields(); };
    resultSlotBox_.onChange = [this] {
        if (isSyncing_)
        {
            return;
        }
        processor_.selectResultSlot(juce::jmax(0, resultSlotBox_.getSelectedItemIndex()));
        refreshStatusViews();
    };
    generateButton_.onClick = [this] {
        persistTextFields();
        processor_.requestGeneration();
        refreshStatusViews();
    };
    choosePreviewButton_.onClick = [this] { choosePreviewFile(); };
    playPreviewButton_.onClick = [this] { playPreviewFile(); };
    stopPreviewButton_.onClick = [this] { stopPreviewFile(); };
    clearPreviewButton_.onClick = [this] { clearPreviewFile(); };
    revealPreviewButton_.onClick = [this] { revealPreviewFile(); };

    for (auto* component : {static_cast<juce::Component*>(&durationBox_),
                            static_cast<juce::Component*>(&modelBox_),
                            static_cast<juce::Component*>(&qualityBox_),
                            static_cast<juce::Component*>(&backendStatusBox_),
                            static_cast<juce::Component*>(&resultSlotBox_),
                            static_cast<juce::Component*>(&generateButton_),
                            static_cast<juce::Component*>(&choosePreviewButton_),
                            static_cast<juce::Component*>(&playPreviewButton_),
                            static_cast<juce::Component*>(&stopPreviewButton_),
                            static_cast<juce::Component*>(&clearPreviewButton_),
                            static_cast<juce::Component*>(&revealPreviewButton_)})
    {
        addAndMakeVisible(*component);
    }
}

void ACEStepVST3AudioProcessorEditor::syncFromProcessor()
{
    const auto& state = processor_.getState();
    isSyncing_ = true;
    backendUrlEditor_.setText(state.backendBaseUrl, juce::dontSendNotification);
    promptEditor_.setText(state.prompt, juce::dontSendNotification);
    lyricsEditor_.setText(state.lyrics, juce::dontSendNotification);
    seedEditor_.setText(juce::String(state.seed), juce::dontSendNotification);
    durationBox_.setSelectedId(state.durationSeconds, juce::dontSendNotification);
    modelBox_.setSelectedId(static_cast<int>(state.modelPreset) + 1, juce::dontSendNotification);
    qualityBox_.setSelectedId(static_cast<int>(state.qualityMode) + 1,
                              juce::dontSendNotification);
    backendStatusBox_.setSelectedId(static_cast<int>(state.backendStatus) + 1,
                                    juce::dontSendNotification);
    isSyncing_ = false;
}

void ACEStepVST3AudioProcessorEditor::persistTextFields()
{
    if (isSyncing_)
    {
        return;
    }

    auto& state = processor_.getMutableState();
    state.backendBaseUrl = backendUrlEditor_.getText().trim();
    if (state.backendBaseUrl.isEmpty())
    {
        state.backendBaseUrl = kDefaultBackendBaseUrl;
        backendUrlEditor_.setText(state.backendBaseUrl, juce::dontSendNotification);
    }

    state.prompt = promptEditor_.getText();
    state.lyrics = lyricsEditor_.getText();
    state.durationSeconds = durationBox_.getSelectedId() == 0 ? kDefaultDurationSeconds
                                                              : durationBox_.getSelectedId();
    state.seed = seedEditor_.getText().getIntValue();
    if (state.seed <= 0)
    {
        state.seed = kDefaultSeed;
        seedEditor_.setText(juce::String(state.seed), juce::dontSendNotification);
    }

    state.modelPreset = static_cast<ModelPreset>(juce::jmax(0, modelBox_.getSelectedItemIndex()));
    state.qualityMode = static_cast<QualityMode>(juce::jmax(0, qualityBox_.getSelectedItemIndex()));
    refreshStatusViews();
}

void ACEStepVST3AudioProcessorEditor::refreshResultSelector()
{
    const auto& state = processor_.getState();
    isSyncing_ = true;
    resultSlotBox_.clear(juce::dontSendNotification);
    for (int index = 0; index < kResultSlotCount; ++index)
    {
        auto label = state.resultSlots[static_cast<size_t>(index)];
        if (label.isEmpty())
        {
            label = "Result " + juce::String(index + 1) + " - empty";
        }
        resultSlotBox_.addItem(label, index + 1);
    }
    resultSlotBox_.setSelectedId(state.selectedResultSlot + 1, juce::dontSendNotification);
    isSyncing_ = false;
}

void ACEStepVST3AudioProcessorEditor::refreshStatusViews()
{
    const auto& state = processor_.getState();
    backendStatusBox_.setSelectedId(static_cast<int>(state.backendStatus) + 1,
                                    juce::dontSendNotification);
    backendStatusValue_.setText(
        "Target  " + state.backendBaseUrl + "\nState    " + toString(state.backendStatus),
        juce::dontSendNotification);
    jobStatusValue_.setText("Job      " + toString(state.jobStatus) + "\nTake     "
                                + juce::String(state.selectedResultSlot + 1) + "\nMessage  "
                                + (state.progressText.isEmpty() ? "Idle" : state.progressText),
                            juce::dontSendNotification);
    errorValue_.setText(state.errorMessage.isEmpty() ? "No active error." : state.errorMessage,
                        juce::dontSendNotification);
    auto previewText = state.previewDisplayName.isEmpty() ? "No preview file loaded."
                                                          : state.previewDisplayName;
    previewText += "\n";
    previewText += state.previewFilePath.isEmpty() ? "Choose a local audio file to preview or reveal."
                                                   : state.previewFilePath;
    if (processor_.isPreviewPlaying())
    {
        previewText += "\nDeck: playback active";
    }
    previewValue_.setText(previewText, juce::dontSendNotification);
    generateButton_.setEnabled(state.jobStatus != JobStatus::submitting
                               && state.jobStatus != JobStatus::queuedOrRunning);
    generateButton_.setButtonText(state.jobStatus == JobStatus::queuedOrRunning ? "Rendering..."
                                                                                : "Generate");
}
}  // namespace acestep::vst3
