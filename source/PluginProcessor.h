#pragma once

#include <juce_audio_processors/juce_audio_processors.h>

#if (MSVC)
#include "ipps.h"
#endif

class PluginProcessor : public juce::AudioProcessor, public juce::AudioProcessorParameter::Listener
{
public:
    PluginProcessor();
    ~PluginProcessor() override;
    void parameterValueChanged(int parameterIndex, float newValue) override;
    void parameterGestureChanged(int parameterIndex, bool starting) override;
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;
    void processBlock (juce::AudioBuffer<double>&, juce::MidiBuffer&) override;
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;
    bool supportsDoublePrecisionProcessing() const override;
    const juce::String getName() const override;
    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;
    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;
    // updateTrackProperties may be called by the host if it feels like it
    // this method calls a similar one in the editor class that updates the editor
    void updateTrackProperties(const TrackProperties& properties) override;
    
    TrackProperties trackProperties;

    //now we can declare variables used in the audio thread
    
    enum Parameters
    {
        KNOBHIG,
        KNOBMID,
        KNOBLOW,
        KNOBTHR,
        KNOBHIP,
        KNOBLOP,
        KNOBPAN,
        KNOBFAD,
    };
    static constexpr int n_params = KNOBFAD + 1;
    std::array<juce::AudioParameterFloat *, n_params> params;
    //This is where we're defining things that go into the plugin's interface.
    
    struct UIToAudioMessage
    {
        enum What
        {
            NEW_VALUE,
            BEGIN_EDIT,
            END_EDIT
        } what{NEW_VALUE};
        Parameters which;
        float newValue = 0.0;
    };
    //This is things the interface can tell the audio thread about.
    
    struct AudioToUIMessage
    {
        enum What
        {
            NEW_VALUE,
            RMS_LEFT,
            RMS_RIGHT,
            PEAK_LEFT,
            PEAK_RIGHT,
            SLEW_LEFT,
            SLEW_RIGHT,
            ZERO_LEFT,
            ZERO_RIGHT,
            BLINKEN_COMP,
            BLINKEN_PAN,
            INCREMENT
        } what{NEW_VALUE};
        Parameters which;
        float newValue = 0.0;
    };
    //This is kinds of information the audio thread can give the interface.
    
    template <typename T, int qSize = 4096> class LockFreeQueue
    {
      public:
        LockFreeQueue() : af(qSize) {}
        bool push(const T &ad)
        {
            auto ret = false;
            int start1, size1, start2, size2;
            af.prepareToWrite(1, start1, size1, start2, size2);
            if (size1 > 0)
            {
                dq[start1] = ad;
                ret = true;
            }
            af.finishedWrite(size1 + size2);
            return ret;
        }
        bool pop(T &ad)
        {
            bool ret = false;
            int start1, size1, start2, size2;
            af.prepareToRead(1, start1, size1, start2, size2);
            if (size1 > 0)
            {
                ad = dq[start1];
                ret = true;
            }
            af.finishedRead(size1 + size2);
            return ret;
        }
        juce::AbstractFifo af;
        T dq[qSize];
    };
    
    LockFreeQueue<UIToAudioMessage> uiToAudio;
    LockFreeQueue<AudioToUIMessage> audioToUI;
    
    
	enum {
		biq_freq,
		biq_reso,
		biq_a0,
		biq_a1,
		biq_a2,
		biq_b1,
		biq_b2,
		biq_sL1,
		biq_sL2,
		biq_sR1,
		biq_sR2,
		biq_total
	}; //coefficient interpolating filter, stereo
	double highFast[biq_total];
	double lowFast[biq_total];
	double highFastLIIR;
	double highFastRIIR;
	double lowFastLIIR;
	double lowFastRIIR;
	//SmoothEQ3
		
	enum {
		bez_A,
		bez_B,
		bez_C,
		bez_Ctrl,
		bez_cycle,
		bez_total
	}; //the new undersampling. bez signifies the bezier curve reconstruction
	double bezCompF[bez_total];
	double bezCompS[bez_total];
	//Dynamics2 custom for buss
	
	enum {
		hilp_freq, hilp_temp,
		hilp_a0, hilp_aA0, hilp_aB0, hilp_a1, hilp_aA1, hilp_aB1, hilp_b1, hilp_bA1, hilp_bB1, hilp_b2, hilp_bA2, hilp_bB2,
		hilp_c0, hilp_cA0, hilp_cB0, hilp_c1, hilp_cA1, hilp_cB1, hilp_d1, hilp_dA1, hilp_dB1, hilp_d2, hilp_dA2, hilp_dB2,
		hilp_e0, hilp_eA0, hilp_eB0, hilp_e1, hilp_eA1, hilp_eB1, hilp_f1, hilp_fA1, hilp_fB1, hilp_f2, hilp_fA2, hilp_fB2,
		hilp_aL1, hilp_aL2, hilp_aR1, hilp_aR2,
		hilp_cL1, hilp_cL2, hilp_cR1, hilp_cR2,
		hilp_eL1, hilp_eL2, hilp_eR1, hilp_eR2,
		hilp_total
	};
	double highpass[hilp_total];
	double lowpass[hilp_total];	
		
	double panA;
	double panB;
	double inTrimA;
	double inTrimB;
	
	double avg32L[33];
	double avg32R[33];
	double avg16L[17];
	double avg16R[17];
	double avg8L[9];
	double avg8R[9];
	double avg4L[5];
	double avg4R[5];
	double avg2L[3];
	double avg2R[3];
	int avgPos;
	double lastSlewL;
	double lastSlewR;
	double lastSlewpleL;
	double lastSlewpleR;
	//preTapeHack	
	
    uint32_t fpdL;
    uint32_t fpdR;
    //default stuff
    
    double peakLeft = 0.0;
    double peakRight = 0.0;
    double slewLeft = 0.0;
    double slewRight = 0.0;
    double rmsLeft = 0.0;
    double rmsRight = 0.0;
    double previousLeft = 0.0;
    double previousRight = 0.0;
    double zeroLeft = 0.0;
    double zeroRight = 0.0;
    double longestZeroLeft = 0.0;
    double longestZeroRight = 0.0;
    double maxComp = 1.0;
    double baseComp = 0.0;
    double didClip = 0.0;
    bool wasPositiveL = false;
    bool wasPositiveR = false;
    int rmsCount = 0;

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PluginProcessor)
};
