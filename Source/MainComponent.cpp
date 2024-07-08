#include "MainComponent.h"
//!!!
#include <string>
using namespace std;
//!!!


//==============================================================================
MainComponent::MainComponent() : state(Basic), buttonLoad("Load"), buttonPlay("Play"), buttonStop("Stop"),
buttonHard("Reset Zero"), timeTrackProgress(currentProgress)
{                              //!!задаем начальное состояние
    // Make sure you set the size of the component after
    // you add any child components.
    setSize (600, 400);

	//!!!! Лямбда функция 
	buttonLoad.onClick = [this] { buttonLoadClick(); };
	//!!! В [] фиксируется отслеживаемое событие
	buttonPlay.onClick = [this] { buttonPlayClick(); };
	buttonStop.onClick = [this] { buttonStopClick(); };
	buttonHard.onClick = [this] { buttonHardClick(); };

	addAndMakeVisible(&buttonLoad);
	addAndMakeVisible(&buttonPlay);
	addAndMakeVisible(&buttonStop);
	addAndMakeVisible(&buttonHard);
	addAndMakeVisible(&timeTrackProgress); //!!

	timeTrackProgress.addMouseListener(this, false);
	
	buttonLoad.setColour(juce::TextButton::buttonColourId, juce::Colours::skyblue);
	buttonLoad.setColour(juce::TextButton::textColourOffId, juce::Colours::whitesmoke);
	//!! juce::TextButton::buttonColourId - заполнением цветом когда кнопка не нажата,
	//!! juce::TextButton::buttonOnColourId - заполнением цветом когда кнопка нажата (???),
	//!! juce::TextButton::textColourOffId/juce::TextButton::textColourOffId (???) - для цвета текста
	buttonPlay.setColour(juce::TextButton::buttonColourId, juce::Colours::limegreen);
	buttonPlay.setColour(juce::TextButton::textColourOffId, juce::Colours::whitesmoke); 
	buttonStop.setColour(juce::TextButton::buttonColourId, juce::Colours::orangered);
	buttonStop.setColour(juce::TextButton::textColourOffId, juce::Colours::whitesmoke);
	buttonHard.setColour(juce::TextButton::buttonColourId, juce::Colours::red);
	buttonHard.setColour(juce::TextButton::textColourOffId, juce::Colours::whitesmoke);

	buttonPlay.setEnabled(true); //!! компонент включен
	buttonStop.setEnabled(false); //!! компонент выключен
	
	audioFormatManager.registerBasicFormats(); //!!! регистриркем форматы по умолчанию (это Wav и Aiff) 
	
	transport.addChangeListener(this); //!!!

    // Some platforms require permissions to open input channels so request that here
    if (juce::RuntimePermissions::isRequired (juce::RuntimePermissions::recordAudio)
        && ! juce::RuntimePermissions::isGranted (juce::RuntimePermissions::recordAudio))
    {
        juce::RuntimePermissions::request (juce::RuntimePermissions::recordAudio,
                                           [&] (bool granted) { setAudioChannels (granted ? 2 : 0, 2); });
    }
    else
    {
        // Specify the number of input and output channels that we want to open
        //setAudioChannels (2, 2);
		setAudioChannels(2, 2);
    }
}

MainComponent::~MainComponent()
{
    // This shuts down the audio device and clears the audio source.
    shutdownAudio();
}

//==============================================================================
void MainComponent::prepareToPlay (int samplesPerBlockExpected, double sampleRate)
{
	transport.prepareToPlay(samplesPerBlockExpected, sampleRate); //!! указываем начальные данные для воспроизведения
	//(размер буфера и частоту семплов)

	currentProgress = 0.0;

	timeTrackProgress.setTextToDisplay("0:0");
	transportPosition = 0.0;
}

void MainComponent::getNextAudioBlock (const juce::AudioSourceChannelInfo& bufferToFill)
{
    bufferToFill.clearActiveBufferRegion();
	transport.getNextAudioBlock(bufferToFill); //!!!
}

void MainComponent::releaseResources()
{
	trasportStateChanged(Stopping);
}

//==============================================================================
void MainComponent::paint (juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));

    // You can add your drawing code here!
}

void MainComponent::buttonLoadClick()
{
	//trasportStateChanged(Stopping);

	//DBG("It's Work!"); //!! выводит значение в консоль (не выводит кирилицу)

	//выбор файла
	juce::FileChooser chooser("Choose the File", juce::File::getSpecialLocation(juce::File::userDesktopDirectory),
		"*.wav,*.aiff,*.mp3");
	
	/*
	??
	juce::FileChooser choser(заголовок (не работает с кирилицей), указание директории для открытия файлов по умолчанию (userDesktopDirectory - на рабочий стол),
		значение филтра расширений (можно перечелять через запятую), использовать ли собственное диалоговое окно?,
		обрабатывать ли пакеты файлов как каталоги);
	??
    */

	if (chooser.browseForFileToOpen()) {
	juce::File opFile;
	//сделал пользователь?
	opFile = chooser.getResult(); //!!! возвращает объект выбранного файла
	//чтение файла
	juce::AudioFormatReader* reader = audioFormatManager.createReaderFor(opFile);

	if (reader != nullptr) {
	//получаем файл для проигрывания
	std:unique_ptr<juce::AudioFormatReaderSource> tempSource(new juce::AudioFormatReaderSource(reader, true));
		//DBG(reader -> getFormatName());  //! выводит в концоль тип формата                            //чтобы он удалялся после использования
		
		transport.setSource(tempSource.get()); //!! подключение ресурсов к transport
		
		trasportStateChanged(Basic);

		playSource.reset(tempSource.release()); //???


		timeTrackProgress.setTextToDisplay("0:0");
		stopTimer();
		transportPosition = 0.0;
		currentProgress = 0.0;


		sizeTrack = transport.getLengthInSeconds();
	    }
	}

}

void MainComponent::buttonPlayClick()
{
	trasportStateChanged(Starting);
}

void MainComponent::buttonStopClick()
{
	trasportStateChanged(Stopping); 
}

void MainComponent::buttonHardClick()
{
	transport.setPosition(0.0);
	transportPosition = 0.0;
	minutes = 0;
	second = 0;
}

void MainComponent::trasportStateChanged(trasportState newState)
{
	if (newState != state) {
		state = newState;

		switch (state)
		{
		case MainComponent::Basic:
			buttonPlay.setEnabled(true);
			buttonStop.setEnabled(false);
			timeTrackProgress.setTextToDisplay(to_string(minutes) + ":"
				+ to_string(second));
			stopTimer();
			transport.stop();
			break;
		case MainComponent::Starting:
			buttonPlay.setEnabled(false);
			buttonStop.setEnabled(true);
			transport.start();
			//!!
			transport.setPosition(transportPosition);
			//!!
			startTimerHz(1);
			break;
		case MainComponent::Stopping:
			buttonPlay.setEnabled(true);
			buttonStop.setEnabled(false);
			transportPosition = transport.getCurrentPosition();
			stopTimer();
			transport.stop();
			break;
		case MainComponent::Playing:
			buttonPlay.setEnabled(false);
			buttonStop.setEnabled(true);
			break;
		default:
			break;
		}
		//Stopping
		 //playbutton disenable
		 //transport stop

		//Starting
		 //stopbutton disenable
		 //transport start
		
		//Stopped
		 //bring transport back to the beginning
	}
}

void MainComponent::changeListenerCallback(juce::ChangeBroadcaster* source)
{
	//!! Является ли воспроизводимый источник transport
	if (source == &transport) {
		//!! Состояние transport - play
		if (transport.isPlaying()) {
			trasportStateChanged(Playing);
		}
		else {
			trasportStateChanged(Basic);
		}
	}

}

void MainComponent::mouseUp(const juce::MouseEvent& event)
{
	//!!!
	if (timeTrackProgress.isMouseOverOrDragging()) {
		timeSet =(double)((double)(event.getPosition().getX()) / timeTrackProgress.getWidth()) * sizeTrack;
		transport.setPosition(timeSet);
		
	}
}




void MainComponent::timerCallback()
{
	//if ((state = Starting) || (state = Playing)) {
	if (transport.isPlaying()) {
		minutes = (int)(transport.getCurrentPosition() / 60);
		second = (int)(transport.getCurrentPosition() - 60 * minutes);
		transportPosition = transport.getCurrentPosition();
	}

	timeTrackProgress.setTextToDisplay(to_string(minutes) + ":"
		+ to_string(second));
	currentProgress = transportPosition / sizeTrack;

	phaseProgress = (getWidth() - 500) / 2;
}




void MainComponent::resized()
{
	buttonLoad.setBounds((getWidth() - 100) / 2, getHeight() - 100, 100, 30);
	buttonPlay.setBounds((getWidth() - 230) / 2, getHeight() - 160, 100, 30);
	buttonStop.setBounds(((getWidth() - 230) / 2) + 130, getHeight() - 160, 100, 30);
	buttonHard.setBounds(((getWidth() - 100) / 2) + 130, getHeight() - 100, 100, 30);
	timeTrackProgress.setBounds((getWidth() - 500) / 2, getHeight() - 220, 500, 30);
	// This is called when the MainContentComponent is resized.
	// If you add any child components, this is where you should
	// update their positions.
}
