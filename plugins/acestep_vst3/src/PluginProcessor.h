#pragma once

#include <JuceHeader.h>

#include "PluginConfig.h"
#include "PluginState.h"

namespace acestep::vst3
{
class ACEStepVST3AudioProcessor final : public juce::AudioProcessor
{
public:
    ACEStepVST3AudioProcessor();
    ~ACEStepVST3AudioProcessor() override;

    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;
    bool isBusesLayoutSupported(const BusesLayout& layouts) const override;
    void processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    const juce::String getName() const override;
    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    bool isSynth() const;
    double getTailLengthSeconds() const override;

    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram(int index) override;
    const juce::String getProgramName(int index) override;
    void changeProgramName(int index, const juce::String& newName) override;
    void getStateInformation(juce::MemoryBlock& destData) override;
    void setStateInformation(const void* data, int sizeInBytes) override;

    const PluginState& getState() const noexcept;
    void setBackendBaseUrl(juce::String baseUrl);
    void setSessionNote(juce::String note);
    juce::String getShellStatusText() const;

private:
    PluginState state_;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ACEStepVST3AudioProcessor)
};
}  // namespace acestep::vst3
