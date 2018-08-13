/*
 * Copyright (C) 2016 The Qt Company Ltd.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

import QtQuick 2.6
import QtWebSockets 1.0
import 'MessageId.js' as MessageId

WebSocket {
	id: root
	active: true

	property string statusString: "waiting..."

	property real fanSpeed: 0.0
	property real leftTemperature: 21.0
	property real rightTemperature: 21.0
	property string language: "en_US"

	property Connections c : Connections {
		target: root
		onFanSpeedChanged: {
			var json = [MessageId.call, '9999', 'hvac/set', {'FanSpeed': fanSpeed}]
			console.debug(JSON.stringify(json))
			sendTextMessage(JSON.stringify(json))
		}
		onLeftTemperatureChanged: {
			var json = [MessageId.call, '9999', 'hvac/set', {'LeftTemperature': leftTemperature}]
			console.debug(JSON.stringify(json))
			sendTextMessage(JSON.stringify(json))

			var json1 = [MessageId.call, '9999', 'hvac/temp_left_zone_led', {'LeftLed': leftTemperature}]
			console.debug(JSON.stringify(json1))
			sendTextMessage(JSON.stringify(json1))
		}
		onRightTemperatureChanged: {
			var json = [MessageId.call, '9999', 'hvac/set', {'RightTemperature': rightTemperature}]
			console.debug(JSON.stringify(json))
			sendTextMessage(JSON.stringify(json))

			var json1 = [MessageId.call, '9999', 'hvac/temp_right_zone_led', {'RightLed': rightTemperature}]
			console.debug(JSON.stringify(json1))
			sendTextMessage(JSON.stringify(json1))
		}
		onLanguageChanged: {
			var json = [MessageId.call, '9999', 'hvac/set', {'Language': language}]
			console.debug(JSON.stringify(json))
			sendTextMessage(JSON.stringify(json))
		}
	}

	onTextMessageReceived: {
		var json = JSON.parse(message)
		var request = json[2].request
		var response = json[2].response
		console.log("HVAC Binding Message: ",message)
		switch (json[0]) {
		case MessageId.call:
			break
		case MessageId.retok:
			root.statusString = request.status
			break
		case MessageId.reterr:
			root.statusString = "Bad return value, binding probably not installed"
			break
		case MessageId.event:
			if (json[1] == "hvac/language")
				console.log("HVAC event received: ",json[2])
				root.language = json[2].data
				root.statusString = "Language changed to "+language
			break
		}
	}
	onStatusChanged: {
		switch (status) {
		case WebSocket.Error:
			root.statusString = "WebSocket error: " + root.errorString
			break
		}
	}
}
