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
    preview_.prepareToPlay(sampleRate, samplesPerBlock);
}

void ACEStepVST3AudioProcessor::releaseResources()
{
    preview_.releaseResources();
}

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
    preview_.render(buffer);
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
    std::unique_ptr<juce::XmlElement> xml(getXmlFromBinary(data, sizeInBytes));
    if (xml != nullptr)
    {
        if (auto parsedState = parseStateXml(*xml))
        {
            state_ = *parsedState;
            syncPreviewFromState();
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

bool ACEStepVST3AudioProcessor::loadPreviewFile(const juce::File& file)
{
    juce::String errorMessage;
    if (!preview_.loadFile(file, errorMessage))
    {
        state_.errorMessage = errorMessage;
        return false;
    }

    state_.previewFilePath = file.getFullPathName();
    state_.previewDisplayName = file.getFileName();
    state_.errorMessage = {};
    return true;
}

void ACEStepVST3AudioProcessor::clearPreviewFile()
{
    preview_.clear();
    state_.previewFilePath = {};
    state_.previewDisplayName = {};
    state_.errorMessage = {};
}

void ACEStepVST3AudioProcessor::playPreview()
{
    if (!hasPreviewFile())
    {
        state_.errorMessage = "Load a preview file before playing it.";
        return;
    }

    state_.errorMessage = {};
    preview_.play();
}

void ACEStepVST3AudioProcessor::stopPreview()
{
    preview_.stop();
}

void ACEStepVST3AudioProcessor::revealPreviewFile() const
{
    preview_.revealToUser();
}

bool ACEStepVST3AudioProcessor::hasPreviewFile() const
{
    return preview_.hasLoadedFile();
}

bool ACEStepVST3AudioProcessor::isPreviewPlaying() const
{
    return preview_.isPlaying();
}

void ACEStepVST3AudioProcessor::syncPreviewFromState()
{
    preview_.clear();
    if (state_.previewFilePath.isEmpty())
    {
        return;
    }

    juce::String errorMessage;
    const juce::File previewFile(state_.previewFilePath);
    if (!preview_.loadFile(previewFile, errorMessage))
    {
        state_.errorMessage = errorMessage;
        return;
    }

    state_.previewDisplayName = previewFile.getFileName();
}
}  // namespace acestep::vst3

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new acestep::vst3::ACEStepVST3AudioProcessor();
}
