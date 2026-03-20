#include "SynthPanelComponent.h"

#include "V2Chrome.h"

namespace acestep::vst3
{
SynthPanelComponent::SynthPanelComponent()
{
    for (auto* label : {&backendUrlLabel_, &promptLabel_, &lyricsLabel_, &modeLabel_,
                        &referenceAudioLabel_, &sourceAudioLabel_, &conditioningCodesLabel_,
                        &coverStrengthLabel_, &durationLabel_, &seedLabel_, &modelLabel_,
                        &qualityLabel_})
    {
        label->setColour(juce::Label::textColourId, v2::kLabelMuted);
        addAndMakeVisible(*label);
    }

    backendUrlLabel_.setText("Signal Path", juce::dontSendNotification);
    modeLabel_.setText("Workflow Mode", juce::dontSendNotification);
    promptLabel_.setText("Prompt", juce::dontSendNotification);
    lyricsLabel_.setText("Lyrics", juce::dontSendNotification);
    referenceAudioLabel_.setText("Reference Audio", juce::dontSendNotification);
    sourceAudioLabel_.setText("Source Audio", juce::dontSendNotification);
    conditioningCodesLabel_.setText("Conditioning Codes", juce::dontSendNotification);
    coverStrengthLabel_.setText("Conditioning Strength", juce::dontSendNotification);
    durationLabel_.setText("Duration", juce::dontSendNotification);
    seedLabel_.setText("Seed", juce::dontSendNotification);
    modelLabel_.setText("Engine", juce::dontSendNotification);
    qualityLabel_.setText("Quality", juce::dontSendNotification);

    promptEditor_.setMultiLine(true);
    lyricsEditor_.setMultiLine(true);
    conditioningCodesEditor_.setMultiLine(true);
    coverStrengthSlider_.setSliderStyle(juce::Slider::LinearHorizontal);
    coverStrengthSlider_.setTextBoxStyle(juce::Slider::TextBoxRight, false, 64, 22);
    coverStrengthSlider_.setRange(0.0, 1.0, 0.05);

    for (auto* component : {static_cast<juce::Component*>(&backendUrlEditor_),
                            static_cast<juce::Component*>(&modeBox_),
                            static_cast<juce::Component*>(&promptEditor_),
                            static_cast<juce::Component*>(&lyricsEditor_),
                            static_cast<juce::Component*>(&referenceAudioEditor_),
                            static_cast<juce::Component*>(&sourceAudioEditor_),
                            static_cast<juce::Component*>(&conditioningCodesEditor_),
                            static_cast<juce::Component*>(&seedEditor_),
                            static_cast<juce::Component*>(&durationBox_),
                            static_cast<juce::Component*>(&modelBox_),
                            static_cast<juce::Component*>(&qualityBox_),
                            static_cast<juce::Component*>(&coverStrengthSlider_),
                            static_cast<juce::Component*>(&chooseReferenceButton_),
                            static_cast<juce::Component*>(&clearReferenceButton_),
                            static_cast<juce::Component*>(&chooseSourceButton_),
                            static_cast<juce::Component*>(&clearSourceButton_)})
    {
        addAndMakeVisible(*component);
    }
}

void SynthPanelComponent::paint(juce::Graphics& g)
{
    v2::drawModule(g, getLocalBounds(), "Compose / Engine", v2::kAccentBlue);
}

void SynthPanelComponent::resized()
{
    auto area = getLocalBounds().reduced(18);
    area.removeFromTop(24);
    auto left = area.removeFromLeft(area.getWidth() / 2 + 18);
    auto right = area;
    const auto labelHeight = 18;
    const auto fieldHeight = 32;

    backendUrlLabel_.setBounds(left.removeFromTop(labelHeight));
    backendUrlEditor_.setBounds(left.removeFromTop(fieldHeight));
    left.removeFromTop(8);
    modeLabel_.setBounds(left.removeFromTop(labelHeight));
    modeBox_.setBounds(left.removeFromTop(fieldHeight));
    left.removeFromTop(8);
    promptLabel_.setBounds(left.removeFromTop(labelHeight));
    promptEditor_.setBounds(left.removeFromTop(72));
    left.removeFromTop(8);
    lyricsLabel_.setBounds(left.removeFromTop(labelHeight));
    lyricsEditor_.setBounds(left.removeFromTop(90));

    referenceAudioLabel_.setBounds(right.removeFromTop(labelHeight));
    referenceAudioEditor_.setBounds(right.removeFromTop(fieldHeight));
    auto referenceButtons = right.removeFromTop(26);
    chooseReferenceButton_.setBounds(referenceButtons.removeFromLeft(82));
    referenceButtons.removeFromLeft(8);
    clearReferenceButton_.setBounds(referenceButtons.removeFromLeft(70));
    right.removeFromTop(8);
    sourceAudioLabel_.setBounds(right.removeFromTop(labelHeight));
    sourceAudioEditor_.setBounds(right.removeFromTop(fieldHeight));
    auto sourceButtons = right.removeFromTop(26);
    chooseSourceButton_.setBounds(sourceButtons.removeFromLeft(82));
    sourceButtons.removeFromLeft(8);
    clearSourceButton_.setBounds(sourceButtons.removeFromLeft(70));
    right.removeFromTop(8);
    conditioningCodesLabel_.setBounds(right.removeFromTop(labelHeight));
    conditioningCodesEditor_.setBounds(right.removeFromTop(64));
    right.removeFromTop(8);
    coverStrengthLabel_.setBounds(right.removeFromTop(labelHeight));
    coverStrengthSlider_.setBounds(right.removeFromTop(24));
    right.removeFromTop(8);
    durationLabel_.setBounds(right.removeFromTop(labelHeight));
    durationBox_.setBounds(right.removeFromTop(fieldHeight));
    right.removeFromTop(8);
    seedLabel_.setBounds(right.removeFromTop(labelHeight));
    seedEditor_.setBounds(right.removeFromTop(fieldHeight));
    right.removeFromTop(8);
    modelLabel_.setBounds(right.removeFromTop(labelHeight));
    modelBox_.setBounds(right.removeFromTop(fieldHeight));
    right.removeFromTop(8);
    qualityLabel_.setBounds(right.removeFromTop(labelHeight));
    qualityBox_.setBounds(right.removeFromTop(fieldHeight));
}

juce::TextEditor& SynthPanelComponent::backendUrlEditor() noexcept { return backendUrlEditor_; }
juce::TextEditor& SynthPanelComponent::promptEditor() noexcept { return promptEditor_; }
juce::TextEditor& SynthPanelComponent::lyricsEditor() noexcept { return lyricsEditor_; }
juce::TextEditor& SynthPanelComponent::referenceAudioEditor() noexcept { return referenceAudioEditor_; }
juce::TextEditor& SynthPanelComponent::sourceAudioEditor() noexcept { return sourceAudioEditor_; }
juce::TextEditor& SynthPanelComponent::conditioningCodesEditor() noexcept
{
    return conditioningCodesEditor_;
}
juce::TextEditor& SynthPanelComponent::seedEditor() noexcept { return seedEditor_; }
juce::ComboBox& SynthPanelComponent::modeBox() noexcept { return modeBox_; }
juce::ComboBox& SynthPanelComponent::durationBox() noexcept { return durationBox_; }
juce::ComboBox& SynthPanelComponent::modelBox() noexcept { return modelBox_; }
juce::ComboBox& SynthPanelComponent::qualityBox() noexcept { return qualityBox_; }
juce::Slider& SynthPanelComponent::coverStrengthSlider() noexcept { return coverStrengthSlider_; }
juce::TextButton& SynthPanelComponent::chooseReferenceButton() noexcept
{
    return chooseReferenceButton_;
}
juce::TextButton& SynthPanelComponent::clearReferenceButton() noexcept
{
    return clearReferenceButton_;
}
juce::TextButton& SynthPanelComponent::chooseSourceButton() noexcept { return chooseSourceButton_; }
juce::TextButton& SynthPanelComponent::clearSourceButton() noexcept { return clearSourceButton_; }
}  // namespace acestep::vst3
