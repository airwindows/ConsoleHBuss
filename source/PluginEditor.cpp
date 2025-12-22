#include "PluginEditor.h"

PluginEditor::PluginEditor (PluginProcessor& p)
    : AudioProcessorEditor (&p), processorRef (p)
{
    juce::ignoreUnused (processorRef);
    setResizable(true, true);
    setLookAndFeel(&airwindowsLookAndFeel);
    if (hostTrackColour != juce::Colour()) {
        airwindowsLookAndFeel.setColour(juce::ResizableWindow::backgroundColourId, hostTrackColour);
        airwindowsLookAndFeel.setColour(juce::Slider::thumbColourId, hostTrackColour);
    } else {
        airwindowsLookAndFeel.setColour(juce::ResizableWindow::backgroundColourId, airwindowsLookAndFeel.defaultColour);
        airwindowsLookAndFeel.setColour(juce::Slider::thumbColourId, airwindowsLookAndFeel.defaultColour);
    }
    updateTrackProperties();

    idleTimer = std::make_unique<IdleTimer>(this);
    idleTimer->startTimer(1000/30); //space between UI screen updates. Larger is slower updates to screen

    meter.setOpaque(true);
    meter.resetArrays();
    meter.displayTrackName = hostTrackName;
    addAndMakeVisible(meter);

    if (airwindowsLookAndFeel.knobMode == 0) {
        HIGKnob.setSliderStyle(juce::Slider::Rotary);
        MIDKnob.setSliderStyle(juce::Slider::Rotary);
        LOWKnob.setSliderStyle(juce::Slider::Rotary);
        THRKnob.setSliderStyle(juce::Slider::Rotary);
        HIPKnob.setSliderStyle(juce::Slider::Rotary);
        LOPKnob.setSliderStyle(juce::Slider::Rotary);
        PANKnob.setSliderStyle(juce::Slider::Rotary);
        FADKnob.setSliderStyle(juce::Slider::Rotary);
    }

    if (airwindowsLookAndFeel.knobMode == 1) {
        HIGKnob.setSliderStyle(juce::Slider::RotaryVerticalDrag);
        MIDKnob.setSliderStyle(juce::Slider::RotaryVerticalDrag);
        LOWKnob.setSliderStyle(juce::Slider::RotaryVerticalDrag);
        THRKnob.setSliderStyle(juce::Slider::RotaryVerticalDrag);
        HIPKnob.setSliderStyle(juce::Slider::RotaryVerticalDrag);
        LOPKnob.setSliderStyle(juce::Slider::RotaryVerticalDrag);
        PANKnob.setSliderStyle(juce::Slider::RotaryVerticalDrag);
        FADKnob.setSliderStyle(juce::Slider::RotaryVerticalDrag);
    }

    if (airwindowsLookAndFeel.knobMode == 2) {
        HIGKnob.setSliderStyle(juce::Slider::RotaryHorizontalDrag);
        MIDKnob.setSliderStyle(juce::Slider::RotaryHorizontalDrag);
        LOWKnob.setSliderStyle(juce::Slider::RotaryHorizontalDrag);
        THRKnob.setSliderStyle(juce::Slider::RotaryHorizontalDrag);
        HIPKnob.setSliderStyle(juce::Slider::RotaryHorizontalDrag);
        LOPKnob.setSliderStyle(juce::Slider::RotaryHorizontalDrag);
        PANKnob.setSliderStyle(juce::Slider::RotaryHorizontalDrag);
        FADKnob.setSliderStyle(juce::Slider::RotaryHorizontalDrag);
    }

    if (airwindowsLookAndFeel.knobMode == 3) {
        HIGKnob.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
        MIDKnob.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
        LOWKnob.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
        THRKnob.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
        HIPKnob.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
        LOPKnob.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
        PANKnob.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
        FADKnob.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    }

    HIGKnob.setTextBoxStyle(juce::Slider::NoTextBox, false, 100, 20);
    HIGKnob.setRange(0.0f, 1.0f);
    HIGKnob.setValue(processorRef.params[PluginProcessor::KNOBHIG]->get(), juce::NotificationType::dontSendNotification);
    HIGKnob.addListener(this);
    addAndMakeVisible(HIGKnob);
 
    MIDKnob.setTextBoxStyle(juce::Slider::NoTextBox, false, 100, 20);
    MIDKnob.setRange(0.0f, 1.0f);
    MIDKnob.setValue(processorRef.params[PluginProcessor::KNOBMID]->get(), juce::NotificationType::dontSendNotification);
    MIDKnob.addListener(this);
    addAndMakeVisible(MIDKnob);
 
    LOWKnob.setTextBoxStyle(juce::Slider::NoTextBox, false, 100, 20);
    LOWKnob.setRange(0.0f, 1.0f);
    LOWKnob.setValue(processorRef.params[PluginProcessor::KNOBLOW]->get(), juce::NotificationType::dontSendNotification);
    LOWKnob.addListener(this);
    addAndMakeVisible(LOWKnob);

    THRKnob.setTextBoxStyle(juce::Slider::NoTextBox, false, 100, 20);
    THRKnob.setRange(0.0f, 1.0f);
    THRKnob.setValue(processorRef.params[PluginProcessor::KNOBTHR]->get(), juce::NotificationType::dontSendNotification);
    THRKnob.addListener(this);
    addAndMakeVisible(THRKnob);
    THRKnob.setColour(juce::Slider::thumbColourId, juce::Colour().fromFloatRGBA(0.0f, 0.0f, 0.0f, 1.0f)); //black

    HIPKnob.setTextBoxStyle(juce::Slider::NoTextBox, false, 100, 20);
    HIPKnob.setRange(0.0f, 1.0f);
    HIPKnob.setValue(processorRef.params[PluginProcessor::KNOBHIP]->get(), juce::NotificationType::dontSendNotification);
    HIPKnob.addListener(this);
    addAndMakeVisible(HIPKnob);
    
    LOPKnob.setTextBoxStyle(juce::Slider::NoTextBox, false, 100, 20);
    LOPKnob.setRange(0.0f, 1.0f);
    LOPKnob.setValue(processorRef.params[PluginProcessor::KNOBLOP]->get(), juce::NotificationType::dontSendNotification);
    LOPKnob.addListener(this);
    addAndMakeVisible(LOPKnob);
    
    PANKnob.setSliderStyle(juce::Slider::LinearHorizontal);
    PANKnob.setTextBoxStyle(juce::Slider::NoTextBox, false, 100, 20);
    PANKnob.setRange(0.0f, 1.0f);
    PANKnob.setValue(processorRef.params[PluginProcessor::KNOBPAN]->get(), juce::NotificationType::dontSendNotification);
    PANKnob.addListener(this);
    addAndMakeVisible(PANKnob);
    PANKnob.setColour(juce::Slider::thumbColourId, juce::Colour().fromFloatRGBA(0.0f, 0.0f, 0.0f, 1.0f)); //black

    FADKnob.setTextBoxStyle(juce::Slider::NoTextBox, false, 100, 20);
    FADKnob.setRange(0.0f, 1.0f);
    FADKnob.setValue(processorRef.params[PluginProcessor::KNOBFAD]->get(), juce::NotificationType::dontSendNotification);
    FADKnob.addListener(this);
    addAndMakeVisible(FADKnob);

    setSize (airwindowsLookAndFeel.userWidth, airwindowsLookAndFeel.userHeight);
    // Make sure that before the constructor has finished, you've set the editor's size to whatever you need it to be.
}

PluginEditor::~PluginEditor(){
    setLookAndFeel(nullptr);
}

void PluginEditor::paint (juce::Graphics& g)
{
    if (airwindowsLookAndFeel.blurImage == juce::Image()) {
        g.fillAll (airwindowsLookAndFeel.defaultColour);
        if (hostTrackColour != juce::Colour()) {
            g.setFillType(juce::FillType(hostTrackColour)); g.setOpacity(airwindowsLookAndFeel.applyTrackColour); g.fillAll();
        }
        airwindowsLookAndFeel.setColour(juce::ResizableWindow::backgroundColourId, airwindowsLookAndFeel.defaultColour.interpolatedWith (hostTrackColour, airwindowsLookAndFeel.applyTrackColour));
        airwindowsLookAndFeel.setColour(juce::Slider::thumbColourId, airwindowsLookAndFeel.defaultColour.interpolatedWith (hostTrackColour, airwindowsLookAndFeel.applyTrackColour));
    } else {
        if (airwindowsLookAndFeel.usingNamedImage) {
            g.drawImageWithin(airwindowsLookAndFeel.backgroundImage, 0, 0, getLocalBounds().getWidth(), getLocalBounds().getHeight(), 0);
        } else {
            g.setTiledImageFill(airwindowsLookAndFeel.backgroundImage, 0, 0, 1.0f); g.fillAll();
        }
                
        if (hostTrackColour != juce::Colour()) {
            g.setFillType(juce::FillType(hostTrackColour)); g.setOpacity(airwindowsLookAndFeel.applyTrackColour); g.fillAll();
        }
        airwindowsLookAndFeel.defaultColour = juce::Colour::fromRGBA(airwindowsLookAndFeel.blurImage.getPixelAt(1,1).getRed(),airwindowsLookAndFeel.blurImage.getPixelAt(1,1).getGreen(),airwindowsLookAndFeel.blurImage.getPixelAt(1,1).getBlue(),1.0);
        airwindowsLookAndFeel.setColour(juce::ResizableWindow::backgroundColourId, airwindowsLookAndFeel.defaultColour);
        airwindowsLookAndFeel.setColour(juce::Slider::thumbColourId, airwindowsLookAndFeel.defaultColour);
    } //find the color of the background tile or image, if there is one. Please use low-contrast stuff, but I'm not your mom :)
    
    g.setFont(juce::FontOptions(airwindowsLookAndFeel.newFont, g.getCurrentFont().getHeight(), 0));
    auto linewidth = getLocalBounds().getWidth(); if (getLocalBounds().getHeight() > linewidth) linewidth = getLocalBounds().getHeight();  linewidth = (int)cbrt(linewidth/2)/2;
    if ((hostTrackName == juce::String()) || (hostTrackName.length() < 1.0f)) hostTrackName = juce::String("ConsoleH Buss");
    meter.displayTrackName = hostTrackName; //if not track name, then name of plugin. To be displayed on the actual peakmeter
    meter.displayFont = airwindowsLookAndFeel.newFont; //in the custom font, if we're using one
        
    g.setColour (findColour(juce::ResizableWindow::backgroundColourId).interpolatedWith (juce::Colours::white, 0.618f));
    g.fillRect(0, 0, getLocalBounds().getWidth(), linewidth); g.fillRect(0, 0, linewidth, getLocalBounds().getHeight());
    g.setColour (findColour(juce::ResizableWindow::backgroundColourId).interpolatedWith (juce::Colours::black, 0.382f));
    g.fillRect(linewidth, getLocalBounds().getHeight()-linewidth, getLocalBounds().getWidth(), linewidth); g.fillRect(getLocalBounds().getWidth()-linewidth, linewidth, linewidth, getLocalBounds().getHeight()-linewidth);
    g.setColour (juce::Colours::black); g.drawRect(0, 0, getLocalBounds().getWidth(), getLocalBounds().getHeight());
    //draw global bevel effect, either from the color or from the color of the blurred texture, and a black border
}

void PluginEditor::resized()
{
    auto area = getLocalBounds(); // this is a huge huge routine, but not all of it runs at all times!
    auto linewidth = (int)fmax(area.getHeight(),area.getWidth());
    linewidth = (int)cbrt(linewidth/2)-1;
    area.reduce(linewidth, linewidth);
    meter.displayWidth = (int)area.getWidth()-((linewidth/2)-2);
    meter.displayHeight = (int)(area.getHeight()*0.6180339f); //meter is golden ratio, controls are inverse ratio
    if (meter.displayHeight > (float)meter.displayWidth*0.6180339f) meter.displayHeight = (int)(meter.displayWidth*0.6180339f);
    meter.setBounds(linewidth,linewidth,meter.displayWidth,meter.displayHeight);
    area.removeFromTop(meter.displayHeight); //remaining area is for controls. getProportion sets first start X and Y placement, then size X and Y placement
    float aspectRatio = (float)area.getWidth() / (float)area.getHeight(); //Larger than 1: horisontal. Smaller than 1: vertical
    // 12h-1w = 0.11  (0.26)  6h-2w = 0.41   4h-3w = 1.0    3h-4w = 1.8    2h-6w = 3.85     1h-12w = 15.42
    float skew = airwindowsLookAndFeel.applyTilt; //this is the amount of tilt the knobs experience at top and bottom. MAX 0.5 becomes full tilt
    float lilKnob = 0.35f; float bigKnob = 0.65f; float hugeKnob = 0.81f;
    float sliderW = 0.99f; float sliderH = 0.25f; //these are knob sizes scaled to the size of the block
    //these are always the same for all aspect ratios, but panels and offsets are unique to the aspect ratios, as are the sequence of control blocks
    
    //float A1x = 0.18f; float A1y = 0.05f; float A2x = 0.06f; float A3x = 0.59f; float A23y = 0.68f; //A is big knob and two smaller knobs below
    
    float B1x = 0.1f; float B1y = 0.33f; float B2x = 0.62f; float B2y = 0.1f; //B is big and small knob above right
    
    //float C1234x = 0.01f; float C1y = 0.07f; float C2y = 0.29f; float C3y = 0.51f; float C4y = 0.73f; //C is the four slider dynamics bank, evenly spaced
    
    float D1x = 0.01f; float D1y = 0.07f; float D2x = 0.18f; float D2y = 0.35f; //D is one slider with a BigKnob under it: trim/more or pan/fader
    
    float S1x = 0.1f; float S1y = 0.15f; //S is single knob

    if (aspectRatio >= 0.0f && aspectRatio < 0.269f) { //6h-1w
        float pY = (float)area.getHeight()/6.0f; //size of each movable block
        float pX = (float)area.getWidth()/1.0f;
        float offsetY = 0.0f; float offsetX = 0.0f; float panelTilt = 0.5f; //1 and 2 wide don't have tilted knobs
        HIGKnob.setColour(juce::Slider::backgroundColourId, juce::Colour().fromFloatRGBA(panelTilt, 0.0f, 0.0f, 1.0f));
        HIGKnob.setBounds(area.getProportion(juce::Rectangle{(offsetX+(pX*S1x))/area.getWidth(), (offsetY+(pY*S1y))/area.getHeight(), (pX*hugeKnob)/area.getWidth(), (pY*hugeKnob)/area.getHeight()})); //High
        offsetY = pY; //put between vertical SECTIONS (not knobs)
        
        MIDKnob.setColour(juce::Slider::backgroundColourId, juce::Colour().fromFloatRGBA(panelTilt, 0.0f, 0.0f, 1.0f));
        MIDKnob.setBounds(area.getProportion(juce::Rectangle{(offsetX+(pX*S1x))/area.getWidth(), (offsetY+(pY*S1y))/area.getHeight(), (pX*hugeKnob)/area.getWidth(), (pY*hugeKnob)/area.getHeight()})); //Mid
        offsetY = pY*2.0f; //put between vertical SECTIONS (not knobs)
        
        LOWKnob.setColour(juce::Slider::backgroundColourId, juce::Colour().fromFloatRGBA(panelTilt, 0.0f, 0.0f, 1.0f));
        LOWKnob.setBounds(area.getProportion(juce::Rectangle{(offsetX+(pX*S1x))/area.getWidth(), (offsetY+(pY*S1y))/area.getHeight(), (pX*hugeKnob)/area.getWidth(), (pY*hugeKnob)/area.getHeight()})); //Low
        offsetY = pY*3.0f; //put between vertical SECTIONS (not knobs)
        
        LOPKnob.setColour(juce::Slider::backgroundColourId, juce::Colour().fromFloatRGBA(panelTilt, 0.0f, 0.0f, 1.0f));
        LOPKnob.setBounds(area.getProportion(juce::Rectangle{(offsetX+(pX*B1x))/area.getWidth(), (offsetY+(pY*B1y))/area.getHeight(), (pX*bigKnob)/area.getWidth(), (pY*bigKnob)/area.getHeight()}));
        HIPKnob.setColour(juce::Slider::backgroundColourId, juce::Colour().fromFloatRGBA(panelTilt, 0.0f, 0.0f, 1.0f));
        HIPKnob.setBounds(area.getProportion(juce::Rectangle{(offsetX+(pX*B2x))/area.getWidth(), (offsetY+(pY*B2y))/area.getHeight(), (pX*lilKnob)/area.getWidth(), (pY*lilKnob)/area.getHeight()})); //Lowpass Highpass
        offsetY = pY*4.0f; //put between vertical SECTIONS (not knobs)
        
        THRKnob.setColour(juce::Slider::backgroundColourId, juce::Colour().fromFloatRGBA(panelTilt, 0.0f, 0.0f, 1.0f));
        THRKnob.setBounds(area.getProportion(juce::Rectangle{(offsetX+(pX*S1x))/area.getWidth(), (offsetY+(pY*S1y))/area.getHeight(), (pX*hugeKnob)/area.getWidth(), (pY*hugeKnob)/area.getHeight()})); //Low
        offsetY = pY*5.0f; //put between vertical SECTIONS (not knobs)
        
        PANKnob.setBounds(area.getProportion(juce::Rectangle{(offsetX+(pX*D1x))/area.getWidth(), (offsetY+(pY*D1y))/area.getHeight(), (pX*sliderW)/area.getWidth(), (pY*sliderH)/area.getHeight()}));
        FADKnob.setColour(juce::Slider::backgroundColourId, juce::Colour().fromFloatRGBA(panelTilt, 0.0f, 0.0f, 1.0f));
        FADKnob.setBounds(area.getProportion(juce::Rectangle{(offsetX+(pX*D2x))/area.getWidth(), (offsetY+(pY*D2y))/area.getHeight(), (pX*bigKnob)/area.getWidth(), (pY*bigKnob)/area.getHeight()})); //Pan Fader D block
    } //6h-1w
    
    if (aspectRatio >= 0.269f && aspectRatio < 1.078f) { //3h-2w
        float pY = (float)area.getHeight()/3.0f; //size of each movable block
        float pX = (float)area.getWidth()/2.0f;
        float offsetY = 0.0f; float offsetX = 0.0f; float panelTilt = 0.5f; //1 and 2 wide don't have angled knobs
        MIDKnob.setColour(juce::Slider::backgroundColourId, juce::Colour().fromFloatRGBA(panelTilt, 0.0f, 0.0f, 1.0f));
        MIDKnob.setBounds(area.getProportion(juce::Rectangle{(offsetX+(pX*S1x))/area.getWidth(), (offsetY+(pY*S1y))/area.getHeight(), (pX*hugeKnob)/area.getWidth(), (pY*hugeKnob)/area.getHeight()})); //Mid
        offsetX += pX; //put between horizontal SECTIONS (not knobs)
        
        HIGKnob.setColour(juce::Slider::backgroundColourId, juce::Colour().fromFloatRGBA(panelTilt, 0.0f, 0.0f, 1.0f));
        HIGKnob.setBounds(area.getProportion(juce::Rectangle{(offsetX+(pX*S1x))/area.getWidth(), (offsetY+(pY*S1y))/area.getHeight(), (pX*hugeKnob)/area.getWidth(), (pY*hugeKnob)/area.getHeight()})); //High
        offsetY = pY; offsetX = 0.0f; //put between vertical SECTIONS (not knobs)
        
        LOWKnob.setColour(juce::Slider::backgroundColourId, juce::Colour().fromFloatRGBA(panelTilt, 0.0f, 0.0f, 1.0f));
        LOWKnob.setBounds(area.getProportion(juce::Rectangle{(offsetX+(pX*S1x))/area.getWidth(), (offsetY+(pY*S1y))/area.getHeight(), (pX*hugeKnob)/area.getWidth(), (pY*hugeKnob)/area.getHeight()})); //Low
        offsetX += pX; //put between horizontal SECTIONS (not knobs)
        
        LOPKnob.setColour(juce::Slider::backgroundColourId, juce::Colour().fromFloatRGBA(panelTilt, 0.0f, 0.0f, 1.0f));
        LOPKnob.setBounds(area.getProportion(juce::Rectangle{(offsetX+(pX*B1x))/area.getWidth(), (offsetY+(pY*B1y))/area.getHeight(), (pX*bigKnob)/area.getWidth(), (pY*bigKnob)/area.getHeight()}));
        HIPKnob.setColour(juce::Slider::backgroundColourId, juce::Colour().fromFloatRGBA(panelTilt, 0.0f, 0.0f, 1.0f));
        HIPKnob.setBounds(area.getProportion(juce::Rectangle{(offsetX+(pX*B2x))/area.getWidth(), (offsetY+(pY*B2y))/area.getHeight(), (pX*lilKnob)/area.getWidth(), (pY*lilKnob)/area.getHeight()})); //Lowpass Highpass
        offsetY = pY*1.99f; offsetX = 0.0f; //put between vertical SECTIONS (not knobs)
        
        THRKnob.setColour(juce::Slider::backgroundColourId, juce::Colour().fromFloatRGBA(panelTilt, 0.0f, 0.0f, 1.0f));
        THRKnob.setBounds(area.getProportion(juce::Rectangle{(offsetX+(pX*S1x))/area.getWidth(), (offsetY+(pY*S1y))/area.getHeight(), (pX*hugeKnob)/area.getWidth(), (pY*hugeKnob)/area.getHeight()})); //Low
        offsetX += pX; //put between horizontal SECTIONS (not knobs)
        
        PANKnob.setBounds(area.getProportion(juce::Rectangle{(offsetX+(pX*D1x))/area.getWidth(), (offsetY+(pY*D1y))/area.getHeight(), (pX*sliderW)/area.getWidth(), (pY*sliderH)/area.getHeight()}));
        FADKnob.setColour(juce::Slider::backgroundColourId, juce::Colour().fromFloatRGBA(panelTilt, 0.0f, 0.0f, 1.0f));
        FADKnob.setBounds(area.getProportion(juce::Rectangle{(offsetX+(pX*D2x))/area.getWidth(), (offsetY+(pY*D2y))/area.getHeight(), (pX*bigKnob)/area.getWidth(), (pY*bigKnob)/area.getHeight()})); //Pan Fader D block
    } //3h-2w

    if (aspectRatio >= 1.078f && aspectRatio < 2.427f) { //2h-3w
        float pX = (float)area.getWidth()/3.0f;
        float offsetX = 0.0f; float panelTilt = 0.5f; //update the new panel tilt each time offsetY is updated. 0.0 is top panel, seen from underneath. 1.0 is bottom panel, seen from above
        float pY = (float)(area.getHeight()/2.0f);
        float offsetY = 0.0f;
        pY -= (pY*skew*0.4f); //top panel is smaller
        LOWKnob.setColour(juce::Slider::backgroundColourId, juce::Colour().fromFloatRGBA(panelTilt, 0.0f, 0.0f, 1.0f));
        LOWKnob.setBounds(area.getProportion(juce::Rectangle{(offsetX+(pX*S1x))/area.getWidth(), (offsetY+(pY*S1y))/area.getHeight(), (pX*hugeKnob)/area.getWidth(), (pY*hugeKnob)/area.getHeight()})); //Low
        offsetX += pX; //put between horizontal SECTIONS (not knobs)
        
        MIDKnob.setColour(juce::Slider::backgroundColourId, juce::Colour().fromFloatRGBA(panelTilt, 0.0f, 0.0f, 1.0f));
        MIDKnob.setBounds(area.getProportion(juce::Rectangle{(offsetX+(pX*S1x))/area.getWidth(), (offsetY+(pY*S1y))/area.getHeight(), (pX*hugeKnob)/area.getWidth(), (pY*hugeKnob)/area.getHeight()})); //Mid
        offsetX += pX; //put between horizontal SECTIONS (not knobs)
        
        HIGKnob.setColour(juce::Slider::backgroundColourId, juce::Colour().fromFloatRGBA(panelTilt, 0.0f, 0.0f, 1.0f));
        HIGKnob.setBounds(area.getProportion(juce::Rectangle{(offsetX+(pX*S1x))/area.getWidth(), (offsetY+(pY*S1y))/area.getHeight(), (pX*hugeKnob)/area.getWidth(), (pY*hugeKnob)/area.getHeight()})); //High
        pY = (float)area.getHeight()/2.0f;
        offsetY = pY; offsetX = 0.0f; //put between vertical SECTIONS (not knobs)
        
        LOPKnob.setColour(juce::Slider::backgroundColourId, juce::Colour().fromFloatRGBA(panelTilt, 0.0f, 0.0f, 1.0f));
        LOPKnob.setBounds(area.getProportion(juce::Rectangle{(offsetX+(pX*B1x))/area.getWidth(), (offsetY+(pY*B1y))/area.getHeight(), (pX*bigKnob)/area.getWidth(), (pY*bigKnob)/area.getHeight()}));
        HIPKnob.setColour(juce::Slider::backgroundColourId, juce::Colour().fromFloatRGBA(panelTilt, 0.0f, 0.0f, 1.0f));
        HIPKnob.setBounds(area.getProportion(juce::Rectangle{(offsetX+(pX*B2x))/area.getWidth(), (offsetY+(pY*B2y))/area.getHeight(), (pX*lilKnob)/area.getWidth(), (pY*lilKnob)/area.getHeight()})); //Lowpass Highpass
        offsetX += pX; //put between horizontal SECTIONS (not knobs)
        
        THRKnob.setColour(juce::Slider::backgroundColourId, juce::Colour().fromFloatRGBA(panelTilt, 0.0f, 0.0f, 1.0f));
        THRKnob.setBounds(area.getProportion(juce::Rectangle{(offsetX+(pX*S1x))/area.getWidth(), (offsetY+(pY*S1y))/area.getHeight(), (pX*hugeKnob)/area.getWidth(), (pY*hugeKnob)/area.getHeight()})); //Low
        offsetX += pX; //put between horizontal SECTIONS (not knobs)
        
        PANKnob.setBounds(area.getProportion(juce::Rectangle{(offsetX+(pX*D1x))/area.getWidth(), (offsetY+(pY*D1y))/area.getHeight(), (pX*sliderW)/area.getWidth(), (pY*sliderH)/area.getHeight()}));
        FADKnob.setColour(juce::Slider::backgroundColourId, juce::Colour().fromFloatRGBA(panelTilt, 0.0f, 0.0f, 1.0f));
        FADKnob.setBounds(area.getProportion(juce::Rectangle{(offsetX+(pX*D2x))/area.getWidth(), (offsetY+(pY*D2y))/area.getHeight(), (pX*bigKnob)/area.getWidth(), (pY*bigKnob)/area.getHeight()})); //Pan Fader D block
    } //2h-3w
    
    
    if (aspectRatio >= 2.427f && aspectRatio < 999999.0f) { //1h-6w
        float pY = (float)area.getHeight()/1.0f; //size of each movable block
        float pX = (float)area.getWidth()/6.0f;
        float offsetY = 0.0f; float offsetX = 0.0f; float panelTilt = 0.5f; //update the new panel tilt each time offsetY is updated. 0.0 is top panel, seen from underneath. 1.0 is bottom panel, seen from above
        LOWKnob.setColour(juce::Slider::backgroundColourId, juce::Colour().fromFloatRGBA(panelTilt, 0.0f, 0.0f, 1.0f));
        LOWKnob.setBounds(area.getProportion(juce::Rectangle{(offsetX+(pX*S1x))/area.getWidth(), (offsetY+(pY*S1y))/area.getHeight(), (pX*hugeKnob)/area.getWidth(), (pY*hugeKnob)/area.getHeight()})); //Low
        offsetX = pX; //put between vertical SECTIONS (not knobs)
        
        MIDKnob.setColour(juce::Slider::backgroundColourId, juce::Colour().fromFloatRGBA(panelTilt, 0.0f, 0.0f, 1.0f));
        MIDKnob.setBounds(area.getProportion(juce::Rectangle{(offsetX+(pX*S1x))/area.getWidth(), (offsetY+(pY*S1y))/area.getHeight(), (pX*hugeKnob)/area.getWidth(), (pY*hugeKnob)/area.getHeight()})); //Mid
        offsetX = pX*2.0f; //put between vertical SECTIONS (not knobs)
        
        HIGKnob.setColour(juce::Slider::backgroundColourId, juce::Colour().fromFloatRGBA(panelTilt, 0.0f, 0.0f, 1.0f));
        HIGKnob.setBounds(area.getProportion(juce::Rectangle{(offsetX+(pX*S1x))/area.getWidth(), (offsetY+(pY*S1y))/area.getHeight(), (pX*hugeKnob)/area.getWidth(), (pY*hugeKnob)/area.getHeight()})); //High
        offsetX = pX*3.0f; //put between vertical SECTIONS (not knobs)
        
        LOPKnob.setColour(juce::Slider::backgroundColourId, juce::Colour().fromFloatRGBA(panelTilt, 0.0f, 0.0f, 1.0f));
        LOPKnob.setBounds(area.getProportion(juce::Rectangle{(offsetX+(pX*B1x))/area.getWidth(), (offsetY+(pY*B1y))/area.getHeight(), (pX*bigKnob)/area.getWidth(), (pY*bigKnob)/area.getHeight()}));
        HIPKnob.setColour(juce::Slider::backgroundColourId, juce::Colour().fromFloatRGBA(panelTilt, 0.0f, 0.0f, 1.0f));
        HIPKnob.setBounds(area.getProportion(juce::Rectangle{(offsetX+(pX*B2x))/area.getWidth(), (offsetY+(pY*B2y))/area.getHeight(), (pX*lilKnob)/area.getWidth(), (pY*lilKnob)/area.getHeight()})); //Lowpass Highpass
        offsetX = pX*4.0f; //put between vertical SECTIONS (not knobs)
        
        THRKnob.setColour(juce::Slider::backgroundColourId, juce::Colour().fromFloatRGBA(panelTilt, 0.0f, 0.0f, 1.0f));
        THRKnob.setBounds(area.getProportion(juce::Rectangle{(offsetX+(pX*S1x))/area.getWidth(), (offsetY+(pY*S1y))/area.getHeight(), (pX*hugeKnob)/area.getWidth(), (pY*hugeKnob)/area.getHeight()})); //Low
        offsetX = pX*5.0f; //put between vertical SECTIONS (not knobs)
        
        PANKnob.setBounds(area.getProportion(juce::Rectangle{(offsetX+(pX*D1x))/area.getWidth(), (offsetY+(pY*D1y))/area.getHeight(), (pX*sliderW)/area.getWidth(), (pY*sliderH)/area.getHeight()}));
        FADKnob.setColour(juce::Slider::backgroundColourId, juce::Colour().fromFloatRGBA(panelTilt, 0.0f, 0.0f, 1.0f));
        FADKnob.setBounds(area.getProportion(juce::Rectangle{(offsetX+(pX*D2x))/area.getWidth(), (offsetY+(pY*D2y))/area.getHeight(), (pX*bigKnob)/area.getWidth(), (pY*bigKnob)/area.getHeight()})); //Pan Fader D block
    } //1h-6w*//

}

void PluginEditor::sliderValueChanged(juce::Slider *s)
{
    if (s == &HIPKnob)
    {
        PluginProcessor::UIToAudioMessage msg;
        msg.what = PluginProcessor::UIToAudioMessage::NEW_VALUE;
        msg.which = (PluginProcessor::Parameters)PluginProcessor::KNOBHIP;
        msg.newValue = (float)s->getValue();
        processorRef.uiToAudio.push(msg);
    }
    if (s == &LOPKnob)
    {
        PluginProcessor::UIToAudioMessage msg;
        msg.what = PluginProcessor::UIToAudioMessage::NEW_VALUE;
        msg.which = (PluginProcessor::Parameters)PluginProcessor::KNOBLOP;
        msg.newValue = (float)s->getValue();
        processorRef.uiToAudio.push(msg);
    }
    if (s == &HIGKnob)
    {
        PluginProcessor::UIToAudioMessage msg;
        msg.what = PluginProcessor::UIToAudioMessage::NEW_VALUE;
        msg.which = (PluginProcessor::Parameters)PluginProcessor::KNOBHIG;
        msg.newValue = (float)s->getValue();
        processorRef.uiToAudio.push(msg);
    }
    if (s == &MIDKnob)
    {
        PluginProcessor::UIToAudioMessage msg;
        msg.what = PluginProcessor::UIToAudioMessage::NEW_VALUE;
        msg.which = (PluginProcessor::Parameters)PluginProcessor::KNOBMID;
        msg.newValue = (float)s->getValue();
        processorRef.uiToAudio.push(msg);
    }
    if (s == &LOWKnob)
    {
        PluginProcessor::UIToAudioMessage msg;
        msg.what = PluginProcessor::UIToAudioMessage::NEW_VALUE;
        msg.which = (PluginProcessor::Parameters)PluginProcessor::KNOBLOW;
        msg.newValue = (float)s->getValue();
        processorRef.uiToAudio.push(msg);
    }
    if (s == &THRKnob)
    {
        PluginProcessor::UIToAudioMessage msg;
        msg.what = PluginProcessor::UIToAudioMessage::NEW_VALUE;
        msg.which = (PluginProcessor::Parameters)PluginProcessor::KNOBTHR;
        msg.newValue = (float)s->getValue();
        processorRef.uiToAudio.push(msg);
    }
    if (s == &PANKnob)
    {
        PluginProcessor::UIToAudioMessage msg;
        msg.what = PluginProcessor::UIToAudioMessage::NEW_VALUE;
        msg.which = (PluginProcessor::Parameters)PluginProcessor::KNOBPAN;
        msg.newValue = (float)s->getValue();
        processorRef.uiToAudio.push(msg);
    }
    if (s == &FADKnob)
    {
        PluginProcessor::UIToAudioMessage msg;
        msg.what = PluginProcessor::UIToAudioMessage::NEW_VALUE;
        msg.which = (PluginProcessor::Parameters)PluginProcessor::KNOBFAD;
        msg.newValue = (float)s->getValue();
        processorRef.uiToAudio.push(msg);
    }
}

void PluginEditor::sliderDragStarted(juce::Slider *s) {sliderDragInternal(s, true);}
void PluginEditor::sliderDragEnded(juce::Slider *s) {sliderDragInternal(s, false);}
void PluginEditor::sliderDragInternal(juce::Slider *s, bool bv)
{
    if (s == &HIPKnob) {
        PluginProcessor::UIToAudioMessage msg;
        msg.what = bv ? PluginProcessor::UIToAudioMessage::BEGIN_EDIT : PluginProcessor::UIToAudioMessage::END_EDIT;
        msg.which = (PluginProcessor::Parameters)PluginProcessor::KNOBHIP;
        processorRef.uiToAudio.push(msg);
    }
    if (s == &LOPKnob) {
        PluginProcessor::UIToAudioMessage msg;
        msg.what = bv ? PluginProcessor::UIToAudioMessage::BEGIN_EDIT : PluginProcessor::UIToAudioMessage::END_EDIT;
        msg.which = (PluginProcessor::Parameters)PluginProcessor::KNOBLOP;
        processorRef.uiToAudio.push(msg);
    }
    if (s == &HIGKnob) {
        PluginProcessor::UIToAudioMessage msg;
        msg.what = bv ? PluginProcessor::UIToAudioMessage::BEGIN_EDIT : PluginProcessor::UIToAudioMessage::END_EDIT;
        msg.which = (PluginProcessor::Parameters)PluginProcessor::KNOBHIG;
        processorRef.uiToAudio.push(msg);
    }
    if (s == &MIDKnob) {
        PluginProcessor::UIToAudioMessage msg;
        msg.what = bv ? PluginProcessor::UIToAudioMessage::BEGIN_EDIT : PluginProcessor::UIToAudioMessage::END_EDIT;
        msg.which = (PluginProcessor::Parameters)PluginProcessor::KNOBMID;
        processorRef.uiToAudio.push(msg);
    }
    if (s == &LOWKnob) {
        PluginProcessor::UIToAudioMessage msg;
        msg.what = bv ? PluginProcessor::UIToAudioMessage::BEGIN_EDIT : PluginProcessor::UIToAudioMessage::END_EDIT;
        msg.which = (PluginProcessor::Parameters)PluginProcessor::KNOBLOW;
        processorRef.uiToAudio.push(msg);
    }
    if (s == &THRKnob) {
        PluginProcessor::UIToAudioMessage msg;
        msg.what = bv ? PluginProcessor::UIToAudioMessage::BEGIN_EDIT : PluginProcessor::UIToAudioMessage::END_EDIT;
        msg.which = (PluginProcessor::Parameters)PluginProcessor::KNOBTHR;
        processorRef.uiToAudio.push(msg);
    }
    if (s == &PANKnob) {
        PluginProcessor::UIToAudioMessage msg;
        msg.what = bv ? PluginProcessor::UIToAudioMessage::BEGIN_EDIT : PluginProcessor::UIToAudioMessage::END_EDIT;
        msg.which = (PluginProcessor::Parameters)PluginProcessor::KNOBPAN;
        processorRef.uiToAudio.push(msg);
    }
    if (s == &FADKnob) {
        PluginProcessor::UIToAudioMessage msg;
        msg.what = bv ? PluginProcessor::UIToAudioMessage::BEGIN_EDIT : PluginProcessor::UIToAudioMessage::END_EDIT;
        msg.which = (PluginProcessor::Parameters)PluginProcessor::KNOBFAD;
        processorRef.uiToAudio.push(msg);
    }
}

void PluginEditor::updateTrackProperties() {
    auto opt = processorRef.trackProperties.colour;
    if (opt.has_value()) hostTrackColour = *opt;
    auto optB = processorRef.trackProperties.name;
    if (optB.has_value()) hostTrackName = *optB;
    repaint();
}

void PluginEditor::idle()
{
    PluginProcessor::AudioToUIMessage msg;
    bool repaintTS{false}; //we don't redraw interface just for getting data into the GUI section
    while (processorRef.audioToUI.pop(msg)) {
        switch (msg.what) {
        case PluginProcessor::AudioToUIMessage::NEW_VALUE:
                if (msg.which == PluginProcessor::KNOBHIG) {HIGKnob.setValue(msg.newValue, juce::NotificationType::dontSendNotification); break;}
                if (msg.which == PluginProcessor::KNOBMID) {MIDKnob.setValue(msg.newValue, juce::NotificationType::dontSendNotification); break;}
                if (msg.which == PluginProcessor::KNOBLOW) {LOWKnob.setValue(msg.newValue, juce::NotificationType::dontSendNotification); break;}
                if (msg.which == PluginProcessor::KNOBTHR) {THRKnob.setValue(msg.newValue, juce::NotificationType::dontSendNotification); break;}
                if (msg.which == PluginProcessor::KNOBHIP) {HIPKnob.setValue(msg.newValue, juce::NotificationType::dontSendNotification); break;}
                if (msg.which == PluginProcessor::KNOBLOP) {LOPKnob.setValue(msg.newValue, juce::NotificationType::dontSendNotification); break;}
                if (msg.which == PluginProcessor::KNOBPAN) {PANKnob.setValue(msg.newValue, juce::NotificationType::dontSendNotification); break;}
                if (msg.which == PluginProcessor::KNOBFAD) {FADKnob.setValue(msg.newValue, juce::NotificationType::dontSendNotification); break;}
                break; //this can grab the knobs away from the user! Should cause the knob to repaint, too.
                
        case PluginProcessor::AudioToUIMessage::RMS_LEFT: meter.pushA(msg.newValue); break;
        case PluginProcessor::AudioToUIMessage::RMS_RIGHT: meter.pushB(msg.newValue); break;
        case PluginProcessor::AudioToUIMessage::PEAK_LEFT: meter.pushC(msg.newValue); break;
        case PluginProcessor::AudioToUIMessage::PEAK_RIGHT: meter.pushD(msg.newValue); break;
        case PluginProcessor::AudioToUIMessage::SLEW_LEFT: meter.pushE(msg.newValue); break;
        case PluginProcessor::AudioToUIMessage::SLEW_RIGHT: meter.pushF(msg.newValue); break;
        case PluginProcessor::AudioToUIMessage::ZERO_LEFT: meter.pushG(msg.newValue); break;
        case PluginProcessor::AudioToUIMessage::ZERO_RIGHT: meter.pushH(msg.newValue); break;
        case PluginProcessor::AudioToUIMessage::BLINKEN_COMP: meter.pushI(msg.newValue); break;
        case PluginProcessor::AudioToUIMessage::BLINKEN_PAN: meter.pushJ(msg.newValue); break;
        case PluginProcessor::AudioToUIMessage::INCREMENT: //Increment is running at 24 FPS and giving the above calculations
                meter.pushIncrement(); repaintTS = true; //we will repaint GUI after doing the following
                
                THRKnob.setColour(juce::Slider::thumbColourId, juce::Colour::fromFloatRGBA (meter.blinkenComp*airwindowsLookAndFeel.LEDColour.getFloatRed(), meter.blinkenComp*airwindowsLookAndFeel.LEDColour.getFloatGreen(), meter.blinkenComp*airwindowsLookAndFeel.LEDColour.getFloatBlue(), 1.0f));
                PANKnob.setColour(juce::Slider::thumbColourId, juce::Colour::fromFloatRGBA (meter.blinkenPan*airwindowsLookAndFeel.LEDColour.getFloatRed(), meter.blinkenPan*airwindowsLookAndFeel.LEDColour.getFloatGreen(), meter.blinkenPan*airwindowsLookAndFeel.LEDColour.getFloatBlue(), 1.0f));

                //User color LEDS are done like this: choose the same meter.data selection for each, and then the color will always be the user color
                //SGTKnob.setColour(juce::Slider::thumbColourId, juce::Colour::fromFloatRGBA (meter.inputRMSL, meter.inputRMSL, meter.inputRMSL, 1.0f));
                //for an RGB or specified color blinken-knob, we don't reference user color, instead we just use the multiple meter.data directly without bringing in LEDColour
                
                //here is where we can make any control's thumb be a continuous blinkenlight with any value in meter. It runs at about 24fps.
                //We can do if statements etc. here, only thing we can NOT do is instantiate new variables. Do it as shown above and it works.
                //Also, this defaults to colors flashing against black, like LEDs: that's a good way to distinguish blinken-knobs from knobs
                break;
        default: break;
        } //end of switch statement for msg.what
    } if (repaintTS) meter.repaint();
}
