#pragma once

#include <JuceHeader.h>

#include <vector>

class GlorpyBassAudioProcessor : public juce::AudioProcessor
{
public:
    GlorpyBassAudioProcessor();
    ~GlorpyBassAudioProcessor() override;

    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;
    bool isBusesLayoutSupported(const BusesLayout& layouts) const override;
    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    const juce::String getName() const override;
    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram(int index) override;
    const juce::String getProgramName(int index) override;
    void changeProgramName(int index, const juce::String& newName) override;

    void getStateInformation(juce::MemoryBlock& destData) override;
    void setStateInformation(const void* data, int sizeInBytes) override;

    juce::AudioProcessorValueTreeState& getValueTreeState();
    juce::MidiKeyboardState& getKeyboardState();
    static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();

private:
    struct ActiveNote
    {
        int midiNote = -1;
        float velocity = 0.0f;
    };

    class MonoBassEngine
    {
    public:
        void prepare(const juce::dsp::ProcessSpec& spec);
        void reset();
        void noteOn(int midiNote, float velocity);
        void noteOff(int midiNote);
        float renderSample(const juce::AudioProcessorValueTreeState& state);

    private:
        float nextEnvelopeSample(float attackStep, float releaseStep);
        float midiToFrequency(int midiNote) const;
        float shapeWetness(float sample, float wetness) const;

        juce::dsp::Oscillator<float> oscA { [](float x) { return std::sin(x); } };
        juce::dsp::Oscillator<float> oscB { [](float x) { return x / juce::MathConstants<float>::pi; } };
        juce::dsp::Oscillator<float> subOsc { [](float x) { return x < 0.0f ? -1.0f : 1.0f; } };
        juce::dsp::Oscillator<float> wobbleLfo { [](float x) { return std::sin(x); } };
        juce::dsp::StateVariableTPTFilter<float> filter;
        std::vector<ActiveNote> heldNotes;
        double sampleRate = 44100.0;
        float currentFrequency = 55.0f;
        float targetFrequency = 55.0f;
        float currentVelocity = 0.0f;
        float ampEnvelope = 0.0f;
        float filterEnvelope = 0.0f;
        bool gate = false;
    };

    juce::AudioProcessorValueTreeState apvts;
    juce::MidiKeyboardState keyboardState;
    MonoBassEngine bassEngine;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(GlorpyBassAudioProcessor)
};
