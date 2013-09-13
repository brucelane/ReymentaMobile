/*
 Copyright (c) 2013, Bruce Lane - All rights reserved.
 This code is intended for use with the Cinder C++ library: http://libcinder.org

 This file is part of ReymentaMobile.

 ReymentaMobile is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.
 
 ReymentaMobile is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.
 
 You should have received a copy of the GNU General Public License
 along with ReymentaMobile.  If not, see <http://www.gnu.org/licenses/>.
*/


#include "cinder/app/AppBasic.h"
#include "OSCSender.h"
#include <list>
using namespace ci;
using namespace ci::app;
using namespace std;

#include "MidiIn.h"
#include "MidiMessage.h"
#include "MidiConstants.h"
#include "ciUI.h"

#include "AudioInput.h"

#define SLIDER_NOTE1 1
#define SLIDER_NOTE2 2
#define SLIDER_NOTE3 3
#define SLIDER_NOTE4 4
#define SLIDER_NOTE5 5
#define SLIDER_NOTE6 6
#define SLIDER_NOTE7 7
#define SLIDER_NOTE8 8

class ReymentaMobileApp : public AppBasic {
 public:
	void prepareSettings(Settings* settings);
	void setup();
	void update();
	void draw();
	void shutDown();
	void keyDown( KeyEvent event );

	void processMidiMessage(midi::Message* message);	
	void quitProgram();
	void sendOSCMessage( string controlType, string controlName, int controlValue0, int controlValue1 = 0 );
	void guiEvent( ciUIEvent *event );    

	ciUITextInput *host;
	ciUITextInput *port;
	ciUILabel *status;
	midi::Input mMidiIn;	
    ColorA backgroundColor; 
	Vec2f position; 
    ciUICanvas *gui;   
    ciUIMovingGraph *mvg; 

	map<int,int> controlValues;
	std::vector< ciUISlider * > sliders;
	std::vector< ciUIRotarySlider * > rotarys;
	std::vector< ciUILabelToggle * > togglesUp;
	std::vector< ciUILabelToggle * > togglesMid;
	std::vector< ciUILabelToggle * > togglesDown;
	std::string					destinationHost;
	int							destinationPort;
	// Audio callback
	void onData( float *data, int32_t size );

private:
	//bool						initialized;
	int							nanoPort;
	string						nanoPortName;
	osc::Sender					mOSCSender;
	// ciUI
    float						guiWidth; 
    float						guiHeight; 
    float						dim; 
    float						length; 
	// Audio input
	float			*mData;
	// float *buffer; 

	AudioInputRef	mInput;
};
