#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

#include <memory>
#include <vector>

class GlorpyBassAudioProcessorEditor : public juce::AudioProcessorEditor
{
public:
    explicit GlorpyBassAudioProcessorEditor(GlorpyBassAudioProcessor&);
    ~GlorpyBassAudioProcessorEditor() override;

    void paint(juce::Graphics&) override;
    void resized() override;

private:
    using SliderAttachment = juce::AudioProcessorValueTreeState::SliderAttachment;

    void addKnob(juce::Slider& slider, juce::Label& label, const juce::String& text, const juce::String& parameterId);

    GlorpyBassAudioProcessor& processor;
    juce::MidiKeyboardComponent keyboard;

    juce::Label titleLabel;
    juce::Label subtitleLabel;
    juce::OwnedArray<juce::Slider> knobs;
    juce::OwnedArray<juce::Label> labels;
    std::vector<std::unique_ptr<SliderAttachment>> attachments;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(GlorpyBassAudioProcessorEditor)
};
