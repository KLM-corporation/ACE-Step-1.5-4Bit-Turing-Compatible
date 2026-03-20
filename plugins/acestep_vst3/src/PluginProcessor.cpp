#include "PluginProcessor.h"

#include "PluginEditor.h"

namespace acestep::vst3
{
ACEStepVST3AudioProcessor::ACEStepVST3AudioProcessor()
    : juce::AudioProcessor(
          BusesProperties().withOutput("Output", juce::AudioChannelSet::stereo(), true))
{
}

ACEStepVST3AudioProcessor::~ACEStepVST3AudioProcessor() = default;

void ACEStepVST3AudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    juce::ignoreUnused(sampleRate, samplesPerBlock);
}

void ACEStepVST3AudioProcessor::releaseResources() {}

bool ACEStepVST3AudioProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const
{
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
    {
        return false;
    }

    return layouts.getMainInputChannelSet().isDisabled();
}

void ACEStepVST3AudioProcessor::processBlock(juce::AudioBuffer<float>& buffer,
                                             juce::MidiBuffer& midiMessages)
{
    juce::ignoreUnused(midiMessages);
    buffer.clear();
}

juce::AudioProcessorEditor* ACEStepVST3AudioProcessor::createEditor()
{
    return new ACEStepVST3AudioProcessorEditor(*this);
}

bool ACEStepVST3AudioProcessor::hasEditor() const
{
    return true;
}

const juce::String ACEStepVST3AudioProcessor::getName() const
{
    return kPluginName;
}

bool ACEStepVST3AudioProcessor::acceptsMidi() const
{
    return true;
}

bool ACEStepVST3AudioProcessor::producesMidi() const
{
    return false;
}

bool ACEStepVST3AudioProcessor::isMidiEffect() const
{
    return false;
}

bool ACEStepVST3AudioProcessor::isSynth() const
{
    return true;
}

double ACEStepVST3AudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int ACEStepVST3AudioProcessor::getNumPrograms()
{
    return 1;
}

int ACEStepVST3AudioProcessor::getCurrentProgram()
{
    return 0;
}

void ACEStepVST3AudioProcessor::setCurrentProgram(int index)
{
    juce::ignoreUnused(index);
}

const juce::String ACEStepVST3AudioProcessor::getProgramName(int index)
{
    juce::ignoreUnused(index);
    return {};
}

void ACEStepVST3AudioProcessor::changeProgramName(int index, const juce::String& newName)
{
    juce::ignoreUnused(index, newName);
}

void ACEStepVST3AudioProcessor::getStateInformation(juce::MemoryBlock& destData)
{
    if (auto xml = createStateXml(state_))
    {
        copyXmlToBinary(*xml, destData);
    }
}

void ACEStepVST3AudioProcessor::setStateInformation(const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xml(juce::getXmlFromBinary(data, sizeInBytes));
    if (xml != nullptr)
    {
        if (auto parsedState = parseStateXml(*xml))
        {
            state_ = *parsedState;
        }
    }
}

const PluginState& ACEStepVST3AudioProcessor::getState() const noexcept
{
    return state_;
}

PluginState& ACEStepVST3AudioProcessor::getMutableState() noexcept
{
    return state_;
}
}  // namespace acestep::vst3

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new acestep::vst3::ACEStepVST3AudioProcessor();
}
