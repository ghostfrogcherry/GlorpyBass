#include "PluginProcessor.h"
#include "PluginEditor.h"

#include <algorithm>
#include <cmath>

namespace
{
juce::String formatPercent(float value)
{
    return juce::String(juce::roundToInt(value * 100.0f)) + "%";
}

juce::String formatHz(float value)
{
    return juce::String(juce::roundToInt(value)) + " Hz";
}

juce::String formatMs(float value)
{
    return juce::String(juce::roundToInt(value)) + " ms";
}

juce::String formatSemitones(float value)
{
    const bool wholeStep = std::abs(value - std::round(value)) < 0.001f;
    return juce::String(value, wholeStep ? 0 : 1) + " st";
}
}

GlorpyBassAudioProcessor::GlorpyBassAudioProcessor()
    : AudioProcessor(BusesProperties().withOutput("Output", juce::AudioChannelSet::stereo(), true))
    , apvts(*this, nullptr, "Parameters", createParameterLayout())
{
}

GlorpyBassAudioProcessor::~GlorpyBassAudioProcessor() = default;

void GlorpyBassAudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    juce::dsp::ProcessSpec spec;
    spec.sampleRate = sampleRate;
    spec.maximumBlockSize = static_cast<juce::uint32>(samplesPerBlock);
    spec.numChannels = 1;

    bassEngine.prepare(spec);
}

void GlorpyBassAudioProcessor::releaseResources()
{
}

bool GlorpyBassAudioProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const
{
    return layouts.getMainOutputChannelSet() == juce::AudioChannelSet::mono()
        || layouts.getMainOutputChannelSet() == juce::AudioChannelSet::stereo();
}

void GlorpyBassAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    keyboardState.processNextMidiBuffer(midiMessages, 0, buffer.getNumSamples(), true);

    for (const auto metadata : midiMessages)
    {
        const auto message = metadata.getMessage();

        if (message.isNoteOn())
            bassEngine.noteOn(message.getNoteNumber(), message.getFloatVelocity());
        else if (message.isNoteOff())
            bassEngine.noteOff(message.getNoteNumber());
        else if (message.isPitchWheel())
            bassEngine.setPitchBend(message.getPitchWheelValue());
        else if (message.isControllerOfType(1))
            bassEngine.setModWheel(static_cast<float>(message.getControllerValue()) / 127.0f);
        else if (message.isAllNotesOff() || message.isAllSoundOff())
            bassEngine.reset();
    }

    buffer.clear();

    auto* left = buffer.getWritePointer(0);
    auto* right = buffer.getNumChannels() > 1 ? buffer.getWritePointer(1) : nullptr;

    for (int sample = 0; sample < buffer.getNumSamples(); ++sample)
    {
        const float value = bassEngine.renderSample(apvts);
        left[sample] = value;

        if (right != nullptr)
            right[sample] = value;
    }
}

juce::AudioProcessorEditor* GlorpyBassAudioProcessor::createEditor()
{
    return new GlorpyBassAudioProcessorEditor(*this);
}

bool GlorpyBassAudioProcessor::hasEditor() const
{
    return true;
}

const juce::String GlorpyBassAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool GlorpyBassAudioProcessor::acceptsMidi() const
{
    return true;
}

bool GlorpyBassAudioProcessor::producesMidi() const
{
    return false;
}

bool GlorpyBassAudioProcessor::isMidiEffect() const
{
    return false;
}

double GlorpyBassAudioProcessor::getTailLengthSeconds() const
{
    return 0.1;
}

int GlorpyBassAudioProcessor::getNumPrograms()
{
    return 1;
}

int GlorpyBassAudioProcessor::getCurrentProgram()
{
    return 0;
}

void GlorpyBassAudioProcessor::setCurrentProgram(int)
{
}

const juce::String GlorpyBassAudioProcessor::getProgramName(int)
{
    return {};
}

void GlorpyBassAudioProcessor::changeProgramName(int, const juce::String&)
{
}

void GlorpyBassAudioProcessor::getStateInformation(juce::MemoryBlock& destData)
{
    auto state = apvts.copyState();
    std::unique_ptr<juce::XmlElement> xml(state.createXml());
    copyXmlToBinary(*xml, destData);
}

void GlorpyBassAudioProcessor::setStateInformation(const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xml(getXmlFromBinary(data, sizeInBytes));

    if (xml != nullptr)
        if (xml->hasTagName(apvts.state.getType()))
            apvts.replaceState(juce::ValueTree::fromXml(*xml));
}

juce::AudioProcessorValueTreeState& GlorpyBassAudioProcessor::getValueTreeState()
{
    return apvts;
}

juce::MidiKeyboardState& GlorpyBassAudioProcessor::getKeyboardState()
{
    return keyboardState;
}

juce::AudioProcessorValueTreeState::ParameterLayout GlorpyBassAudioProcessor::createParameterLayout()
{
    using Parameter = juce::AudioParameterFloat;
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> parameters;

    auto makeFloat = [](const char* id,
                        const char* name,
                        juce::NormalisableRange<float> range,
                        float defaultValue,
                        std::function<juce::String(float)> formatter = {})
    {
        auto attributes = juce::AudioParameterFloatAttributes();

        if (formatter != nullptr)
        {
            attributes = attributes.withStringFromValueFunction([formatter](float value, int) {
                return formatter(value);
            });
        }

        return std::make_unique<Parameter>(juce::ParameterID { id, 1 }, name, range, defaultValue, attributes);
    };

    parameters.push_back(makeFloat("mix", "Wave Mix", juce::NormalisableRange<float>(0.0f, 1.0f), 0.42f, formatPercent));
    parameters.push_back(makeFloat("sub", "Sub", juce::NormalisableRange<float>(0.0f, 1.0f), 0.55f, formatPercent));
    parameters.push_back(makeFloat("cutoff", "Cutoff", juce::NormalisableRange<float>(70.0f, 2800.0f, 1.0f, 0.35f), 420.0f, formatHz));
    parameters.push_back(makeFloat("resonance", "Resonance", juce::NormalisableRange<float>(0.15f, 1.2f), 0.85f, formatPercent));
    parameters.push_back(makeFloat("env", "Glorp", juce::NormalisableRange<float>(0.0f, 1.0f), 0.75f, formatPercent));
    parameters.push_back(makeFloat("wobbleRate", "Wobble Rate", juce::NormalisableRange<float>(0.1f, 12.0f, 0.01f, 0.4f), 2.8f));
    parameters.push_back(makeFloat("wobbleDepth", "Wobble Depth", juce::NormalisableRange<float>(0.0f, 1.0f), 0.35f, formatPercent));
    parameters.push_back(makeFloat("glide", "Glide", juce::NormalisableRange<float>(0.0f, 220.0f), 85.0f, formatMs));
    parameters.push_back(makeFloat("drive", "Drive", juce::NormalisableRange<float>(1.0f, 8.0f), 3.4f));
    parameters.push_back(makeFloat("attack", "Attack", juce::NormalisableRange<float>(0.0f, 200.0f, 1.0f, 0.5f), 8.0f, formatMs));
    parameters.push_back(makeFloat("release", "Release", juce::NormalisableRange<float>(30.0f, 2000.0f, 1.0f, 0.45f), 320.0f, formatMs));
    parameters.push_back(makeFloat("velocity", "Velocity", juce::NormalisableRange<float>(0.0f, 1.0f), 0.7f, formatPercent));
    parameters.push_back(makeFloat("bend", "Bend Range", juce::NormalisableRange<float>(1.0f, 12.0f, 1.0f), 2.0f, formatSemitones));
    parameters.push_back(makeFloat("level", "Level", juce::NormalisableRange<float>(0.0f, 1.0f), 0.8f, formatPercent));

    return { parameters.begin(), parameters.end() };
}

void GlorpyBassAudioProcessor::MonoBassEngine::prepare(const juce::dsp::ProcessSpec& spec)
{
    sampleRate = spec.sampleRate;
    oscA.prepare(spec);
    oscB.prepare(spec);
    subOsc.prepare(spec);
    wobbleLfo.prepare(spec);
    filter.prepare(spec);
    filter.setType(juce::dsp::StateVariableTPTFilterType::lowpass);
    reset();
}

void GlorpyBassAudioProcessor::MonoBassEngine::reset()
{
    oscA.reset();
    oscB.reset();
    subOsc.reset();
    wobbleLfo.reset();
    filter.reset();
    heldNotes.clear();
    currentFrequency = targetFrequency = 55.0f;
    pitchBendSemitones = 0.0f;
    modWheel = 0.0f;
    currentVelocity = 0.0f;
    ampEnvelope = 0.0f;
    filterEnvelope = 0.0f;
    gate = false;
}

void GlorpyBassAudioProcessor::MonoBassEngine::noteOn(int midiNote, float velocity)
{
    heldNotes.erase(std::remove_if(heldNotes.begin(), heldNotes.end(), [midiNote](const ActiveNote& note) {
        return note.midiNote == midiNote;
    }), heldNotes.end());

    heldNotes.push_back({ midiNote, velocity });
    targetFrequency = midiToFrequency(midiNote);
    currentVelocity = juce::jlimit(0.0f, 1.0f, velocity);
    gate = true;
}

void GlorpyBassAudioProcessor::MonoBassEngine::noteOff(int midiNote)
{
    heldNotes.erase(std::remove_if(heldNotes.begin(), heldNotes.end(), [midiNote](const ActiveNote& note) {
        return note.midiNote == midiNote;
    }), heldNotes.end());

    if (heldNotes.empty())
    {
        gate = false;
        return;
    }

    const auto& lastNote = heldNotes.back();
    targetFrequency = midiToFrequency(lastNote.midiNote);
    currentVelocity = lastNote.velocity;
    gate = true;
}

void GlorpyBassAudioProcessor::MonoBassEngine::setPitchBend(int value)
{
    constexpr float center = 8192.0f;
    pitchBendSemitones = (static_cast<float>(value) - center) / center;
}

void GlorpyBassAudioProcessor::MonoBassEngine::setModWheel(float value)
{
    modWheel = juce::jlimit(0.0f, 1.0f, value);
}

float GlorpyBassAudioProcessor::MonoBassEngine::renderSample(const juce::AudioProcessorValueTreeState& state)
{
    auto getParam = [&state](const char* id) {
        return state.getRawParameterValue(id)->load();
    };

    const float mix = getParam("mix");
    const float sub = getParam("sub");
    const float cutoff = getParam("cutoff");
    const float resonance = getParam("resonance");
    const float glorp = getParam("env");
    const float wobbleRate = getParam("wobbleRate");
    const float wobbleDepth = getParam("wobbleDepth");
    const float glideMs = getParam("glide");
    const float drive = getParam("drive");
    const float attackMs = getParam("attack");
    const float releaseMs = getParam("release");
    const float velocityTracking = getParam("velocity");
    const float bendRange = getParam("bend");
    const float level = getParam("level");

    const float glideCoeff = glideMs <= 1.0f ? 1.0f : 1.0f - std::exp(-1.0f / (0.001f * glideMs * static_cast<float>(sampleRate)));
    currentFrequency += (targetFrequency - currentFrequency) * glideCoeff;

    const float bentFrequency = currentFrequency * std::pow(2.0f, (pitchBendSemitones * bendRange) / 12.0f);

    oscA.setFrequency(bentFrequency);
    oscB.setFrequency(bentFrequency * 1.003f);
    subOsc.setFrequency(bentFrequency * 0.5f);
    wobbleLfo.setFrequency(wobbleRate);

    const float attackStep = 1.0f / juce::jmax(1.0, sampleRate * (0.001 * juce::jmax(1.0f, attackMs)));
    const float releaseStep = 1.0f / juce::jmax(1.0, sampleRate * (0.001 * releaseMs));

    ampEnvelope = nextEnvelopeSample(attackStep, releaseStep);
    filterEnvelope = gate ? juce::jmin(1.0f, filterEnvelope + (attackStep * 1.7f))
                          : juce::jmax(0.0f, filterEnvelope - (releaseStep * 0.75f));

    const float wobble = (wobbleLfo.processSample(0.0f) * 0.5f) + 0.5f;
    const float performanceWobble = juce::jlimit(0.0f, 1.25f, wobbleDepth + (modWheel * 0.75f));
    const float movingCutoff = cutoff + (glorp * 2000.0f * filterEnvelope) + (performanceWobble * 900.0f * wobble);
    filter.setCutoffFrequency(juce::jlimit(50.0f, 8000.0f, movingCutoff));
    filter.setResonance(resonance);

    const float base = (1.0f - mix) * oscA.processSample(0.0f) + mix * oscB.processSample(0.0f);
    const float swollen = base + sub * 0.65f * subOsc.processSample(0.0f);
    const float filtered = filter.processSample(0, swollen);
    const float wet = shapeWetness(filtered * drive, glorp);
    const float velocityGain = (1.0f - velocityTracking) + (currentVelocity * velocityTracking);
    const float voiced = wet * ampEnvelope * velocityGain * level;

    return std::tanh(voiced * 1.25f) * 0.92f;
}

float GlorpyBassAudioProcessor::MonoBassEngine::nextEnvelopeSample(float attackStep, float releaseStep)
{
    if (gate)
        return juce::jmin(1.0f, ampEnvelope + attackStep);

    return juce::jmax(0.0f, ampEnvelope - releaseStep);
}

float GlorpyBassAudioProcessor::MonoBassEngine::midiToFrequency(int midiNote) const
{
    return static_cast<float>(juce::MidiMessage::getMidiNoteInHertz(midiNote));
}

float GlorpyBassAudioProcessor::MonoBassEngine::shapeWetness(float sample, float wetness) const
{
    const float squish = std::tanh(sample);
    const float bubble = std::sin(sample * (1.3f + (wetness * 1.8f)));
    return juce::jlimit(-1.0f, 1.0f, squish * (1.0f - wetness * 0.35f) + bubble * wetness * 0.32f);
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new GlorpyBassAudioProcessor();
}
