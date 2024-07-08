#pragma once

#include <JuceHeader.h>

//==============================================================================
/*
    This component lives inside our window, and this is where you should put all
    your controls and content.
*/
class MainComponent  : public juce::AudioAppComponent, public juce::ChangeListener, public juce::MouseListener, 
	public juce::Timer
{
public:
    //==============================================================================
    MainComponent();
    ~MainComponent() override;

    //==============================================================================
    void prepareToPlay (int samplesPerBlockExpected, double sampleRate) override;
    void getNextAudioBlock (const juce::AudioSourceChannelInfo& bufferToFill) override;
    void releaseResources() override;

    //==============================================================================
    void paint (juce::Graphics& g) override;
    void resized() override;

private:
	//!!! Перечисляем состояния аудиоплера (создаем свой тип)
	enum trasportState
	{
		Basic,
		Starting,
		Stopping,
		Playing
	};
	//!!!
	trasportState state;

	void buttonLoadClick();
	void buttonPlayClick();
	void buttonStopClick();
	void buttonHardClick();
	void trasportStateChanged(trasportState newState);
	void changeListenerCallback(juce::ChangeBroadcaster* source) override; 

	void mouseUp(const juce::MouseEvent& event) override;


	void timerCallback() override;

	juce::AudioFormatManager audioFormatManager; //!!! менеджер форматов(расширений) аудио
	std::unique_ptr<juce::AudioFormatReaderSource>  playSource;

	juce::TextButton buttonLoad; //!!! кнопка с текстом
	juce::TextButton buttonPlay;
	juce::TextButton buttonStop;
	juce::TextButton buttonHard;

	juce::ProgressBar timeTrackProgress;
	//std::unique_ptr<juce::ProgressBar> timeTrackProgress;
	double currentProgress;
	int minutes;
	int second;
	int sizeTrack;
	double timeSet;
	double phaseProgress;
	//double posTransport;

	juce::AudioTransportSource transport; //!!! класс, что управляет состоянием аудио (Play, Stop, начало воспроизведения...)
	double transportPosition;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainComponent)
   
};
