#include "PluginEditor.h"

#include <array>

GlorpyBassAudioProcessorEditor::GlorpyBassAudioProcessorEditor(GlorpyBassAudioProcessor& p)
    : AudioProcessorEditor(&p)
    , processor(p)
    , keyboard(processor.getKeyboardState(), juce::MidiKeyboardComponent::horizontalKeyboard)
{
    setSize(720, 560);

    titleLabel.setText("GLORPY BASS", juce::dontSendNotification);
    titleLabel.setJustificationType(juce::Justification::centredLeft);
    titleLabel.setFont(juce::FontOptions(34.0f, juce::Font::bold));
    addAndMakeVisible(titleLabel);

    subtitleLabel.setText("Live-ready mono bass. Mod wheel adds wobble, pitch bend slides the note, and the output is soft-limited for safer performance.", juce::dontSendNotification);
    subtitleLabel.setJustificationType(juce::Justification::centredLeft);
    subtitleLabel.setColour(juce::Label::textColourId, juce::Colours::white.withAlpha(0.75f));
    addAndMakeVisible(subtitleLabel);

    keyboard.setAvailableRange(24, 84);
    keyboard.setKeyWidth(26.0f);
    keyboard.setScrollButtonsVisible(false);
    keyboard.setColour(juce::MidiKeyboardComponent::whiteNoteColourId, juce::Colour::fromRGB(245, 244, 235));
    keyboard.setColour(juce::MidiKeyboardComponent::blackNoteColourId, juce::Colour::fromRGB(39, 35, 46));
    keyboard.setColour(juce::MidiKeyboardComponent::keyDownOverlayColourId, juce::Colour::fromRGB(120, 255, 187).withAlpha(0.7f));
    keyboard.setColour(juce::MidiKeyboardComponent::textLabelColourId, juce::Colours::black.withAlpha(0.7f));
    addAndMakeVisible(keyboard);

    const std::array<std::pair<const char*, const char*>, 14> controls = {{
        { "attack", "Attack" },
        { "release", "Release" },
        { "velocity", "Velocity" },
        { "bend", "Bend" },
        { "mix", "Wave" },
        { "sub", "Sub" },
        { "cutoff", "Cutoff" },
        { "resonance", "Reso" },
        { "env", "Glorp" },
        { "wobbleRate", "Rate" },
        { "wobbleDepth", "Depth" },
        { "glide", "Glide" },
        { "drive", "Drive" },
        { "level", "Level" }
    }};

    knobs.ensureStorageAllocated(static_cast<int>(controls.size()));
    labels.ensureStorageAllocated(static_cast<int>(controls.size()));
    attachments.reserve(controls.size());

    for (const auto& [parameterId, text] : controls)
    {
        auto* slider = knobs.add(new juce::Slider());
        auto* label = labels.add(new juce::Label());
        addKnob(*slider, *label, text, parameterId);
    }
}

GlorpyBassAudioProcessorEditor::~GlorpyBassAudioProcessorEditor() = default;

void GlorpyBassAudioProcessorEditor::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colour::fromRGB(18, 10, 23));

    juce::ColourGradient background(juce::Colour::fromRGB(33, 18, 41), 0.0f, 0.0f,
                                    juce::Colour::fromRGB(7, 35, 42), static_cast<float>(getWidth()), static_cast<float>(getHeight()), false);
    background.addColour(0.45, juce::Colour::fromRGB(80, 24, 79));
    g.setGradientFill(background);
    g.fillRoundedRectangle(getLocalBounds().toFloat().reduced(10.0f), 24.0f);

    g.setColour(juce::Colour::fromRGB(131, 255, 214).withAlpha(0.22f));
    g.fillEllipse(440.0f, 34.0f, 220.0f, 150.0f);

    g.setColour(juce::Colours::white.withAlpha(0.08f));
    for (int x = 24; x < getWidth() - 24; x += 18)
        g.drawVerticalLine(x, 110.0f, static_cast<float>(getHeight() - 24));
}

void GlorpyBassAudioProcessorEditor::resized()
{
    auto bounds = getLocalBounds().reduced(26);
    auto header = bounds.removeFromTop(84);

    titleLabel.setBounds(header.removeFromTop(40));
    subtitleLabel.setBounds(header.removeFromTop(22));

    auto keyboardArea = bounds.removeFromBottom(120).reduced(0, 8);
    keyboard.setBounds(keyboardArea);

    auto knobsArea = bounds.reduced(0, 12);
    const int columns = 4;
    const int knobWidth = knobsArea.getWidth() / columns;
    const int rows = 4;
    const int knobHeight = knobsArea.getHeight() / rows;

    for (int i = 0; i < knobs.size(); ++i)
    {
        const int row = i / columns;
        const int column = i % columns;
        auto area = juce::Rectangle<int>(knobsArea.getX() + column * knobWidth,
                                         knobsArea.getY() + row * knobHeight,
                                         knobWidth,
                                         knobHeight).reduced(10);

        labels[i]->setBounds(area.removeFromTop(24));
        knobs[i]->setBounds(area);
    }
}

void GlorpyBassAudioProcessorEditor::addKnob(juce::Slider& slider, juce::Label& label, const juce::String& text, const juce::String& parameterId)
{
    slider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    slider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 72, 18);
    slider.setColour(juce::Slider::rotarySliderFillColourId, juce::Colour::fromRGB(120, 255, 187));
    slider.setColour(juce::Slider::rotarySliderOutlineColourId, juce::Colour::fromRGB(42, 48, 54));
    slider.setColour(juce::Slider::textBoxTextColourId, juce::Colours::white);
    slider.setColour(juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);
    addAndMakeVisible(slider);

    label.setText(text, juce::dontSendNotification);
    label.setJustificationType(juce::Justification::centred);
    label.setColour(juce::Label::textColourId, juce::Colours::white);
    addAndMakeVisible(label);

    attachments.push_back(std::make_unique<SliderAttachment>(processor.getValueTreeState(), parameterId, slider));
}
