// * Name: ofApp.cpp
// * Project: Footfall
// * Author: David Haylock
// * Creation Date: 13/02/2017
// * Copyright: (c) 2017 by Watershed Arts Trust Ltd.


#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup()
{
	ofSetVerticalSync(true);
	ofSetFrameRate(25);

	cout << "-------------------- Footfall --------------------" << endl;

	configManager.loadConfiguration("config.json");

	char hostname[HOST_NAME_MAX];
	gethostname(hostname, HOST_NAME_MAX);
	_hostname = ofToString(hostname);

	_sequence = 0;
	_sep = ",";

	_logToCsv = configManager.getConfiguration().useCsvLogging;
	_logToServer = configManager.getConfiguration().useMQTT;

	cameraManager.setup(configManager.getConfiguration().cameraConfig);
	trackingManager.setup(configManager.getConfiguration().trackingConfig);

	if (_logToServer) mqttManager.setup(configManager.getConfiguration().mqttConfig);
	if (_logToCsv) csvManager.setup("csvlogs");

	ofAddListener(trackingManager.blobIn, this, &ofApp::blobIn);
	ofAddListener(trackingManager.blobOut, this, &ofApp::blobOut);
}
//--------------------------------------------------------------
void ofApp::exit()
{
	if (_logToServer) mqttManager.close();
	if (_logToCsv) csvManager.close();

	ofRemoveListener(trackingManager.blobIn, this, &ofApp::blobIn);
	ofRemoveListener(trackingManager.blobOut, this, &ofApp::blobOut);
}
//--------------------------------------------------------------
void ofApp::update()
{
	cameraManager.update();
	trackingManager.update(cameraManager.getImage());
}
//--------------------------------------------------------------
void ofApp::draw()
{
	ofPushMatrix();
	ofScale(1,1);
	cameraManager.draw();
	trackingManager.draw();
	ofPopMatrix();

	stringstream ss;
	ss << "Footfall" << endl;
	ss << "People In: " << peopleIn;
	ss << " People Out: " << peopleOut;
	ss << " Total: " << (peopleIn+abs(peopleOut));
	ss << " FPS: " << ofGetFrameRate() << endl;
	ofDrawBitmapStringHighlight(ss.str(),7,ofGetHeight()-20);
}
//--------------------------------------------------------------
void ofApp::keyPressed(int key)
{

}
//--------------------------------------------------------------
void ofApp::keyReleased(int key)
{

}
//--------------------------------------------------------------
void ofApp::blobIn(int &val)
{
	// Blink green LED for 1 second
	system("sudo bash -c 'echo 1 >/sys/class/leds/led0/brightness &&
												sleep 1 &&
												echo 0 >/sys/class/leds/led0/brightness' & disown");
	peopleIn += val;
	cout << ofGetTimestampString("%Y-%m-%d %H:%M:%S") << " | seq "<< _sequence << " | +" << val << " blob(s) | " << peopleIn+abs(peopleOut) << " total" << endl;

	string csv = ofToString(time(0)) + _sep +
									 _hostname + _sep +
									 ofToString(val) + _sep +
									 ofToString(_sequence);

	if (_logToServer) mqttManager.publish(csv);
	if (_logToCsv) csvManager.addRecord(ofToString(val), ofGetTimestampString("%Y-%m-%d %H:%M:%S"));
	if (_logToCsv) csvManager.close();

	_sequence++;
}
//--------------------------------------------------------------
void ofApp::blobOut(int &val)
{
	// Blink green LED for 1 second
	system("sudo bash -c 'echo 1 >/sys/class/leds/led0/brightness &&
												sleep 1 &&
												echo 0 >/sys/class/leds/led0/brightness' & disown");
	peopleOut += abs(val);
	cout << ofGetTimestampString("%Y-%m-%d %H:%M:%S") << " | seq "<< _sequence << " | " << val << " blob(s) | " << peopleIn+abs(peopleOut) << " total" << endl;

	string csv = ofToString(time(0)) + _sep +
									 _hostname + _sep +
									 ofToString(val) + _sep +
									 ofToString(_sequence);

	if (_logToServer) mqttManager.publish(csv);
	if (_logToCsv) csvManager.addRecord(ofToString(val), ofGetTimestampString("%Y-%m-%d %H:%M:%S"));
	if (_logToCsv) csvManager.close();

	_sequence++;
}
