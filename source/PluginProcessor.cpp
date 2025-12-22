#include "PluginProcessor.h"
#include "PluginEditor.h"
#ifndef M_PI
#  define M_PI (3.14159265358979323846)
#endif
#ifndef M_PI_2
#  define M_PI_2 (1.57079632679489661923)
#endif

//==============================================================================
PluginProcessor::PluginProcessor():AudioProcessor (
                    BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
){
	for (int x = 0; x < biq_total; x++) {
		highFast[x] = 0.0;
		lowFast[x] = 0.0;
	}
	highFastLIIR = 0.0;
	highFastRIIR = 0.0;
	lowFastLIIR = 0.0;
	lowFastRIIR = 0.0;
	//SmoothEQ3
		
	for (int x = 0; x < bez_total; x++) {bezCompF[x] = 0.0;bezCompS[x] = 0.0;}
	bezCompF[bez_cycle] = 1.0;
	bezCompS[bez_cycle] = 1.0;
	//Dynamics2
	
	for (int x = 0; x < hilp_total; x++) {
		highpass[x] = 0.0;
		lowpass[x] = 0.0;
	}	
	
	for (int x = 0; x < 33; x++) {avg32L[x] = 0.0; avg32R[x] = 0.0;}
	for (int x = 0; x < 17; x++) {avg16L[x] = 0.0; avg16R[x] = 0.0;}
	for (int x = 0; x < 9; x++) {avg8L[x] = 0.0; avg8R[x] = 0.0;}
	for (int x = 0; x < 5; x++) {avg4L[x] = 0.0; avg4R[x] = 0.0;}
	for (int x = 0; x < 3; x++) {avg2L[x] = 0.0; avg2R[x] = 0.0;}
	avgPos = 0;
	lastSlewL = 0.0; lastSlewR = 0.0;
	lastSlewpleL = 0.0; lastSlewpleR = 0.0;
	//preTapeHack
	
	panA = 0.5; panB = 0.5;
	inTrimA = 0.5; inTrimB = 0.5;
	
    fpdL = 1.0; while (fpdL < 16386) fpdL = rand()*UINT32_MAX;
    fpdR = 1.0; while (fpdR < 16386) fpdR = rand()*UINT32_MAX;
    //this is reset: values being initialized only once. Startup values, whatever they are.

    // (internal ID, how it's shown in DAW generic view, {min, max}, default)
    addParameter(params[KNOBHIG] = new juce::AudioParameterFloat("high", "High", {0.0f, 1.0f}, 0.5f));                      params[KNOBHIG]->addListener(this);
    addParameter(params[KNOBMID] = new juce::AudioParameterFloat("mid", "Mid", {0.0f, 1.0f}, 0.5f));                        params[KNOBMID]->addListener(this);
    addParameter(params[KNOBLOW] = new juce::AudioParameterFloat("low", "Low", {0.0f, 1.0f}, 0.5f));                        params[KNOBLOW]->addListener(this);
    addParameter(params[KNOBTHR] = new juce::AudioParameterFloat("threshold", "Threshold", {0.0f, 1.0f}, 1.0f));            params[KNOBTHR]->addListener(this);
    addParameter(params[KNOBHIP] = new juce::AudioParameterFloat("highpass", "Highpass", {0.0f, 1.0f}, 0.0f));              params[KNOBHIP]->addListener(this);
    addParameter(params[KNOBLOP] = new juce::AudioParameterFloat("lowpass", "Lowpass", {0.0f, 1.0f}, 1.0f));                params[KNOBLOP]->addListener(this);
    addParameter(params[KNOBPAN] = new juce::AudioParameterFloat("pan", "Pan", {0.0f, 1.0f}, 0.5f));                        params[KNOBPAN]->addListener(this);
    addParameter(params[KNOBFAD] = new juce::AudioParameterFloat("fader", "Fader", {0.0f, 1.0f}, 0.5f));                    params[KNOBFAD]->addListener(this);
}

PluginProcessor::~PluginProcessor() {}
void PluginProcessor::parameterValueChanged(int parameterIndex, float newValue)
{
    AudioToUIMessage msg;
    msg.what = AudioToUIMessage::NEW_VALUE;
    msg.which = (PluginProcessor::Parameters)parameterIndex;
    msg.newValue = params[parameterIndex]->convertFrom0to1(newValue);
    audioToUI.push(msg);
}
void PluginProcessor::parameterGestureChanged(int parameterIndex, bool starting) {}
const juce::String PluginProcessor::getName() const {return JucePlugin_Name;}
bool PluginProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}
bool PluginProcessor::supportsDoublePrecisionProcessing() const
{
   #if JucePlugin_SupportsDoublePrecisionProcessing
    return true;
   #else
    return true;
    //note: I don't know whether that config option is set, so I'm hardcoding it
    //knowing I have enabled such support: keeping the boilerplate stuff tho
    //in case I can sort out where it's enabled as a flag
   #endif
}
bool PluginProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}
bool PluginProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}
double PluginProcessor::getTailLengthSeconds() const {return 0.0;}
int PluginProcessor::getNumPrograms() {return 1;}
int PluginProcessor::getCurrentProgram() {return 0;}
void PluginProcessor::setCurrentProgram (int index) {juce::ignoreUnused (index);}
const juce::String PluginProcessor::getProgramName (int index) {juce::ignoreUnused (index); return {};}
void PluginProcessor::changeProgramName (int index, const juce::String& newName) {juce::ignoreUnused (index, newName);}

//==============================================================================
void PluginProcessor::prepareToPlay (double sampleRate, int samplesPerBlock) {juce::ignoreUnused (sampleRate, samplesPerBlock);}
void PluginProcessor::releaseResources() {}
bool PluginProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
  #else
    // This is the place where you check if the layout is supported.
    // In this metering code we only support stereo.
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    // This checks if the input layout matches the output layout
   #if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
   #endif

    return true;
  #endif
}

//==============================================================================

bool PluginProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* PluginProcessor::createEditor()
{
    return new PluginEditor (*this);
}

//==============================================================================
void PluginProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
    std::unique_ptr<juce::XmlElement> xml(new juce::XmlElement("consolehbuss"));
    xml->setAttribute("streamingVersion", (int)8524);

    for (int i = 0; i < n_params; ++i)
    {
        juce::String nm = juce::String("awchb_") + std::to_string(i);
        float val = 0.0f; if (i < n_params) val = *(params[i]);
        xml->setAttribute(nm, val);
    }
    copyXmlToBinary(*xml, destData);
}

void PluginProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
    std::unique_ptr<juce::XmlElement> xmlState(getXmlFromBinary(data, sizeInBytes));

    if (xmlState.get() != nullptr)
    {
        if (xmlState->hasTagName("consolehbuss"))
        {
            for (int i = 0; i < n_params; ++i)
            {
                juce::String nm = juce::String("awchb_") + std::to_string(i);
                auto f = xmlState->getDoubleAttribute(nm);
                params[i]->setValueNotifyingHost(f);
            }
        }
        updateHostDisplay();
    }
    //These functions are adapted (simplified) from baconpaul's airwin2rack and all thanks to getting
    //it working shall go there, though sudara or anyone could've spotted that I hadn't done these.
    //baconpaul pointed me to the working versions in airwin2rack, that I needed to see.
}

void PluginProcessor::updateTrackProperties(const TrackProperties& properties)
{
    trackProperties = properties;
    // call the verison in the editor to update there
    if (auto* editor = dynamic_cast<PluginEditor*> (getActiveEditor()))
        editor->updateTrackProperties();
}


//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new PluginProcessor();
}


void PluginProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ignoreUnused (midiMessages);
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();
    // In case we have more outputs than inputs, this code clears any output
    // channels that didn't contain input data, (because these aren't
    // guaranteed to be empty - they may contain garbage).
    // This is here to avoid people getting screaming feedback
    // when they first compile a plugin, but obviously you don't need to keep
    // this code if your algorithm always overwrites all the output channels.
    
    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());
    
    if (!(getBus(false, 0)->isEnabled() && getBus(true, 0)->isEnabled())) return;
    auto mainOutput = getBusBuffer(buffer, false, 0); //if we have audio busses at all,
    auto mainInput = getBusBuffer(buffer, true, 0); //they're now mainOutput and mainInput.
    
    UIToAudioMessage uim;
    while (uiToAudio.pop(uim)) {
        switch (uim.what) {
        case UIToAudioMessage::NEW_VALUE: params[uim.which]->setValueNotifyingHost(params[uim.which]->convertTo0to1(uim.newValue)); break;
        case UIToAudioMessage::BEGIN_EDIT: params[uim.which]->beginChangeGesture(); break;
        case UIToAudioMessage::END_EDIT: params[uim.which]->endChangeGesture(); break;
        }
    } //Handle inbound messages from the UI thread

    double rmsSize = (1881.0 / 44100.0)*getSampleRate(); //higher is slower with larger RMS buffers
    double zeroCrossScale = (1.0 / getSampleRate())*44100.0;
    int inFramesToProcess = buffer.getNumSamples(); //vst doesn't give us this as a separate variable so we'll make it
    double overallscale = 1.0;
    overallscale /= 44100.0;
    overallscale *= getSampleRate();

	int spacing = floor(overallscale*2.0);
	if (spacing < 2) spacing = 2; if (spacing > 32) spacing = 32;
	
	double trebleGain = (params[KNOBHIG]->get()-0.5)*2.0;
	trebleGain = 1.0+(trebleGain*fabs(trebleGain)*fabs(trebleGain));
	double midGain = (params[KNOBMID]->get()-0.5)*2.0;
	midGain = 1.0+(midGain*fabs(midGain)*fabs(midGain));
	double bassGain = (params[KNOBLOW]->get()-0.5)*2.0;
	bassGain = 1.0+(bassGain*fabs(bassGain)*fabs(bassGain));
	//separate from filtering stage, this is amplitude, centered on 1.0 unity gain
	double highCoef = 0.0;
	double lowCoef = 0.0;
	double omega = 0.0;
	double biqK = 0.0;
	double norm = 0.0;
	
	bool eqOff = (trebleGain == 1.0 && midGain == 1.0 && bassGain == 1.0);
	//we get to completely bypass EQ if we're truly not using it. The mechanics of it mean that
	//it cancels out to bit-identical anyhow, but we get to skip the calculation
	if (!eqOff) {
		//SmoothEQ3 is how to get 3rd order steepness at very low CPU.
		//because sample rate varies, you could also vary the crossovers
		//you can't vary Q because math is simplified to take advantage of
		//how the accurate Q value for this filter is always exactly 1.0.
		highFast[biq_freq] = (4000.0/getSampleRate());
		omega = 2.0*M_PI*(4000.0/getSampleRate()); //mid-high crossover freq
		biqK = 2.0 - cos(omega);
		highCoef = -sqrt(biqK*biqK - 1.0) + biqK;
		lowFast[biq_freq] = (200.0/getSampleRate());
		omega = 2.0*M_PI*(200.0/getSampleRate()); //low-mid crossover freq
		biqK = 2.0 - cos(omega);
		lowCoef = -sqrt(biqK*biqK - 1.0) + biqK;
		//exponential IIR filter as part of an accurate 3rd order Butterworth filter 
		biqK = tan(M_PI * highFast[biq_freq]);
		norm = 1.0 / (1.0 + biqK + biqK*biqK);
		highFast[biq_a0] = biqK * biqK * norm;
		highFast[biq_a1] = 2.0 * highFast[biq_a0];
		highFast[biq_a2] = highFast[biq_a0];
		highFast[biq_b1] = 2.0 * (biqK*biqK - 1.0) * norm;
		highFast[biq_b2] = (1.0 - biqK + biqK*biqK) * norm;
		biqK = tan(M_PI * lowFast[biq_freq]);
		norm = 1.0 / (1.0 + biqK + biqK*biqK);
		lowFast[biq_a0] = biqK * biqK * norm;
		lowFast[biq_a1] = 2.0 * lowFast[biq_a0];
		lowFast[biq_a2] = lowFast[biq_a0];
		lowFast[biq_b1] = 2.0 * (biqK*biqK - 1.0) * norm;
		lowFast[biq_b2] = (1.0 - biqK + biqK*biqK) * norm;
		//custom biquad setup with Q = 1.0 gets to omit some divides
	}
	//SmoothEQ3
	
	double bezCThresh = pow(1.0-params[KNOBTHR]->get(), 6.0) * 8.0;
	double bezRez = pow(1.0-params[KNOBTHR]->get(), 12.360679774997898) / overallscale;
	double sloRez = pow(1.0-params[KNOBTHR]->get(),10.0) / overallscale;
	sloRez = fmin(fmax(sloRez,0.00001),1.0);
	bezRez = fmin(fmax(bezRez,0.00001),1.0);
	//Dynamics2
	
	highpass[hilp_freq] = ((pow(params[KNOBHIP]->get(),3)*24000.0)+10.0)/getSampleRate();
	if (highpass[hilp_freq] > 0.495) highpass[hilp_freq] = 0.495;
	bool highpassEngage = true; if (params[KNOBHIP]->get() == 0.0) highpassEngage = false;
	
	lowpass[hilp_freq] = ((pow(params[KNOBLOP]->get(),3)*24000.0)+10.0)/getSampleRate();
	if (lowpass[hilp_freq] > 0.495) lowpass[hilp_freq] = 0.495;
	bool lowpassEngage = true; if (params[KNOBLOP]->get() == 1.0) lowpassEngage = false;
	
	highpass[hilp_aA0] = highpass[hilp_aB0];
	highpass[hilp_aA1] = highpass[hilp_aB1];
	highpass[hilp_bA1] = highpass[hilp_bB1];
	highpass[hilp_bA2] = highpass[hilp_bB2];
	highpass[hilp_cA0] = highpass[hilp_cB0];
	highpass[hilp_cA1] = highpass[hilp_cB1];
	highpass[hilp_dA1] = highpass[hilp_dB1];
	highpass[hilp_dA2] = highpass[hilp_dB2];
	highpass[hilp_eA0] = highpass[hilp_eB0];
	highpass[hilp_eA1] = highpass[hilp_eB1];
	highpass[hilp_fA1] = highpass[hilp_fB1];
	highpass[hilp_fA2] = highpass[hilp_fB2];
	lowpass[hilp_aA0] = lowpass[hilp_aB0];
	lowpass[hilp_aA1] = lowpass[hilp_aB1];
	lowpass[hilp_bA1] = lowpass[hilp_bB1];
	lowpass[hilp_bA2] = lowpass[hilp_bB2];
	lowpass[hilp_cA0] = lowpass[hilp_cB0];
	lowpass[hilp_cA1] = lowpass[hilp_cB1];
	lowpass[hilp_dA1] = lowpass[hilp_dB1];
	lowpass[hilp_dA2] = lowpass[hilp_dB2];
	lowpass[hilp_eA0] = lowpass[hilp_eB0];
	lowpass[hilp_eA1] = lowpass[hilp_eB1];
	lowpass[hilp_fA1] = lowpass[hilp_fB1];
	lowpass[hilp_fA2] = lowpass[hilp_fB2];
	//previous run through the buffer is still in the filter, so we move it
	//to the A section and now it's the new starting point.
	//On the buss, highpass and lowpass are isolators meant to be moved,
	//so they are interpolated where the channels are not
	
	biqK = tan(M_PI * highpass[hilp_freq]); //highpass
	norm = 1.0 / (1.0 + biqK / 1.93185165 + biqK * biqK);
	highpass[hilp_aB0] = norm;
	highpass[hilp_aB1] = -2.0 * highpass[hilp_aB0];
	highpass[hilp_bB1] = 2.0 * (biqK * biqK - 1.0) * norm;
	highpass[hilp_bB2] = (1.0 - biqK / 1.93185165 + biqK * biqK) * norm;
	norm = 1.0 / (1.0 + biqK / 0.70710678 + biqK * biqK);
	highpass[hilp_cB0] = norm;
	highpass[hilp_cB1] = -2.0 * highpass[hilp_cB0];
	highpass[hilp_dB1] = 2.0 * (biqK * biqK - 1.0) * norm;
	highpass[hilp_dB2] = (1.0 - biqK / 0.70710678 + biqK * biqK) * norm;
	norm = 1.0 / (1.0 + biqK / 0.51763809 + biqK * biqK);
	highpass[hilp_eB0] = norm;
	highpass[hilp_eB1] = -2.0 * highpass[hilp_eB0];
	highpass[hilp_fB1] = 2.0 * (biqK * biqK - 1.0) * norm;
	highpass[hilp_fB2] = (1.0 - biqK / 0.51763809 + biqK * biqK) * norm;
	
	biqK = tan(M_PI * lowpass[hilp_freq]); //lowpass
	norm = 1.0 / (1.0 + biqK / 1.93185165 + biqK * biqK);
	lowpass[hilp_aB0] = biqK * biqK * norm;
	lowpass[hilp_aB1] = 2.0 * lowpass[hilp_aB0];
	lowpass[hilp_bB1] = 2.0 * (biqK * biqK - 1.0) * norm;
	lowpass[hilp_bB2] = (1.0 - biqK / 1.93185165 + biqK * biqK) * norm;
	norm = 1.0 / (1.0 + biqK / 0.70710678 + biqK * biqK);
	lowpass[hilp_cB0] = biqK * biqK * norm;
	lowpass[hilp_cB1] = 2.0 * lowpass[hilp_cB0];
	lowpass[hilp_dB1] = 2.0 * (biqK * biqK - 1.0) * norm;
	lowpass[hilp_dB2] = (1.0 - biqK / 0.70710678 + biqK * biqK) * norm;
	norm = 1.0 / (1.0 + biqK / 0.51763809 + biqK * biqK);
	lowpass[hilp_eB0] = biqK * biqK * norm;
	lowpass[hilp_eB1] = 2.0 * lowpass[hilp_eB0];
	lowpass[hilp_fB1] = 2.0 * (biqK * biqK - 1.0) * norm;
	lowpass[hilp_fB2] = (1.0 - biqK / 0.51763809 + biqK * biqK) * norm;
	
	if (highpass[hilp_aA0] == 0.0) { // if we have just started, start directly with raw info
		highpass[hilp_aA0] = highpass[hilp_aB0];
		highpass[hilp_aA1] = highpass[hilp_aB1];
		highpass[hilp_bA1] = highpass[hilp_bB1];
		highpass[hilp_bA2] = highpass[hilp_bB2];
		highpass[hilp_cA0] = highpass[hilp_cB0];
		highpass[hilp_cA1] = highpass[hilp_cB1];
		highpass[hilp_dA1] = highpass[hilp_dB1];
		highpass[hilp_dA2] = highpass[hilp_dB2];
		highpass[hilp_eA0] = highpass[hilp_eB0];
		highpass[hilp_eA1] = highpass[hilp_eB1];
		highpass[hilp_fA1] = highpass[hilp_fB1];
		highpass[hilp_fA2] = highpass[hilp_fB2];
		lowpass[hilp_aA0] = lowpass[hilp_aB0];
		lowpass[hilp_aA1] = lowpass[hilp_aB1];
		lowpass[hilp_bA1] = lowpass[hilp_bB1];
		lowpass[hilp_bA2] = lowpass[hilp_bB2];
		lowpass[hilp_cA0] = lowpass[hilp_cB0];
		lowpass[hilp_cA1] = lowpass[hilp_cB1];
		lowpass[hilp_dA1] = lowpass[hilp_dB1];
		lowpass[hilp_dA2] = lowpass[hilp_dB2];
		lowpass[hilp_eA0] = lowpass[hilp_eB0];
		lowpass[hilp_eA1] = lowpass[hilp_eB1];
		lowpass[hilp_fA1] = lowpass[hilp_fB1];
		lowpass[hilp_fA2] = lowpass[hilp_fB2];
	}
	
	panA = panB; panB = params[KNOBPAN]->get()*1.57079633;
	inTrimA = inTrimB; inTrimB = params[KNOBFAD]->get()*2.0;
	//Console
    
    for (int i = 0; i < buffer.getNumSamples(); ++i)
    {
        auto outL = mainOutput.getWritePointer(0, i);
        auto outR = mainOutput.getWritePointer(1, i);
        auto inL = mainInput.getReadPointer(0, i); //in isBussesLayoutSupported, we have already
        auto inR = mainInput.getReadPointer(1, i); //specified that we can only be stereo and never mono
        long double inputSampleL = *inL;
        long double inputSampleR = *inR;
        if (fabs(inputSampleL)<1.18e-23) inputSampleL = fpdL * 1.18e-17;
        if (fabs(inputSampleR)<1.18e-23) inputSampleR = fpdR * 1.18e-17;
        //NOTE: I don't yet know whether counting the buffer backwards means that gainA and gainB must be reversed.
        //If the audio flow is in fact reversed we must swap the temp and (1.0-temp) and I have done this provisionally
        //Original VST2 is counting DOWN with -- operator, but this counts UP with ++
        //If this doesn't create zipper noise on sine processing then it's correct
		
		const double temp = (double)i/inFramesToProcess;
		highpass[hilp_a0] = (highpass[hilp_aA0]*temp)+(highpass[hilp_aB0]*(1.0-temp));
		highpass[hilp_a1] = (highpass[hilp_aA1]*temp)+(highpass[hilp_aB1]*(1.0-temp));
		highpass[hilp_b1] = (highpass[hilp_bA1]*temp)+(highpass[hilp_bB1]*(1.0-temp));
		highpass[hilp_b2] = (highpass[hilp_bA2]*temp)+(highpass[hilp_bB2]*(1.0-temp));
		highpass[hilp_c0] = (highpass[hilp_cA0]*temp)+(highpass[hilp_cB0]*(1.0-temp));
		highpass[hilp_c1] = (highpass[hilp_cA1]*temp)+(highpass[hilp_cB1]*(1.0-temp));
		highpass[hilp_d1] = (highpass[hilp_dA1]*temp)+(highpass[hilp_dB1]*(1.0-temp));
		highpass[hilp_d2] = (highpass[hilp_dA2]*temp)+(highpass[hilp_dB2]*(1.0-temp));
		highpass[hilp_e0] = (highpass[hilp_eA0]*temp)+(highpass[hilp_eB0]*(1.0-temp));
		highpass[hilp_e1] = (highpass[hilp_eA1]*temp)+(highpass[hilp_eB1]*(1.0-temp));
		highpass[hilp_f1] = (highpass[hilp_fA1]*temp)+(highpass[hilp_fB1]*(1.0-temp));
		highpass[hilp_f2] = (highpass[hilp_fA2]*temp)+(highpass[hilp_fB2]*(1.0-temp));
		lowpass[hilp_a0] = (lowpass[hilp_aA0]*temp)+(lowpass[hilp_aB0]*(1.0-temp));
		lowpass[hilp_a1] = (lowpass[hilp_aA1]*temp)+(lowpass[hilp_aB1]*(1.0-temp));
		lowpass[hilp_b1] = (lowpass[hilp_bA1]*temp)+(lowpass[hilp_bB1]*(1.0-temp));
		lowpass[hilp_b2] = (lowpass[hilp_bA2]*temp)+(lowpass[hilp_bB2]*(1.0-temp));
		lowpass[hilp_c0] = (lowpass[hilp_cA0]*temp)+(lowpass[hilp_cB0]*(1.0-temp));
		lowpass[hilp_c1] = (lowpass[hilp_cA1]*temp)+(lowpass[hilp_cB1]*(1.0-temp));
		lowpass[hilp_d1] = (lowpass[hilp_dA1]*temp)+(lowpass[hilp_dB1]*(1.0-temp));
		lowpass[hilp_d2] = (lowpass[hilp_dA2]*temp)+(lowpass[hilp_dB2]*(1.0-temp));
		lowpass[hilp_e0] = (lowpass[hilp_eA0]*temp)+(lowpass[hilp_eB0]*(1.0-temp));
		lowpass[hilp_e1] = (lowpass[hilp_eA1]*temp)+(lowpass[hilp_eB1]*(1.0-temp));
		lowpass[hilp_f1] = (lowpass[hilp_fA1]*temp)+(lowpass[hilp_fB1]*(1.0-temp));
		lowpass[hilp_f2] = (lowpass[hilp_fA2]*temp)+(lowpass[hilp_fB2]*(1.0-temp));
		
		
		if (highpassEngage) { //distributed Highpass
			highpass[hilp_temp] = (inputSampleL*highpass[hilp_a0])+highpass[hilp_aL1];
			highpass[hilp_aL1] = (inputSampleL*highpass[hilp_a1])-(highpass[hilp_temp]*highpass[hilp_b1])+highpass[hilp_aL2];
			highpass[hilp_aL2] = (inputSampleL*highpass[hilp_a0])-(highpass[hilp_temp]*highpass[hilp_b2]); inputSampleL = highpass[hilp_temp];
			highpass[hilp_temp] = (inputSampleR*highpass[hilp_a0])+highpass[hilp_aR1];
			highpass[hilp_aR1] = (inputSampleR*highpass[hilp_a1])-(highpass[hilp_temp]*highpass[hilp_b1])+highpass[hilp_aR2];
			highpass[hilp_aR2] = (inputSampleR*highpass[hilp_a0])-(highpass[hilp_temp]*highpass[hilp_b2]); inputSampleR = highpass[hilp_temp];
		} else highpass[hilp_aR1] = highpass[hilp_aR2] = highpass[hilp_aL1] = highpass[hilp_aL2] = 0.0;
		if (lowpassEngage) { //distributed Lowpass
			lowpass[hilp_temp] = (inputSampleL*lowpass[hilp_a0])+lowpass[hilp_aL1];
			lowpass[hilp_aL1] = (inputSampleL*lowpass[hilp_a1])-(lowpass[hilp_temp]*lowpass[hilp_b1])+lowpass[hilp_aL2];
			lowpass[hilp_aL2] = (inputSampleL*lowpass[hilp_a0])-(lowpass[hilp_temp]*lowpass[hilp_b2]); inputSampleL = lowpass[hilp_temp];
			lowpass[hilp_temp] = (inputSampleR*lowpass[hilp_a0])+lowpass[hilp_aR1];
			lowpass[hilp_aR1] = (inputSampleR*lowpass[hilp_a1])-(lowpass[hilp_temp]*lowpass[hilp_b1])+lowpass[hilp_aR2];
			lowpass[hilp_aR2] = (inputSampleR*lowpass[hilp_a0])-(lowpass[hilp_temp]*lowpass[hilp_b2]); inputSampleR = lowpass[hilp_temp];
		} else lowpass[hilp_aR1] = lowpass[hilp_aR2] = lowpass[hilp_aL1] = lowpass[hilp_aL2] = 0.0;
		//first Highpass/Lowpass blocks aliasing before the nonlinearity of ConsoleXBuss and Parametric		
		
		if (inputSampleL > 1.0) inputSampleL = 1.0;
		else if (inputSampleL > 0.0) inputSampleL = -expm1((log1p(-inputSampleL) * 0.6180339887498949));
		if (inputSampleL < -1.0) inputSampleL = -1.0;
		else if (inputSampleL < 0.0) inputSampleL = expm1((log1p(inputSampleL) * 0.6180339887498949));
		
		if (inputSampleR > 1.0) inputSampleR = 1.0;
		else if (inputSampleR > 0.0) inputSampleR = -expm1((log1p(-inputSampleR) * 0.6180339887498949));
		if (inputSampleR < -1.0) inputSampleR = -1.0;
		else if (inputSampleR < 0.0) inputSampleR = expm1((log1p(inputSampleR) * 0.6180339887498949));
		
		if (highpassEngage) { //distributed Highpass
			highpass[hilp_temp] = (inputSampleL*highpass[hilp_c0])+highpass[hilp_cL1];
			highpass[hilp_cL1] = (inputSampleL*highpass[hilp_c1])-(highpass[hilp_temp]*highpass[hilp_d1])+highpass[hilp_cL2];
			highpass[hilp_cL2] = (inputSampleL*highpass[hilp_c0])-(highpass[hilp_temp]*highpass[hilp_d2]); inputSampleL = highpass[hilp_temp];
			highpass[hilp_temp] = (inputSampleR*highpass[hilp_c0])+highpass[hilp_cR1];
			highpass[hilp_cR1] = (inputSampleR*highpass[hilp_c1])-(highpass[hilp_temp]*highpass[hilp_d1])+highpass[hilp_cR2];
			highpass[hilp_cR2] = (inputSampleR*highpass[hilp_c0])-(highpass[hilp_temp]*highpass[hilp_d2]); inputSampleR = highpass[hilp_temp];
		} else highpass[hilp_cR1] = highpass[hilp_cR2] = highpass[hilp_cL1] = highpass[hilp_cL2] = 0.0;
		if (lowpassEngage) { //distributed Lowpass
			lowpass[hilp_temp] = (inputSampleL*lowpass[hilp_c0])+lowpass[hilp_cL1];
			lowpass[hilp_cL1] = (inputSampleL*lowpass[hilp_c1])-(lowpass[hilp_temp]*lowpass[hilp_d1])+lowpass[hilp_cL2];
			lowpass[hilp_cL2] = (inputSampleL*lowpass[hilp_c0])-(lowpass[hilp_temp]*lowpass[hilp_d2]); inputSampleL = lowpass[hilp_temp];
			lowpass[hilp_temp] = (inputSampleR*lowpass[hilp_c0])+lowpass[hilp_cR1];
			lowpass[hilp_cR1] = (inputSampleR*lowpass[hilp_c1])-(lowpass[hilp_temp]*lowpass[hilp_d1])+lowpass[hilp_cR2];
			lowpass[hilp_cR2] = (inputSampleR*lowpass[hilp_c0])-(lowpass[hilp_temp]*lowpass[hilp_d2]); inputSampleR = lowpass[hilp_temp];
		} else lowpass[hilp_cR1] = lowpass[hilp_cR2] = lowpass[hilp_cL1] = lowpass[hilp_cL2] = 0.0;
		//another stage of Highpass/Lowpass before bringing in the parametric bands		
		
		if (!eqOff) {
			double trebleFastL = inputSampleL;		
			double outSample = (trebleFastL * highFast[biq_a0]) + highFast[biq_sL1];
			highFast[biq_sL1] = (trebleFastL * highFast[biq_a1]) - (outSample * highFast[biq_b1]) + highFast[biq_sL2];
			highFast[biq_sL2] = (trebleFastL * highFast[biq_a2]) - (outSample * highFast[biq_b2]);
			double midFastL = outSample; trebleFastL -= midFastL;
			outSample = (midFastL * lowFast[biq_a0]) + lowFast[biq_sL1];
			lowFast[biq_sL1] = (midFastL * lowFast[biq_a1]) - (outSample * lowFast[biq_b1]) + lowFast[biq_sL2];
			lowFast[biq_sL2] = (midFastL * lowFast[biq_a2]) - (outSample * lowFast[biq_b2]);
			double bassFastL = outSample; midFastL -= bassFastL;
			trebleFastL = (bassFastL*bassGain) + (midFastL*midGain) + (trebleFastL*trebleGain);
			//first stage of two crossovers is biquad of exactly 1.0 Q
			highFastLIIR = (highFastLIIR*highCoef) + (trebleFastL*(1.0-highCoef));
			midFastL = highFastLIIR; trebleFastL -= midFastL;
			lowFastLIIR = (lowFastLIIR*lowCoef) + (midFastL*(1.0-lowCoef));
			bassFastL = lowFastLIIR; midFastL -= bassFastL;
			inputSampleL = (bassFastL*bassGain) + (midFastL*midGain) + (trebleFastL*trebleGain);		
			//second stage of two crossovers is the exponential filters
			//this produces a slightly steeper Butterworth filter very cheaply
			double trebleFastR = inputSampleR;		
			outSample = (trebleFastR * highFast[biq_a0]) + highFast[biq_sR1];
			highFast[biq_sR1] = (trebleFastR * highFast[biq_a1]) - (outSample * highFast[biq_b1]) + highFast[biq_sR2];
			highFast[biq_sR2] = (trebleFastR * highFast[biq_a2]) - (outSample * highFast[biq_b2]);
			double midFastR = outSample; trebleFastR -= midFastR;
			outSample = (midFastR * lowFast[biq_a0]) + lowFast[biq_sR1];
			lowFast[biq_sR1] = (midFastR * lowFast[biq_a1]) - (outSample * lowFast[biq_b1]) + lowFast[biq_sR2];
			lowFast[biq_sR2] = (midFastR * lowFast[biq_a2]) - (outSample * lowFast[biq_b2]);
			double bassFastR = outSample; midFastR -= bassFastR;
			trebleFastR = (bassFastR*bassGain) + (midFastR*midGain) + (trebleFastR*trebleGain);
			//first stage of two crossovers is biquad of exactly 1.0 Q
			highFastRIIR = (highFastRIIR*highCoef) + (trebleFastR*(1.0-highCoef));
			midFastR = highFastRIIR; trebleFastR -= midFastR;
			lowFastRIIR = (lowFastRIIR*lowCoef) + (midFastR*(1.0-lowCoef));
			bassFastR = lowFastRIIR; midFastR -= bassFastR;
			inputSampleR = (bassFastR*bassGain) + (midFastR*midGain) + (trebleFastR*trebleGain);		
			//second stage of two crossovers is the exponential filters
			//this produces a slightly steeper Butterworth filter very cheaply
		}
		//SmoothEQ3
		
		if (baseComp < fabs(inputSampleR*64.0)) baseComp = fmin(fabs(inputSampleR*64.0),1.0);
        if (baseComp < fabs(inputSampleL*64.0)) baseComp = fmin(fabs(inputSampleL*64.0),1.0);
		
		if (bezCThresh > 0.0) {
			inputSampleL *= ((bezCThresh*0.5)+1.0);
			inputSampleR *= ((bezCThresh*0.5)+1.0);
			bezCompF[bez_cycle] += bezRez;
			bezCompF[bez_Ctrl] += (fmax(fabs(inputSampleL),fabs(inputSampleR)) * bezRez);
			if (bezCompF[bez_cycle] > 1.0) {
				bezCompF[bez_cycle] -= 1.0;
				bezCompF[bez_C] = bezCompF[bez_B];
				bezCompF[bez_B] = bezCompF[bez_A];
				bezCompF[bez_A] = bezCompF[bez_Ctrl];
				bezCompF[bez_Ctrl] = 0.0;
			}
			bezCompS[bez_cycle] += sloRez;
			bezCompS[bez_Ctrl] += (fmax(fabs(inputSampleL),fabs(inputSampleR)) * sloRez);
			if (bezCompS[bez_cycle] > 1.0) {
				bezCompS[bez_cycle] -= 1.0;
				bezCompS[bez_C] = bezCompS[bez_B];
				bezCompS[bez_B] = bezCompS[bez_A];
				bezCompS[bez_A] = bezCompS[bez_Ctrl];
				bezCompS[bez_Ctrl] = 0.0;
			}
			double CBF = (bezCompF[bez_C]*(1.0-bezCompF[bez_cycle]))+(bezCompF[bez_B]*bezCompF[bez_cycle]);
			double BAF = (bezCompF[bez_B]*(1.0-bezCompF[bez_cycle]))+(bezCompF[bez_A]*bezCompF[bez_cycle]);
			double CBAF = (bezCompF[bez_B]+(CBF*(1.0-bezCompF[bez_cycle]))+(BAF*bezCompF[bez_cycle]))*0.5;
			double CBS = (bezCompS[bez_C]*(1.0-bezCompS[bez_cycle]))+(bezCompS[bez_B]*bezCompS[bez_cycle]);
			double BAS = (bezCompS[bez_B]*(1.0-bezCompS[bez_cycle]))+(bezCompS[bez_A]*bezCompS[bez_cycle]);
			double CBAS = (bezCompS[bez_B]+(CBS*(1.0-bezCompS[bez_cycle]))+(BAS*bezCompS[bez_cycle]))*0.5;
			double CBAMax = fmax(CBAS,CBAF); if (CBAMax > 0.0) CBAMax = 1.0/CBAMax;
			double CBAFade = ((CBAS*-CBAMax)+(CBAF*CBAMax)+1.0)*0.5;
			inputSampleL *= 1.0-(fmin(((CBAS*(1.0-CBAFade))+(CBAF*CBAFade))*bezCThresh,1.0));
			inputSampleR *= 1.0-(fmin(((CBAS*(1.0-CBAFade))+(CBAF*CBAFade))*bezCThresh,1.0));
            if (maxComp > pow(1.0-fmin(CBAS,CBAF),4.0)) maxComp = pow(1.0-fmin(CBAS,CBAF),4.0);
		} else {bezCompF[bez_Ctrl] = 0.0; bezCompS[bez_Ctrl] = 0.0;}
		//Dynamics2 custom version for buss
		

		if (highpassEngage) { //distributed Highpass
			highpass[hilp_temp] = (inputSampleL*highpass[hilp_e0])+highpass[hilp_eL1];
			highpass[hilp_eL1] = (inputSampleL*highpass[hilp_e1])-(highpass[hilp_temp]*highpass[hilp_f1])+highpass[hilp_eL2];
			highpass[hilp_eL2] = (inputSampleL*highpass[hilp_e0])-(highpass[hilp_temp]*highpass[hilp_f2]); inputSampleL = highpass[hilp_temp];
			highpass[hilp_temp] = (inputSampleR*highpass[hilp_e0])+highpass[hilp_eR1];
			highpass[hilp_eR1] = (inputSampleR*highpass[hilp_e1])-(highpass[hilp_temp]*highpass[hilp_f1])+highpass[hilp_eR2];
			highpass[hilp_eR2] = (inputSampleR*highpass[hilp_e0])-(highpass[hilp_temp]*highpass[hilp_f2]); inputSampleR = highpass[hilp_temp];
		} else highpass[hilp_eR1] = highpass[hilp_eR2] = highpass[hilp_eL1] = highpass[hilp_eL2] = 0.0;
		if (lowpassEngage) { //distributed Lowpass
			lowpass[hilp_temp] = (inputSampleL*lowpass[hilp_e0])+lowpass[hilp_eL1];
			lowpass[hilp_eL1] = (inputSampleL*lowpass[hilp_e1])-(lowpass[hilp_temp]*lowpass[hilp_f1])+lowpass[hilp_eL2];
			lowpass[hilp_eL2] = (inputSampleL*lowpass[hilp_e0])-(lowpass[hilp_temp]*lowpass[hilp_f2]); inputSampleL = lowpass[hilp_temp];
			lowpass[hilp_temp] = (inputSampleR*lowpass[hilp_e0])+lowpass[hilp_eR1];
			lowpass[hilp_eR1] = (inputSampleR*lowpass[hilp_e1])-(lowpass[hilp_temp]*lowpass[hilp_f1])+lowpass[hilp_eR2];
			lowpass[hilp_eR2] = (inputSampleR*lowpass[hilp_e0])-(lowpass[hilp_temp]*lowpass[hilp_f2]); inputSampleR = lowpass[hilp_temp];
		} else lowpass[hilp_eR1] = lowpass[hilp_eR2] = lowpass[hilp_eL1] = lowpass[hilp_eL2] = 0.0;		
		//final Highpass/Lowpass continues to address aliasing
		//final stacked biquad section is the softest Q for smoothness

		double gainR = (panA*temp)+(panB*(1.0-temp));
		double gainL = 1.57079633-gainR;
		gainR = sin(gainR); gainL = sin(gainL);
		double gain = (inTrimA*temp)+(inTrimB*(1.0-temp));
		if (gain > 1.0) gain *= gain;
		if (gain < 1.0) gain = 1.0-pow(1.0-gain,2);
		gain *= 2.0;
		
		inputSampleL = inputSampleL * gainL * gain;
		inputSampleR = inputSampleR * gainR * gain;
		//applies pan section, and smoothed fader gain
        
        double unclippedL = inputSampleL*inputSampleL;
        double unclippedR = inputSampleR*inputSampleR;

		double darkSampleL = inputSampleL;
		double darkSampleR = inputSampleR;
		if (avgPos > 31) avgPos = 0;
		if (spacing > 31) {
			avg32L[avgPos] = darkSampleL; avg32R[avgPos] = darkSampleR;
			darkSampleL = 0.0; darkSampleR = 0.0;
			for (int x = 0; x < 32; x++) {darkSampleL += avg32L[x]; darkSampleR += avg32R[x];}
			darkSampleL /= 32.0; darkSampleR /= 32.0;
		} if (spacing > 15) {
			avg16L[avgPos%16] = darkSampleL; avg16R[avgPos%16] = darkSampleR;
			darkSampleL = 0.0; darkSampleR = 0.0;
			for (int x = 0; x < 16; x++) {darkSampleL += avg16L[x]; darkSampleR += avg16R[x];}
			darkSampleL /= 16.0; darkSampleR /= 16.0;
		} if (spacing > 7) {
			avg8L[avgPos%8] = darkSampleL; avg8R[avgPos%8] = darkSampleR;
			darkSampleL = 0.0; darkSampleR = 0.0;
			for (int x = 0; x < 8; x++) {darkSampleL += avg8L[x]; darkSampleR += avg8R[x];}
			darkSampleL /= 8.0; darkSampleR /= 8.0;
		} if (spacing > 3) {
			avg4L[avgPos%4] = darkSampleL; avg4R[avgPos%4] = darkSampleR;
			darkSampleL = 0.0; darkSampleR = 0.0;
			for (int x = 0; x < 4; x++) {darkSampleL += avg4L[x]; darkSampleR += avg4R[x];}
			darkSampleL /= 4.0; darkSampleR /= 4.0;
		} if (spacing > 1) {
			avg2L[avgPos%2] = darkSampleL; avg2R[avgPos%2] = darkSampleR;
			darkSampleL = 0.0; darkSampleR = 0.0;
			for (int x = 0; x < 2; x++) {darkSampleL += avg2L[x]; darkSampleR += avg2R[x];}
			darkSampleL /= 2.0; darkSampleR /= 2.0; 
		} avgPos++;
		lastSlewL += fabs(lastSlewpleL-inputSampleL); lastSlewpleL = inputSampleL;
		double avgSlewL = fmin(lastSlewL*lastSlewL*(0.0635-(overallscale*0.0018436)),1.0);
		lastSlewL = fmax(lastSlewL*0.78,2.39996322972865332223);
		lastSlewR += fabs(lastSlewpleR-inputSampleR); lastSlewpleR = inputSampleR;
		double avgSlewR = fmin(lastSlewR*lastSlewR*(0.0635-(overallscale*0.0018436)),1.0);
		lastSlewR = fmax(lastSlewR*0.78,2.39996322972865332223); //look up Golden Angle, it's cool
		inputSampleL = (inputSampleL*(1.0-avgSlewL)) + (darkSampleL*avgSlewL);
		inputSampleR = (inputSampleR*(1.0-avgSlewR)) + (darkSampleR*avgSlewR);
		
		inputSampleL = fmin(fmax(inputSampleL,-2.032610446872596),2.032610446872596);
		long double X = inputSampleL * inputSampleL;
		long double sat = inputSampleL * X;
		inputSampleL -= (sat*0.125); sat *= X;
		inputSampleL += (sat*0.0078125); sat *= X;
		inputSampleL -= (sat*0.000244140625); sat *= X;
		inputSampleL += (sat*0.000003814697265625); sat *= X;
		inputSampleL -= (sat*0.0000000298023223876953125); sat *= X;
		//purestsaturation: sine, except all the corrections
		//retain mantissa of a long double increasing power function
		
		inputSampleR = fmin(fmax(inputSampleR,-2.032610446872596),2.032610446872596);
		X = inputSampleR * inputSampleR;
		sat = inputSampleR * X;
		inputSampleR -= (sat*0.125); sat *= X;
		inputSampleR += (sat*0.0078125); sat *= X;
		inputSampleR -= (sat*0.000244140625); sat *= X;
		inputSampleR += (sat*0.000003814697265625); sat *= X;
		inputSampleR -= (sat*0.0000000298023223876953125); sat *= X;
		//purestsaturation: sine, except all the corrections
		//retain mantissa of a long double increasing power function
		
		//we are leaving it as a clip that will go over 0dB.
		//it is a softclip so it will give you a more forgiving experience,
		//but you are meant to not drive the softclip for just level.

        //begin bar display section
        if ((fabs(inputSampleL-previousLeft)/28000.0f)*getSampleRate() > slewLeft) slewLeft =  (fabs(inputSampleL-previousLeft)/28000.0f)*getSampleRate();
        if ((fabs(inputSampleR-previousRight)/28000.0f)*getSampleRate() > slewRight) slewRight = (fabs(inputSampleR-previousRight)/28000.0f)*getSampleRate();
        previousLeft = inputSampleL; previousRight = inputSampleR; //slew measurement is NOT rectified
        double rectifiedL = fabs(inputSampleL);
        double rectifiedR = fabs(inputSampleR);
        if (rectifiedL > peakLeft) peakLeft = rectifiedL;
        if (rectifiedR > peakRight) peakRight = rectifiedR;
        rmsLeft += (rectifiedL * rectifiedL);
        rmsRight += (rectifiedR * rectifiedR);
        rmsCount++;
        zeroLeft += zeroCrossScale;
        if (longestZeroLeft < zeroLeft) longestZeroLeft = zeroLeft;
        if (wasPositiveL && inputSampleL < 0.0) {
            wasPositiveL = false;
            zeroLeft = 0.0;
        } else if (!wasPositiveL && inputSampleL > 0.0) {
            wasPositiveL = true;
            zeroLeft = 0.0;
        }
        zeroRight += zeroCrossScale;
        if (longestZeroRight < zeroRight) longestZeroRight = zeroRight;
        if (wasPositiveR && inputSampleR < 0.0) {
            wasPositiveR = false;
            zeroRight = 0.0;
        } else if (!wasPositiveR && inputSampleR > 0.0) {
            wasPositiveR = true;
            zeroRight = 0.0;
        } //end bar display section
        
        if (unclippedL-fabs(inputSampleL) > 0.0f) didClip += unclippedL-fabs(inputSampleL);
        if (unclippedR-fabs(inputSampleR) > 0.0f) didClip += unclippedR-fabs(inputSampleR);
        
        //begin 32 bit stereo floating point dither
        int expon; frexpf((float)inputSampleL, &expon);
        fpdL ^= fpdL << 13; fpdL ^= fpdL >> 17; fpdL ^= fpdL << 5;
        inputSampleL += ((double(fpdL)-uint32_t(0x7fffffff)) * 5.5e-36l * pow(2,expon+62));
        frexpf((float)inputSampleR, &expon);
        fpdR ^= fpdR << 13; fpdR ^= fpdR >> 17; fpdR ^= fpdR << 5;
        inputSampleR += ((double(fpdR)-uint32_t(0x7fffffff)) * 5.5e-36l * pow(2,expon+62));
        //end 32 bit stereo floating point dither
                
        *outL = inputSampleL;
        *outR = inputSampleR;
    }


    if (rmsCount > rmsSize)
    {
        AudioToUIMessage msg; //define the thing we're telling JUCE
        msg.what = AudioToUIMessage::SLEW_LEFT; msg.newValue = (float)slewLeft; audioToUI.push(msg);
        msg.what = AudioToUIMessage::SLEW_RIGHT; msg.newValue = (float)slewRight; audioToUI.push(msg);
        msg.what = AudioToUIMessage::PEAK_LEFT; msg.newValue = (float)sqrt(peakLeft); audioToUI.push(msg);
        msg.what = AudioToUIMessage::PEAK_RIGHT; msg.newValue = (float)sqrt(peakRight); audioToUI.push(msg);
        msg.what = AudioToUIMessage::RMS_LEFT; msg.newValue = (float)sqrt(sqrt(rmsLeft/rmsCount)); audioToUI.push(msg);
        msg.what = AudioToUIMessage::RMS_RIGHT; msg.newValue = (float)sqrt(sqrt(rmsRight/rmsCount)); audioToUI.push(msg);
        msg.what = AudioToUIMessage::ZERO_LEFT; msg.newValue = (float)longestZeroLeft; audioToUI.push(msg);
        msg.what = AudioToUIMessage::ZERO_RIGHT; msg.newValue = (float)longestZeroRight; audioToUI.push(msg);
        msg.what = AudioToUIMessage::BLINKEN_COMP; msg.newValue = (float)maxComp; audioToUI.push(msg);
        msg.what = AudioToUIMessage::BLINKEN_PAN; msg.newValue = (float)didClip; audioToUI.push(msg);
        msg.what = AudioToUIMessage::INCREMENT; msg.newValue = 1200.0f; audioToUI.push(msg);
        slewLeft = 0.0;
        slewRight = 0.0;
        peakLeft = 0.0;
        peakRight = 0.0;
        rmsLeft = 0.0;
        rmsRight = 0.0;
        zeroLeft = 0.0;
        zeroRight = 0.0;
        longestZeroLeft = 0.0;
        longestZeroRight = 0.0;
        didClip = 0.0f;
        maxComp = baseComp;
        baseComp = 0.0;
        rmsCount = 0;
    }
}

//==============================================================================

void PluginProcessor::processBlock (juce::AudioBuffer<double>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ignoreUnused (midiMessages);
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();
    // In case we have more outputs than inputs, this code clears any output
    // channels that didn't contain input data, (because these aren't
    // guaranteed to be empty - they may contain garbage).
    // This is here to avoid people getting screaming feedback
    // when they first compile a plugin, but obviously you don't need to keep
    // this code if your algorithm always overwrites all the output channels.
    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());
    if (!(getBus(false, 0)->isEnabled() && getBus(true, 0)->isEnabled())) return;
    auto mainOutput = getBusBuffer(buffer, false, 0); //if we have audio busses at all,
    auto mainInput = getBusBuffer(buffer, true, 0); //they're now mainOutput and mainInput.
    UIToAudioMessage uim;
    while (uiToAudio.pop(uim)) {
        switch (uim.what) {
        case UIToAudioMessage::NEW_VALUE: params[uim.which]->setValueNotifyingHost(params[uim.which]->convertTo0to1(uim.newValue)); break;
        case UIToAudioMessage::BEGIN_EDIT: params[uim.which]->beginChangeGesture(); break;
        case UIToAudioMessage::END_EDIT: params[uim.which]->endChangeGesture(); break;
        }
    } //Handle inbound messages from the UI thread
    
    double rmsSize = (1881.0 / 44100.0)*getSampleRate(); //higher is slower with larger RMS buffers
    double zeroCrossScale = (1.0 / getSampleRate())*44100.0;
    int inFramesToProcess = buffer.getNumSamples(); //vst doesn't give us this as a separate variable so we'll make it
    double overallscale = 1.0;
    overallscale /= 44100.0;
    overallscale *= getSampleRate();

	int spacing = floor(overallscale*2.0);
	if (spacing < 2) spacing = 2; if (spacing > 32) spacing = 32;
	
	double trebleGain = (params[KNOBHIG]->get()-0.5)*2.0;
	trebleGain = 1.0+(trebleGain*fabs(trebleGain)*fabs(trebleGain));
	double midGain = (params[KNOBMID]->get()-0.5)*2.0;
	midGain = 1.0+(midGain*fabs(midGain)*fabs(midGain));
	double bassGain = (params[KNOBLOW]->get()-0.5)*2.0;
	bassGain = 1.0+(bassGain*fabs(bassGain)*fabs(bassGain));
	//separate from filtering stage, this is amplitude, centered on 1.0 unity gain
	double highCoef = 0.0;
	double lowCoef = 0.0;
	double omega = 0.0;
	double biqK = 0.0;
	double norm = 0.0;
	
	bool eqOff = (trebleGain == 1.0 && midGain == 1.0 && bassGain == 1.0);
	//we get to completely bypass EQ if we're truly not using it. The mechanics of it mean that
	//it cancels out to bit-identical anyhow, but we get to skip the calculation
	if (!eqOff) {
		//SmoothEQ3 is how to get 3rd order steepness at very low CPU.
		//because sample rate varies, you could also vary the crossovers
		//you can't vary Q because math is simplified to take advantage of
		//how the accurate Q value for this filter is always exactly 1.0.
		highFast[biq_freq] = (4000.0/getSampleRate());
		omega = 2.0*M_PI*(4000.0/getSampleRate()); //mid-high crossover freq
		biqK = 2.0 - cos(omega);
		highCoef = -sqrt(biqK*biqK - 1.0) + biqK;
		lowFast[biq_freq] = (200.0/getSampleRate());
		omega = 2.0*M_PI*(200.0/getSampleRate()); //low-mid crossover freq
		biqK = 2.0 - cos(omega);
		lowCoef = -sqrt(biqK*biqK - 1.0) + biqK;
		//exponential IIR filter as part of an accurate 3rd order Butterworth filter 
		biqK = tan(M_PI * highFast[biq_freq]);
		norm = 1.0 / (1.0 + biqK + biqK*biqK);
		highFast[biq_a0] = biqK * biqK * norm;
		highFast[biq_a1] = 2.0 * highFast[biq_a0];
		highFast[biq_a2] = highFast[biq_a0];
		highFast[biq_b1] = 2.0 * (biqK*biqK - 1.0) * norm;
		highFast[biq_b2] = (1.0 - biqK + biqK*biqK) * norm;
		biqK = tan(M_PI * lowFast[biq_freq]);
		norm = 1.0 / (1.0 + biqK + biqK*biqK);
		lowFast[biq_a0] = biqK * biqK * norm;
		lowFast[biq_a1] = 2.0 * lowFast[biq_a0];
		lowFast[biq_a2] = lowFast[biq_a0];
		lowFast[biq_b1] = 2.0 * (biqK*biqK - 1.0) * norm;
		lowFast[biq_b2] = (1.0 - biqK + biqK*biqK) * norm;
		//custom biquad setup with Q = 1.0 gets to omit some divides
	}
	//SmoothEQ3
	
	double bezCThresh = pow(1.0-params[KNOBTHR]->get(), 6.0) * 8.0;
	double bezRez = pow(1.0-params[KNOBTHR]->get(), 12.360679774997898) / overallscale;
	double sloRez = pow(1.0-params[KNOBTHR]->get(),10.0) / overallscale;
	sloRez = fmin(fmax(sloRez,0.00001),1.0);
	bezRez = fmin(fmax(bezRez,0.00001),1.0);
	//Dynamics2
	
	highpass[hilp_freq] = ((pow(params[KNOBHIP]->get(),3)*24000.0)+10.0)/getSampleRate();
	if (highpass[hilp_freq] > 0.495) highpass[hilp_freq] = 0.495;
	bool highpassEngage = true; if (params[KNOBHIP]->get() == 0.0) highpassEngage = false;
	
	lowpass[hilp_freq] = ((pow(params[KNOBLOP]->get(),3)*24000.0)+10.0)/getSampleRate();
	if (lowpass[hilp_freq] > 0.495) lowpass[hilp_freq] = 0.495;
	bool lowpassEngage = true; if (params[KNOBLOP]->get() == 1.0) lowpassEngage = false;
	
	highpass[hilp_aA0] = highpass[hilp_aB0];
	highpass[hilp_aA1] = highpass[hilp_aB1];
	highpass[hilp_bA1] = highpass[hilp_bB1];
	highpass[hilp_bA2] = highpass[hilp_bB2];
	highpass[hilp_cA0] = highpass[hilp_cB0];
	highpass[hilp_cA1] = highpass[hilp_cB1];
	highpass[hilp_dA1] = highpass[hilp_dB1];
	highpass[hilp_dA2] = highpass[hilp_dB2];
	highpass[hilp_eA0] = highpass[hilp_eB0];
	highpass[hilp_eA1] = highpass[hilp_eB1];
	highpass[hilp_fA1] = highpass[hilp_fB1];
	highpass[hilp_fA2] = highpass[hilp_fB2];
	lowpass[hilp_aA0] = lowpass[hilp_aB0];
	lowpass[hilp_aA1] = lowpass[hilp_aB1];
	lowpass[hilp_bA1] = lowpass[hilp_bB1];
	lowpass[hilp_bA2] = lowpass[hilp_bB2];
	lowpass[hilp_cA0] = lowpass[hilp_cB0];
	lowpass[hilp_cA1] = lowpass[hilp_cB1];
	lowpass[hilp_dA1] = lowpass[hilp_dB1];
	lowpass[hilp_dA2] = lowpass[hilp_dB2];
	lowpass[hilp_eA0] = lowpass[hilp_eB0];
	lowpass[hilp_eA1] = lowpass[hilp_eB1];
	lowpass[hilp_fA1] = lowpass[hilp_fB1];
	lowpass[hilp_fA2] = lowpass[hilp_fB2];
	//previous run through the buffer is still in the filter, so we move it
	//to the A section and now it's the new starting point.
	//On the buss, highpass and lowpass are isolators meant to be moved,
	//so they are interpolated where the channels are not
	
	biqK = tan(M_PI * highpass[hilp_freq]); //highpass
	norm = 1.0 / (1.0 + biqK / 1.93185165 + biqK * biqK);
	highpass[hilp_aB0] = norm;
	highpass[hilp_aB1] = -2.0 * highpass[hilp_aB0];
	highpass[hilp_bB1] = 2.0 * (biqK * biqK - 1.0) * norm;
	highpass[hilp_bB2] = (1.0 - biqK / 1.93185165 + biqK * biqK) * norm;
	norm = 1.0 / (1.0 + biqK / 0.70710678 + biqK * biqK);
	highpass[hilp_cB0] = norm;
	highpass[hilp_cB1] = -2.0 * highpass[hilp_cB0];
	highpass[hilp_dB1] = 2.0 * (biqK * biqK - 1.0) * norm;
	highpass[hilp_dB2] = (1.0 - biqK / 0.70710678 + biqK * biqK) * norm;
	norm = 1.0 / (1.0 + biqK / 0.51763809 + biqK * biqK);
	highpass[hilp_eB0] = norm;
	highpass[hilp_eB1] = -2.0 * highpass[hilp_eB0];
	highpass[hilp_fB1] = 2.0 * (biqK * biqK - 1.0) * norm;
	highpass[hilp_fB2] = (1.0 - biqK / 0.51763809 + biqK * biqK) * norm;
	
	biqK = tan(M_PI * lowpass[hilp_freq]); //lowpass
	norm = 1.0 / (1.0 + biqK / 1.93185165 + biqK * biqK);
	lowpass[hilp_aB0] = biqK * biqK * norm;
	lowpass[hilp_aB1] = 2.0 * lowpass[hilp_aB0];
	lowpass[hilp_bB1] = 2.0 * (biqK * biqK - 1.0) * norm;
	lowpass[hilp_bB2] = (1.0 - biqK / 1.93185165 + biqK * biqK) * norm;
	norm = 1.0 / (1.0 + biqK / 0.70710678 + biqK * biqK);
	lowpass[hilp_cB0] = biqK * biqK * norm;
	lowpass[hilp_cB1] = 2.0 * lowpass[hilp_cB0];
	lowpass[hilp_dB1] = 2.0 * (biqK * biqK - 1.0) * norm;
	lowpass[hilp_dB2] = (1.0 - biqK / 0.70710678 + biqK * biqK) * norm;
	norm = 1.0 / (1.0 + biqK / 0.51763809 + biqK * biqK);
	lowpass[hilp_eB0] = biqK * biqK * norm;
	lowpass[hilp_eB1] = 2.0 * lowpass[hilp_eB0];
	lowpass[hilp_fB1] = 2.0 * (biqK * biqK - 1.0) * norm;
	lowpass[hilp_fB2] = (1.0 - biqK / 0.51763809 + biqK * biqK) * norm;
	
	if (highpass[hilp_aA0] == 0.0) { // if we have just started, start directly with raw info
		highpass[hilp_aA0] = highpass[hilp_aB0];
		highpass[hilp_aA1] = highpass[hilp_aB1];
		highpass[hilp_bA1] = highpass[hilp_bB1];
		highpass[hilp_bA2] = highpass[hilp_bB2];
		highpass[hilp_cA0] = highpass[hilp_cB0];
		highpass[hilp_cA1] = highpass[hilp_cB1];
		highpass[hilp_dA1] = highpass[hilp_dB1];
		highpass[hilp_dA2] = highpass[hilp_dB2];
		highpass[hilp_eA0] = highpass[hilp_eB0];
		highpass[hilp_eA1] = highpass[hilp_eB1];
		highpass[hilp_fA1] = highpass[hilp_fB1];
		highpass[hilp_fA2] = highpass[hilp_fB2];
		lowpass[hilp_aA0] = lowpass[hilp_aB0];
		lowpass[hilp_aA1] = lowpass[hilp_aB1];
		lowpass[hilp_bA1] = lowpass[hilp_bB1];
		lowpass[hilp_bA2] = lowpass[hilp_bB2];
		lowpass[hilp_cA0] = lowpass[hilp_cB0];
		lowpass[hilp_cA1] = lowpass[hilp_cB1];
		lowpass[hilp_dA1] = lowpass[hilp_dB1];
		lowpass[hilp_dA2] = lowpass[hilp_dB2];
		lowpass[hilp_eA0] = lowpass[hilp_eB0];
		lowpass[hilp_eA1] = lowpass[hilp_eB1];
		lowpass[hilp_fA1] = lowpass[hilp_fB1];
		lowpass[hilp_fA2] = lowpass[hilp_fB2];
	}
	
	panA = panB; panB = params[KNOBPAN]->get()*1.57079633;
	inTrimA = inTrimB; inTrimB = params[KNOBFAD]->get()*2.0;
	//Console

    for (int i = 0; i < buffer.getNumSamples(); ++i)
    {
        auto outL = mainOutput.getWritePointer(0, i);
        auto outR = mainOutput.getWritePointer(1, i);
        auto inL = mainInput.getReadPointer(0, i); //in isBussesLayoutSupported, we have already
        auto inR = mainInput.getReadPointer(1, i); //specified that we can only be stereo and never mono
        long double inputSampleL = *inL;
        long double inputSampleR = *inR;
        if (fabs(inputSampleL)<1.18e-23) inputSampleL = fpdL * 1.18e-17;
        if (fabs(inputSampleR)<1.18e-23) inputSampleR = fpdR * 1.18e-17;
        //NOTE: I don't yet know whether counting the buffer backwards means that gainA and gainB must be reversed.
        //If the audio flow is in fact reversed we must swap the temp and (1.0-temp) and I have done this provisionally
        //Original VST2 is counting DOWN with -- operator, but this counts UP with ++
        //If this doesn't create zipper noise on sine processing then it's correct
		
		const double temp = (double)i/inFramesToProcess;
		highpass[hilp_a0] = (highpass[hilp_aA0]*temp)+(highpass[hilp_aB0]*(1.0-temp));
		highpass[hilp_a1] = (highpass[hilp_aA1]*temp)+(highpass[hilp_aB1]*(1.0-temp));
		highpass[hilp_b1] = (highpass[hilp_bA1]*temp)+(highpass[hilp_bB1]*(1.0-temp));
		highpass[hilp_b2] = (highpass[hilp_bA2]*temp)+(highpass[hilp_bB2]*(1.0-temp));
		highpass[hilp_c0] = (highpass[hilp_cA0]*temp)+(highpass[hilp_cB0]*(1.0-temp));
		highpass[hilp_c1] = (highpass[hilp_cA1]*temp)+(highpass[hilp_cB1]*(1.0-temp));
		highpass[hilp_d1] = (highpass[hilp_dA1]*temp)+(highpass[hilp_dB1]*(1.0-temp));
		highpass[hilp_d2] = (highpass[hilp_dA2]*temp)+(highpass[hilp_dB2]*(1.0-temp));
		highpass[hilp_e0] = (highpass[hilp_eA0]*temp)+(highpass[hilp_eB0]*(1.0-temp));
		highpass[hilp_e1] = (highpass[hilp_eA1]*temp)+(highpass[hilp_eB1]*(1.0-temp));
		highpass[hilp_f1] = (highpass[hilp_fA1]*temp)+(highpass[hilp_fB1]*(1.0-temp));
		highpass[hilp_f2] = (highpass[hilp_fA2]*temp)+(highpass[hilp_fB2]*(1.0-temp));
		lowpass[hilp_a0] = (lowpass[hilp_aA0]*temp)+(lowpass[hilp_aB0]*(1.0-temp));
		lowpass[hilp_a1] = (lowpass[hilp_aA1]*temp)+(lowpass[hilp_aB1]*(1.0-temp));
		lowpass[hilp_b1] = (lowpass[hilp_bA1]*temp)+(lowpass[hilp_bB1]*(1.0-temp));
		lowpass[hilp_b2] = (lowpass[hilp_bA2]*temp)+(lowpass[hilp_bB2]*(1.0-temp));
		lowpass[hilp_c0] = (lowpass[hilp_cA0]*temp)+(lowpass[hilp_cB0]*(1.0-temp));
		lowpass[hilp_c1] = (lowpass[hilp_cA1]*temp)+(lowpass[hilp_cB1]*(1.0-temp));
		lowpass[hilp_d1] = (lowpass[hilp_dA1]*temp)+(lowpass[hilp_dB1]*(1.0-temp));
		lowpass[hilp_d2] = (lowpass[hilp_dA2]*temp)+(lowpass[hilp_dB2]*(1.0-temp));
		lowpass[hilp_e0] = (lowpass[hilp_eA0]*temp)+(lowpass[hilp_eB0]*(1.0-temp));
		lowpass[hilp_e1] = (lowpass[hilp_eA1]*temp)+(lowpass[hilp_eB1]*(1.0-temp));
		lowpass[hilp_f1] = (lowpass[hilp_fA1]*temp)+(lowpass[hilp_fB1]*(1.0-temp));
		lowpass[hilp_f2] = (lowpass[hilp_fA2]*temp)+(lowpass[hilp_fB2]*(1.0-temp));
		
		
		if (highpassEngage) { //distributed Highpass
			highpass[hilp_temp] = (inputSampleL*highpass[hilp_a0])+highpass[hilp_aL1];
			highpass[hilp_aL1] = (inputSampleL*highpass[hilp_a1])-(highpass[hilp_temp]*highpass[hilp_b1])+highpass[hilp_aL2];
			highpass[hilp_aL2] = (inputSampleL*highpass[hilp_a0])-(highpass[hilp_temp]*highpass[hilp_b2]); inputSampleL = highpass[hilp_temp];
			highpass[hilp_temp] = (inputSampleR*highpass[hilp_a0])+highpass[hilp_aR1];
			highpass[hilp_aR1] = (inputSampleR*highpass[hilp_a1])-(highpass[hilp_temp]*highpass[hilp_b1])+highpass[hilp_aR2];
			highpass[hilp_aR2] = (inputSampleR*highpass[hilp_a0])-(highpass[hilp_temp]*highpass[hilp_b2]); inputSampleR = highpass[hilp_temp];
		} else highpass[hilp_aR1] = highpass[hilp_aR2] = highpass[hilp_aL1] = highpass[hilp_aL2] = 0.0;
		if (lowpassEngage) { //distributed Lowpass
			lowpass[hilp_temp] = (inputSampleL*lowpass[hilp_a0])+lowpass[hilp_aL1];
			lowpass[hilp_aL1] = (inputSampleL*lowpass[hilp_a1])-(lowpass[hilp_temp]*lowpass[hilp_b1])+lowpass[hilp_aL2];
			lowpass[hilp_aL2] = (inputSampleL*lowpass[hilp_a0])-(lowpass[hilp_temp]*lowpass[hilp_b2]); inputSampleL = lowpass[hilp_temp];
			lowpass[hilp_temp] = (inputSampleR*lowpass[hilp_a0])+lowpass[hilp_aR1];
			lowpass[hilp_aR1] = (inputSampleR*lowpass[hilp_a1])-(lowpass[hilp_temp]*lowpass[hilp_b1])+lowpass[hilp_aR2];
			lowpass[hilp_aR2] = (inputSampleR*lowpass[hilp_a0])-(lowpass[hilp_temp]*lowpass[hilp_b2]); inputSampleR = lowpass[hilp_temp];
		} else lowpass[hilp_aR1] = lowpass[hilp_aR2] = lowpass[hilp_aL1] = lowpass[hilp_aL2] = 0.0;
		//first Highpass/Lowpass blocks aliasing before the nonlinearity of ConsoleXBuss and Parametric		
		
		if (inputSampleL > 1.0) inputSampleL = 1.0;
		else if (inputSampleL > 0.0) inputSampleL = -expm1((log1p(-inputSampleL) * 0.6180339887498949));
		if (inputSampleL < -1.0) inputSampleL = -1.0;
		else if (inputSampleL < 0.0) inputSampleL = expm1((log1p(inputSampleL) * 0.6180339887498949));
		
		if (inputSampleR > 1.0) inputSampleR = 1.0;
		else if (inputSampleR > 0.0) inputSampleR = -expm1((log1p(-inputSampleR) * 0.6180339887498949));
		if (inputSampleR < -1.0) inputSampleR = -1.0;
		else if (inputSampleR < 0.0) inputSampleR = expm1((log1p(inputSampleR) * 0.6180339887498949));
		
		if (highpassEngage) { //distributed Highpass
			highpass[hilp_temp] = (inputSampleL*highpass[hilp_c0])+highpass[hilp_cL1];
			highpass[hilp_cL1] = (inputSampleL*highpass[hilp_c1])-(highpass[hilp_temp]*highpass[hilp_d1])+highpass[hilp_cL2];
			highpass[hilp_cL2] = (inputSampleL*highpass[hilp_c0])-(highpass[hilp_temp]*highpass[hilp_d2]); inputSampleL = highpass[hilp_temp];
			highpass[hilp_temp] = (inputSampleR*highpass[hilp_c0])+highpass[hilp_cR1];
			highpass[hilp_cR1] = (inputSampleR*highpass[hilp_c1])-(highpass[hilp_temp]*highpass[hilp_d1])+highpass[hilp_cR2];
			highpass[hilp_cR2] = (inputSampleR*highpass[hilp_c0])-(highpass[hilp_temp]*highpass[hilp_d2]); inputSampleR = highpass[hilp_temp];
		} else highpass[hilp_cR1] = highpass[hilp_cR2] = highpass[hilp_cL1] = highpass[hilp_cL2] = 0.0;
		if (lowpassEngage) { //distributed Lowpass
			lowpass[hilp_temp] = (inputSampleL*lowpass[hilp_c0])+lowpass[hilp_cL1];
			lowpass[hilp_cL1] = (inputSampleL*lowpass[hilp_c1])-(lowpass[hilp_temp]*lowpass[hilp_d1])+lowpass[hilp_cL2];
			lowpass[hilp_cL2] = (inputSampleL*lowpass[hilp_c0])-(lowpass[hilp_temp]*lowpass[hilp_d2]); inputSampleL = lowpass[hilp_temp];
			lowpass[hilp_temp] = (inputSampleR*lowpass[hilp_c0])+lowpass[hilp_cR1];
			lowpass[hilp_cR1] = (inputSampleR*lowpass[hilp_c1])-(lowpass[hilp_temp]*lowpass[hilp_d1])+lowpass[hilp_cR2];
			lowpass[hilp_cR2] = (inputSampleR*lowpass[hilp_c0])-(lowpass[hilp_temp]*lowpass[hilp_d2]); inputSampleR = lowpass[hilp_temp];
		} else lowpass[hilp_cR1] = lowpass[hilp_cR2] = lowpass[hilp_cL1] = lowpass[hilp_cL2] = 0.0;
		//another stage of Highpass/Lowpass before bringing in the parametric bands		
		
		if (!eqOff) {
			double trebleFastL = inputSampleL;		
			double outSample = (trebleFastL * highFast[biq_a0]) + highFast[biq_sL1];
			highFast[biq_sL1] = (trebleFastL * highFast[biq_a1]) - (outSample * highFast[biq_b1]) + highFast[biq_sL2];
			highFast[biq_sL2] = (trebleFastL * highFast[biq_a2]) - (outSample * highFast[biq_b2]);
			double midFastL = outSample; trebleFastL -= midFastL;
			outSample = (midFastL * lowFast[biq_a0]) + lowFast[biq_sL1];
			lowFast[biq_sL1] = (midFastL * lowFast[biq_a1]) - (outSample * lowFast[biq_b1]) + lowFast[biq_sL2];
			lowFast[biq_sL2] = (midFastL * lowFast[biq_a2]) - (outSample * lowFast[biq_b2]);
			double bassFastL = outSample; midFastL -= bassFastL;
			trebleFastL = (bassFastL*bassGain) + (midFastL*midGain) + (trebleFastL*trebleGain);
			//first stage of two crossovers is biquad of exactly 1.0 Q
			highFastLIIR = (highFastLIIR*highCoef) + (trebleFastL*(1.0-highCoef));
			midFastL = highFastLIIR; trebleFastL -= midFastL;
			lowFastLIIR = (lowFastLIIR*lowCoef) + (midFastL*(1.0-lowCoef));
			bassFastL = lowFastLIIR; midFastL -= bassFastL;
			inputSampleL = (bassFastL*bassGain) + (midFastL*midGain) + (trebleFastL*trebleGain);		
			//second stage of two crossovers is the exponential filters
			//this produces a slightly steeper Butterworth filter very cheaply
			double trebleFastR = inputSampleR;		
			outSample = (trebleFastR * highFast[biq_a0]) + highFast[biq_sR1];
			highFast[biq_sR1] = (trebleFastR * highFast[biq_a1]) - (outSample * highFast[biq_b1]) + highFast[biq_sR2];
			highFast[biq_sR2] = (trebleFastR * highFast[biq_a2]) - (outSample * highFast[biq_b2]);
			double midFastR = outSample; trebleFastR -= midFastR;
			outSample = (midFastR * lowFast[biq_a0]) + lowFast[biq_sR1];
			lowFast[biq_sR1] = (midFastR * lowFast[biq_a1]) - (outSample * lowFast[biq_b1]) + lowFast[biq_sR2];
			lowFast[biq_sR2] = (midFastR * lowFast[biq_a2]) - (outSample * lowFast[biq_b2]);
			double bassFastR = outSample; midFastR -= bassFastR;
			trebleFastR = (bassFastR*bassGain) + (midFastR*midGain) + (trebleFastR*trebleGain);
			//first stage of two crossovers is biquad of exactly 1.0 Q
			highFastRIIR = (highFastRIIR*highCoef) + (trebleFastR*(1.0-highCoef));
			midFastR = highFastRIIR; trebleFastR -= midFastR;
			lowFastRIIR = (lowFastRIIR*lowCoef) + (midFastR*(1.0-lowCoef));
			bassFastR = lowFastRIIR; midFastR -= bassFastR;
			inputSampleR = (bassFastR*bassGain) + (midFastR*midGain) + (trebleFastR*trebleGain);		
			//second stage of two crossovers is the exponential filters
			//this produces a slightly steeper Butterworth filter very cheaply
		}
		//SmoothEQ3
		
		if (baseComp < fabs(inputSampleR*64.0)) baseComp = fmin(fabs(inputSampleR*64.0),1.0);
        if (baseComp < fabs(inputSampleL*64.0)) baseComp = fmin(fabs(inputSampleL*64.0),1.0);
		
		if (bezCThresh > 0.0) {
			inputSampleL *= ((bezCThresh*0.5)+1.0);
			inputSampleR *= ((bezCThresh*0.5)+1.0);
			bezCompF[bez_cycle] += bezRez;
			bezCompF[bez_Ctrl] += (fmax(fabs(inputSampleL),fabs(inputSampleR)) * bezRez);
			if (bezCompF[bez_cycle] > 1.0) {
				bezCompF[bez_cycle] -= 1.0;
				bezCompF[bez_C] = bezCompF[bez_B];
				bezCompF[bez_B] = bezCompF[bez_A];
				bezCompF[bez_A] = bezCompF[bez_Ctrl];
				bezCompF[bez_Ctrl] = 0.0;
			}
			bezCompS[bez_cycle] += sloRez;
			bezCompS[bez_Ctrl] += (fmax(fabs(inputSampleL),fabs(inputSampleR)) * sloRez);
			if (bezCompS[bez_cycle] > 1.0) {
				bezCompS[bez_cycle] -= 1.0;
				bezCompS[bez_C] = bezCompS[bez_B];
				bezCompS[bez_B] = bezCompS[bez_A];
				bezCompS[bez_A] = bezCompS[bez_Ctrl];
				bezCompS[bez_Ctrl] = 0.0;
			}
			double CBF = (bezCompF[bez_C]*(1.0-bezCompF[bez_cycle]))+(bezCompF[bez_B]*bezCompF[bez_cycle]);
			double BAF = (bezCompF[bez_B]*(1.0-bezCompF[bez_cycle]))+(bezCompF[bez_A]*bezCompF[bez_cycle]);
			double CBAF = (bezCompF[bez_B]+(CBF*(1.0-bezCompF[bez_cycle]))+(BAF*bezCompF[bez_cycle]))*0.5;
			double CBS = (bezCompS[bez_C]*(1.0-bezCompS[bez_cycle]))+(bezCompS[bez_B]*bezCompS[bez_cycle]);
			double BAS = (bezCompS[bez_B]*(1.0-bezCompS[bez_cycle]))+(bezCompS[bez_A]*bezCompS[bez_cycle]);
			double CBAS = (bezCompS[bez_B]+(CBS*(1.0-bezCompS[bez_cycle]))+(BAS*bezCompS[bez_cycle]))*0.5;
			double CBAMax = fmax(CBAS,CBAF); if (CBAMax > 0.0) CBAMax = 1.0/CBAMax;
			double CBAFade = ((CBAS*-CBAMax)+(CBAF*CBAMax)+1.0)*0.5;
			inputSampleL *= 1.0-(fmin(((CBAS*(1.0-CBAFade))+(CBAF*CBAFade))*bezCThresh,1.0));
			inputSampleR *= 1.0-(fmin(((CBAS*(1.0-CBAFade))+(CBAF*CBAFade))*bezCThresh,1.0));
            if (maxComp > pow(1.0-fmin(CBAS,CBAF),4.0)) maxComp = pow(1.0-fmin(CBAS,CBAF),4.0);
		} else {bezCompF[bez_Ctrl] = 0.0; bezCompS[bez_Ctrl] = 0.0;}
		//Dynamics2 custom version for buss
		

		if (highpassEngage) { //distributed Highpass
			highpass[hilp_temp] = (inputSampleL*highpass[hilp_e0])+highpass[hilp_eL1];
			highpass[hilp_eL1] = (inputSampleL*highpass[hilp_e1])-(highpass[hilp_temp]*highpass[hilp_f1])+highpass[hilp_eL2];
			highpass[hilp_eL2] = (inputSampleL*highpass[hilp_e0])-(highpass[hilp_temp]*highpass[hilp_f2]); inputSampleL = highpass[hilp_temp];
			highpass[hilp_temp] = (inputSampleR*highpass[hilp_e0])+highpass[hilp_eR1];
			highpass[hilp_eR1] = (inputSampleR*highpass[hilp_e1])-(highpass[hilp_temp]*highpass[hilp_f1])+highpass[hilp_eR2];
			highpass[hilp_eR2] = (inputSampleR*highpass[hilp_e0])-(highpass[hilp_temp]*highpass[hilp_f2]); inputSampleR = highpass[hilp_temp];
		} else highpass[hilp_eR1] = highpass[hilp_eR2] = highpass[hilp_eL1] = highpass[hilp_eL2] = 0.0;
		if (lowpassEngage) { //distributed Lowpass
			lowpass[hilp_temp] = (inputSampleL*lowpass[hilp_e0])+lowpass[hilp_eL1];
			lowpass[hilp_eL1] = (inputSampleL*lowpass[hilp_e1])-(lowpass[hilp_temp]*lowpass[hilp_f1])+lowpass[hilp_eL2];
			lowpass[hilp_eL2] = (inputSampleL*lowpass[hilp_e0])-(lowpass[hilp_temp]*lowpass[hilp_f2]); inputSampleL = lowpass[hilp_temp];
			lowpass[hilp_temp] = (inputSampleR*lowpass[hilp_e0])+lowpass[hilp_eR1];
			lowpass[hilp_eR1] = (inputSampleR*lowpass[hilp_e1])-(lowpass[hilp_temp]*lowpass[hilp_f1])+lowpass[hilp_eR2];
			lowpass[hilp_eR2] = (inputSampleR*lowpass[hilp_e0])-(lowpass[hilp_temp]*lowpass[hilp_f2]); inputSampleR = lowpass[hilp_temp];
		} else lowpass[hilp_eR1] = lowpass[hilp_eR2] = lowpass[hilp_eL1] = lowpass[hilp_eL2] = 0.0;		
		//final Highpass/Lowpass continues to address aliasing
		//final stacked biquad section is the softest Q for smoothness

		double gainR = (panA*temp)+(panB*(1.0-temp));
		double gainL = 1.57079633-gainR;
		gainR = sin(gainR); gainL = sin(gainL);
		double gain = (inTrimA*temp)+(inTrimB*(1.0-temp));
		if (gain > 1.0) gain *= gain;
		if (gain < 1.0) gain = 1.0-pow(1.0-gain,2);
		gain *= 2.0;
		
		inputSampleL = inputSampleL * gainL * gain;
		inputSampleR = inputSampleR * gainR * gain;
		//applies pan section, and smoothed fader gain
        
        double unclippedL = inputSampleL*inputSampleL;
        double unclippedR = inputSampleR*inputSampleR;
		
		double darkSampleL = inputSampleL;
		double darkSampleR = inputSampleR;
		if (avgPos > 31) avgPos = 0;
		if (spacing > 31) {
			avg32L[avgPos] = darkSampleL; avg32R[avgPos] = darkSampleR;
			darkSampleL = 0.0; darkSampleR = 0.0;
			for (int x = 0; x < 32; x++) {darkSampleL += avg32L[x]; darkSampleR += avg32R[x];}
			darkSampleL /= 32.0; darkSampleR /= 32.0;
		} if (spacing > 15) {
			avg16L[avgPos%16] = darkSampleL; avg16R[avgPos%16] = darkSampleR;
			darkSampleL = 0.0; darkSampleR = 0.0;
			for (int x = 0; x < 16; x++) {darkSampleL += avg16L[x]; darkSampleR += avg16R[x];}
			darkSampleL /= 16.0; darkSampleR /= 16.0;
		} if (spacing > 7) {
			avg8L[avgPos%8] = darkSampleL; avg8R[avgPos%8] = darkSampleR;
			darkSampleL = 0.0; darkSampleR = 0.0;
			for (int x = 0; x < 8; x++) {darkSampleL += avg8L[x]; darkSampleR += avg8R[x];}
			darkSampleL /= 8.0; darkSampleR /= 8.0;
		} if (spacing > 3) {
			avg4L[avgPos%4] = darkSampleL; avg4R[avgPos%4] = darkSampleR;
			darkSampleL = 0.0; darkSampleR = 0.0;
			for (int x = 0; x < 4; x++) {darkSampleL += avg4L[x]; darkSampleR += avg4R[x];}
			darkSampleL /= 4.0; darkSampleR /= 4.0;
		} if (spacing > 1) {
			avg2L[avgPos%2] = darkSampleL; avg2R[avgPos%2] = darkSampleR;
			darkSampleL = 0.0; darkSampleR = 0.0;
			for (int x = 0; x < 2; x++) {darkSampleL += avg2L[x]; darkSampleR += avg2R[x];}
			darkSampleL /= 2.0; darkSampleR /= 2.0; 
		} avgPos++;
		lastSlewL += fabs(lastSlewpleL-inputSampleL); lastSlewpleL = inputSampleL;
		double avgSlewL = fmin(lastSlewL*lastSlewL*(0.0635-(overallscale*0.0018436)),1.0);
		lastSlewL = fmax(lastSlewL*0.78,2.39996322972865332223);
		lastSlewR += fabs(lastSlewpleR-inputSampleR); lastSlewpleR = inputSampleR;
		double avgSlewR = fmin(lastSlewR*lastSlewR*(0.0635-(overallscale*0.0018436)),1.0);
		lastSlewR = fmax(lastSlewR*0.78,2.39996322972865332223); //look up Golden Angle, it's cool
		inputSampleL = (inputSampleL*(1.0-avgSlewL)) + (darkSampleL*avgSlewL);
		inputSampleR = (inputSampleR*(1.0-avgSlewR)) + (darkSampleR*avgSlewR);
		
		inputSampleL = fmin(fmax(inputSampleL,-2.032610446872596),2.032610446872596);
		long double X = inputSampleL * inputSampleL;
		long double sat = inputSampleL * X;
		inputSampleL -= (sat*0.125); sat *= X;
		inputSampleL += (sat*0.0078125); sat *= X;
		inputSampleL -= (sat*0.000244140625); sat *= X;
		inputSampleL += (sat*0.000003814697265625); sat *= X;
		inputSampleL -= (sat*0.0000000298023223876953125); sat *= X;
		//purestsaturation: sine, except all the corrections
		//retain mantissa of a long double increasing power function
		
		inputSampleR = fmin(fmax(inputSampleR,-2.032610446872596),2.032610446872596);
		X = inputSampleR * inputSampleR;
		sat = inputSampleR * X;
		inputSampleR -= (sat*0.125); sat *= X;
		inputSampleR += (sat*0.0078125); sat *= X;
		inputSampleR -= (sat*0.000244140625); sat *= X;
		inputSampleR += (sat*0.000003814697265625); sat *= X;
		inputSampleR -= (sat*0.0000000298023223876953125); sat *= X;
		//purestsaturation: sine, except all the corrections
		//retain mantissa of a long double increasing power function
		
		//we are leaving it as a clip that will go over 0dB.
		//it is a softclip so it will give you a more forgiving experience,
		//but you are meant to not drive the softclip for just level.
		
        //begin bar display section
        if ((fabs(inputSampleL-previousLeft)/28000.0f)*getSampleRate() > slewLeft) slewLeft =  (fabs(inputSampleL-previousLeft)/28000.0f)*getSampleRate();
        if ((fabs(inputSampleR-previousRight)/28000.0f)*getSampleRate() > slewRight) slewRight = (fabs(inputSampleR-previousRight)/28000.0f)*getSampleRate();
        previousLeft = inputSampleL; previousRight = inputSampleR; //slew measurement is NOT rectified
        double rectifiedL = fabs(inputSampleL);
        double rectifiedR = fabs(inputSampleR);
        if (rectifiedL > peakLeft) peakLeft = rectifiedL;
        if (rectifiedR > peakRight) peakRight = rectifiedR;
        rmsLeft += (rectifiedL * rectifiedL);
        rmsRight += (rectifiedR * rectifiedR);
        rmsCount++;
        zeroLeft += zeroCrossScale;
        if (longestZeroLeft < zeroLeft) longestZeroLeft = zeroLeft;
        if (wasPositiveL && inputSampleL < 0.0) {
            wasPositiveL = false;
            zeroLeft = 0.0;
        } else if (!wasPositiveL && inputSampleL > 0.0) {
            wasPositiveL = true;
            zeroLeft = 0.0;
        }
        zeroRight += zeroCrossScale;
        if (longestZeroRight < zeroRight) longestZeroRight = zeroRight;
        if (wasPositiveR && inputSampleR < 0.0) {
            wasPositiveR = false;
            zeroRight = 0.0;
        } else if (!wasPositiveR && inputSampleR > 0.0) {
            wasPositiveR = true;
            zeroRight = 0.0;
        } //end bar display section
        
        if (unclippedL-fabs(inputSampleL) > 0.0f) didClip += unclippedL-fabs(inputSampleL);
        if (unclippedR-fabs(inputSampleR) > 0.0f) didClip += unclippedR-fabs(inputSampleR);

        //begin 64 bit stereo floating point dither
        int expon; frexp((double)inputSampleL, &expon);
        fpdL ^= fpdL << 13; fpdL ^= fpdL >> 17; fpdL ^= fpdL << 5;
        inputSampleL += ((double(fpdL)-uint32_t(0x7fffffff)) * 1.1e-44l * pow(2,expon+62));
        frexp((double)inputSampleR, &expon);
        fpdR ^= fpdR << 13; fpdR ^= fpdR >> 17; fpdR ^= fpdR << 5;
        inputSampleR += ((double(fpdR)-uint32_t(0x7fffffff)) * 1.1e-44l * pow(2,expon+62));
        //end 64 bit stereo floating point dither
        
        *outL = inputSampleL;
        *outR = inputSampleR;
    }

    if (rmsCount > rmsSize)
    {
        AudioToUIMessage msg; //define the thing we're telling JUCE
        msg.what = AudioToUIMessage::SLEW_LEFT; msg.newValue = (float)slewLeft; audioToUI.push(msg);
        msg.what = AudioToUIMessage::SLEW_RIGHT; msg.newValue = (float)slewRight; audioToUI.push(msg);
        msg.what = AudioToUIMessage::PEAK_LEFT; msg.newValue = (float)sqrt(peakLeft); audioToUI.push(msg);
        msg.what = AudioToUIMessage::PEAK_RIGHT; msg.newValue = (float)sqrt(peakRight); audioToUI.push(msg);
        msg.what = AudioToUIMessage::RMS_LEFT; msg.newValue = (float)sqrt(sqrt(rmsLeft/rmsCount)); audioToUI.push(msg);
        msg.what = AudioToUIMessage::RMS_RIGHT; msg.newValue = (float)sqrt(sqrt(rmsRight/rmsCount)); audioToUI.push(msg);
        msg.what = AudioToUIMessage::ZERO_LEFT; msg.newValue = (float)longestZeroLeft; audioToUI.push(msg);
        msg.what = AudioToUIMessage::ZERO_RIGHT; msg.newValue = (float)longestZeroRight; audioToUI.push(msg);
        msg.what = AudioToUIMessage::BLINKEN_COMP; msg.newValue = (float)maxComp; audioToUI.push(msg);
        msg.what = AudioToUIMessage::BLINKEN_PAN; msg.newValue = (float)didClip; audioToUI.push(msg);
        msg.what = AudioToUIMessage::INCREMENT; msg.newValue = 1200.0f; audioToUI.push(msg);
        slewLeft = 0.0;
        slewRight = 0.0;
        peakLeft = 0.0;
        peakRight = 0.0;
        rmsLeft = 0.0;
        rmsRight = 0.0;
        zeroLeft = 0.0;
        zeroRight = 0.0;
        longestZeroLeft = 0.0;
        longestZeroRight = 0.0;
        didClip = 0.0f;
        maxComp = baseComp;
        baseComp = 0.0;
        rmsCount = 0;
    }
}

