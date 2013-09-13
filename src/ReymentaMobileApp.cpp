#include "ReymentaMobileApp.h"

void ReymentaMobileApp::prepareSettings(Settings* settings)
{
	settings->setFrameRate(50.0f);
	settings->setWindowSize(960, 600);
	settings->enableConsoleWindow();
}

void ReymentaMobileApp::setup()
{
	// OSC sender
	destinationHost = "192.168.0.18";
	destinationPort = 9999;
	nanoPort = 0;
	mOSCSender.setup(destinationHost, destinationPort);
	// midi
	if (mMidiIn.getNumPorts() > 0)
	{
		mMidiIn.listPorts();
		for (int i = 0; i < mMidiIn.getNumPorts()-1; i++)
		{
			console() << "port:" << mMidiIn.mPortNames[i] << std::endl;
			size_t found = mMidiIn.mPortNames[i].find("nano");
			if ( found != string::npos )
			{
				nanoPort = i;
				nanoPortName = mMidiIn.mPortNames[i];
			}
		}

		mMidiIn.openPort(nanoPort);
		console() << "Opening MIDI port " << nanoPort << " " << nanoPortName << std::endl;
	}
	else 
	{
		console() << "No MIDI Ports found!!!!" << std::endl;
	}

	// AUDIO
	// Initialize array
	mData = new float[1024];
	vector<float> mbuffer; 
    for(int i = 0; i < 1024; i++)
    {
        mData[i] = 0;
        mbuffer.push_back(0);        
    }
	// Create audio input
	mInput = AudioInput::create();

	// Bail if no devices present
	if ( mInput->getDeviceCount() <= 0 ) {
		return;
	}

	// Start receiving audio
	mInput->addCallback( &ReymentaMobileApp::onData, this );
	mInput->start();

	// List devices
	DeviceList devices = mInput->getDeviceList();
	for ( DeviceList::const_iterator deviceIt = devices.begin(); deviceIt != devices.end(); ++deviceIt ) {
		console() << deviceIt->second << std::endl;
	}

	//GUI
	backgroundColor.r = 0.5; 
	backgroundColor.g = 0.5; 
	backgroundColor.b = 0.5;  

	backgroundColor = ColorA(233.0/255.0, 52.0/255.0, 27.0/255.0); 
	position = Vec2f(getWindowWidth()*.5, getWindowHeight()*.5); 

	guiWidth = getWindowWidth(); 
	guiHeight= getWindowHeight(); 
	dim = 40; 
	length = guiWidth/2;

	gui = new ciUICanvas(0,0,guiWidth, guiHeight);
	gui->setTheme( CI_UI_THEME_RUSTICORANGE );

	for (int c = 0; c < 128; c++)
	{
		controlValues[c] = 0;
	}

	for (int i = 0; i < 8; i++)
	{
		stringstream rotaryName; 
		rotaryName << i + 11;
		rotarys.push_back( new ciUIRotarySlider( dim*2.5, 0.0, 127.0, controlValues[i + 11], rotaryName.str() ) ); //0x0B = 11
		if ( i == 0 )
		{
			gui->addWidgetSouthOf(rotarys[i], "IP");
		}
		else
		{
			gui->addWidgetEastOf(rotarys[i], rotarys[i-1]->getName());
		}

		stringstream toggleName0; 
		toggleName0 << i + 21;
		togglesUp.push_back( new ciUILabelToggle( dim, controlValues[i + 21], toggleName0.str(), CI_UI_FONT_MEDIUM, dim) );//0x15 = 21
		gui->addWidgetSouthOf(togglesUp[i], rotarys[i]->getName());

		stringstream toggleName1;
		toggleName1 << i + 31;
		togglesMid.push_back( new ciUILabelToggle(dim, controlValues[i + 31], toggleName1.str(), CI_UI_FONT_MEDIUM, dim) );// 0x1F = 31
		gui->addWidgetSouthOf(togglesMid[i], togglesUp[i]->getName());

		stringstream toggleName2;
		toggleName2 << i + 41;
		togglesDown.push_back( new ciUILabelToggle(dim, controlValues[i + 41], toggleName2.str(), CI_UI_FONT_MEDIUM, dim) );// 0x29 = 41
		gui->addWidgetSouthOf(togglesDown[i], togglesMid[i]->getName());

		stringstream sliderName; 
		sliderName << i + 1;
		sliders.push_back( new ciUISlider( dim, length/3.0 - CI_UI_GLOBAL_WIDGET_SPACING, 0.0, 127.0, controlValues[i + 1], sliderName.str() ) ); //0x01
		gui->addWidgetEastOf(sliders[i], togglesUp[i]->getName());
	}	
	// no midi in for pad
	float padWidth = 256; 
	float padHeight = 192; 
	gui->addWidgetDown(new ciUI2DPad(padWidth, padHeight, Vec2f(0, 1024), Vec2f(0, 768), Vec2f(padWidth*.5, padHeight*.5), "pad"));    
	// LaunchPad
	((ciUIToggleMatrix *)gui->addWidgetRight(new ciUIToggleMatrix(dim, dim, 4, 5, "LaunchPad")))->setAllowMultiple(false);    
	// ip text gui->addWidgetRight(new ciUITextInput(160, "IP", "Input Text", CI_UI_FONT_LARGE, 30.0f)); 
	host = new ciUITextInput(120, "ip", destinationHost, CI_UI_FONT_SMALL, 40.0f);
	host->setAutoClear(false);
	gui->addWidgetRight(host); 
	// port
	port = new ciUITextInput(80, "port", "9999", CI_UI_FONT_SMALL, 40.0f);
	port->setAutoClear(false);
	gui->addWidgetRight(port); 
	// cnx btn
	gui->addWidgetRight( new ciUILabelButton(80, false, "connect", CI_UI_FONT_SMALL, 40.0f) );
	// audio
	gui->addWidgetSouthOf(new ciUILabel("audio", CI_UI_FONT_SMALL), "ip");        
    gui->addWidgetSouthOf(new ciUIWaveform(240, 30, mData, 1024, -1.0, 1.0, "waveform"), "audio");  
    /*gui->addWidgetDown(new ciUISpectrum(240, 30, mData, 1024, -1.0, 1.0, "SPECTRUM"));  */
	gui->addWidgetSouthOf(new ciUILabel("fps", CI_UI_FONT_SMALL), "waveform");        

    mvg = (ciUIMovingGraph *) gui->addWidgetSouthOf(new ciUIMovingGraph(240, 30, mbuffer, 1024, 0, 120, "fpsmvg"), "fps");    

	// status
	status = new ciUILabel("status", CI_UI_FONT_SMALL);
	gui->addWidgetSouthOf( status, "fpsmvg" );    

	gui->registerUIEvents(this, &ReymentaMobileApp::guiEvent);
		

}
// Called when buffer is full
void ReymentaMobileApp::onData( float *data, int32_t size )
{
	mData = data;
}

void ReymentaMobileApp::draw()
{
	gl::clear(Color(0,0,0), true);
	//gl::drawSolidRect(Rectf(Vec2f(0, 0), Vec2f(sliderValue * getWindowWidth(), 5)));
	gui->draw();

}

void ReymentaMobileApp::keyDown( KeyEvent event )
{
	if(event.getChar() == 'g')
	{
		gui->toggleVisible(); 
	}
	else if(event.getChar() == 's')
	{
		gui->saveSettings("guiSettings.xml"); 
	}
	else if(event.getChar() == 'l')
	{
		gui->loadSettings("guiSettings.xml"); 
	}
}
void ReymentaMobileApp::sendOSCMessage( string controlType, string controlName, int controlValue0, int controlValue1 )
{
	stringstream address; 
	address << controlType << controlName;
	osc::Message m;
	m.setAddress( address.str() );
	m.addIntArg( controlValue0 );	
	m.addIntArg( controlValue1 );	
	mOSCSender.sendMessage( m );
	address << ": " << controlValue0 << ", " << controlValue1;
	status->setLabel( address.str() );
}

void ReymentaMobileApp::guiEvent(ciUIEvent *event)
{
	string controlName = event->widget->getName(); 
	int name = atoi( controlName.c_str() );

	// slider
	if ( name < 9 )
	{
		ciUISlider *slider = (ciUISlider *) event->widget; 
		int newValue = slider->getScaledValue();
		if ( newValue != controlValues[name] )
		{
			controlValues[name] = newValue;
			sendOSCMessage( "/slider/", controlName, slider->getScaledValue() );	
		}
	}
	// toggle
	if ( name > 20 && name < 49 )
	{
		ciUILabelToggle *toggle = (ciUILabelToggle *) event->widget; 
		int newValue = toggle->getValue();
		if ( newValue != controlValues[name] )
		{
			controlValues[name] = newValue;
			sendOSCMessage( "/toggle/", controlName, toggle->getValue() );
		}
	}
	// rotary
	if ( name > 10 && name < 19 )
	{
		ciUIRotarySlider *rotary = (ciUIRotarySlider *) event->widget; 
		int newValue = rotary->getScaledValue();
		if ( newValue != controlValues[name] )
		{
			controlValues[name] = newValue;
			sendOSCMessage( "/rotary/", controlName, rotary->getScaledValue() );
		}		
	}
	size_t found = controlName.find("pad");
	if ( found != string::npos )
	{
		ciUI2DPad *pad = (ciUI2DPad *) event->widget; 
		sendOSCMessage( "/pad/", "0", pad->getScaledValue().x, pad->getScaledValue().y );	
	}
	found = controlName.find("ip");
	if ( found != string::npos )
	{
		cout << host->getTextString() << endl;	
	}
	found = controlName.find("port");
	if ( found != string::npos )
	{
		cout << port->getTextString() << endl;	
	}
	if ( controlName == "connect" ) 
	{
		destinationHost = host->getTextString();
		destinationPort = atoi( port->getTextString().c_str() );
		mOSCSender.setup(destinationHost, destinationPort);
	}
}

void ReymentaMobileApp::processMidiMessage(midi::Message* message)
{
	console() << "midi port: " << message->port << " ch: " << message->channel << " status: " << message->status;
	console() << " byteOne: " << message->byteOne << " byteTwo: " << message->byteTwo << std::endl;

	switch (message->status) 
	{
	case MIDI_CONTROL_CHANGE:
		/*if (message->byteOne == SLIDER_NOTE2){
		sliderValue = message->byteTwo / 127.0f;
		}*/
		int name = message->byteOne;
		int newValue = message->byteTwo;
		stringstream ssName; 
		ssName << name;
		string controlName = ssName.str(); 

		// slider
		if ( name < 9 )
		{
			ciUISlider *slider = (ciUISlider *) gui->getWidget( controlName ); 
			if ( newValue != controlValues[name] )
			{
				controlValues[name] = newValue;
				slider->setValue(newValue);
				sendOSCMessage( "/slider/", controlName, newValue );	
			}
		}
		// toggle
		if ( name > 20 && name < 49 )
		{
			ciUILabelToggle *toggle = (ciUILabelToggle *) gui->getWidget( controlName ); 
			if ( newValue != controlValues[name] )
			{
				controlValues[name] = newValue;
				toggle->setValue(newValue);
				sendOSCMessage( "/toggle/", controlName, newValue );
			}
		}
		// rotary
		if ( name > 10 && name < 19 )
		{
			ciUIRotarySlider *rotary = (ciUIRotarySlider *) gui->getWidget( controlName ); 
			if ( newValue != controlValues[name] )
			{
				controlValues[name] = newValue;
				rotary->setValue(newValue);
				sendOSCMessage( "/rotary/", controlName, newValue );
			}		
		}

		break;
	}
}
void ReymentaMobileApp::update()
{
	while (mMidiIn.hasWaitingMessages()) {
		midi::Message message;
		mMidiIn.getNextMessage(&message);

		processMidiMessage(&message);

	}
	gui->update(); 
	mvg->addPoint(getAverageFps());
	// We have audio data
	if ( mData != 0 ) 
	{
		float maxVolume = 0.0f;
		// Get size of data
		int32_t mDataSize = mInput->getDataSize();

		// Iterate through data and populate line
		for ( int32_t i = 0; i < mDataSize; i++ ) 
		{
			if ( mData[ i ] > maxVolume ) maxVolume = mData[ i ];
		}
		sendOSCMessage( "/volume/", "256", maxVolume*100 );
		console() << "volume: " << maxVolume*100 << endl;

	}
}

void ReymentaMobileApp::shutDown()
{
	// Stop input
	mInput->stop();

	// Free resources
	if ( mData != 0 ) {
		delete [] mData;
	}
	delete gui; 
}

// This line tells Cinder to actually create the application
CINDER_APP_BASIC( ReymentaMobileApp, RendererGl )
