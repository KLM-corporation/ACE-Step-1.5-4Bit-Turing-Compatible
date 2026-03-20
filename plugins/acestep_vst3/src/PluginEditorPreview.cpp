#include "PluginEditor.h"

#include "PluginProcessor.h"

namespace acestep::vst3
{
namespace
{
constexpr auto kAudioFileFilter = "*.wav;*.aiff;*.flac;*.ogg;*.mp3";
}

void ACEStepVST3AudioProcessorEditor::choosePreviewFile()
{
    previewChooser_ = std::make_unique<juce::FileChooser>("Select preview audio file",
                                                          juce::File(),
                                                          kAudioFileFilter);
    previewChooser_->launchAsync(juce::FileBrowserComponent::openMode
                                     | juce::FileBrowserComponent::canSelectFiles,
                                 [this](const juce::FileChooser& chooser) {
                                     const auto file = chooser.getResult();
                                     previewChooser_.reset();
                                     if (file == juce::File())
                                     {
                                         return;
                                     }

                                     processor_.stopPreview();
                                     [[maybe_unused]] const auto loaded = processor_.loadPreviewFile(file);
                                     refreshStatusViews();
                                 });
}

void ACEStepVST3AudioProcessorEditor::chooseReferenceFile()
{
    referenceChooser_ = std::make_unique<juce::FileChooser>("Select reference audio file",
                                                            juce::File(),
                                                            kAudioFileFilter);
    referenceChooser_->launchAsync(juce::FileBrowserComponent::openMode
                                       | juce::FileBrowserComponent::canSelectFiles,
                                   [this](const juce::FileChooser& chooser) {
                                       const auto file = chooser.getResult();
                                       referenceChooser_.reset();
                                       if (file == juce::File())
                                       {
                                           return;
                                       }

                                       synthPanel_.referenceAudioEditor().setText(file.getFullPathName(),
                                                                                  juce::sendNotification);
                                       persistTextFields();
                                   });
}

void ACEStepVST3AudioProcessorEditor::clearReferenceFile()
{
    synthPanel_.referenceAudioEditor().clear();
    persistTextFields();
}

void ACEStepVST3AudioProcessorEditor::chooseSourceFile()
{
    sourceChooser_ = std::make_unique<juce::FileChooser>("Select source audio file",
                                                         juce::File(),
                                                         kAudioFileFilter);
    sourceChooser_->launchAsync(juce::FileBrowserComponent::openMode
                                    | juce::FileBrowserComponent::canSelectFiles,
                                [this](const juce::FileChooser& chooser) {
                                    const auto file = chooser.getResult();
                                    sourceChooser_.reset();
                                    if (file == juce::File())
                                    {
                                        return;
                                    }

                                    synthPanel_.sourceAudioEditor().setText(file.getFullPathName(),
                                                                            juce::sendNotification);
                                    persistTextFields();
                                });
}

void ACEStepVST3AudioProcessorEditor::clearSourceFile()
{
    synthPanel_.sourceAudioEditor().clear();
    persistTextFields();
}

void ACEStepVST3AudioProcessorEditor::chooseSessionExportFile()
{
    persistTextFields();
    exportChooser_ = std::make_unique<juce::FileChooser>("Export session summary",
                                                         juce::File("ace-step-session.txt"),
                                                         "*.txt");
    exportChooser_->launchAsync(juce::FileBrowserComponent::saveMode
                                    | juce::FileBrowserComponent::canSelectFiles
                                    | juce::FileBrowserComponent::warnAboutOverwriting,
                                [this](const juce::FileChooser& chooser) {
                                    auto file = chooser.getResult();
                                    exportChooser_.reset();
                                    if (file == juce::File())
                                    {
                                        return;
                                    }

                                    if (!file.hasFileExtension(".txt"))
                                    {
                                        file = file.withFileExtension(".txt");
                                    }

                                    [[maybe_unused]] const auto exported =
                                        processor_.exportSessionSummary(file);
                                    refreshStatusViews();
                                });
}

void ACEStepVST3AudioProcessorEditor::playPreviewFile()
{
    processor_.playPreview();
    refreshStatusViews();
}

void ACEStepVST3AudioProcessorEditor::stopPreviewFile()
{
    processor_.stopPreview();
    refreshStatusViews();
}

void ACEStepVST3AudioProcessorEditor::clearPreviewFile()
{
    processor_.stopPreview();
    processor_.clearPreviewFile();
    refreshStatusViews();
}

void ACEStepVST3AudioProcessorEditor::revealPreviewFile()
{
    processor_.revealPreviewFile();
}
}  // namespace acestep::vst3
