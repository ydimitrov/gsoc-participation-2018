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
import QtQuick.Controls 2.0

ListView {
    id: root
    clip: true
    width: 318
    height: 219 * 2 + 20
    spacing: 20
    opacity: enabled ? 1 : 0.5

    property int degree: currentIndex > -1 ? model.get(currentIndex).value : -1
    model: ListModel {
        Component.onCompleted: {
            append({value: 15, modelData: translator.translate(qsTr('LO'), translator.language)})
            for (var d = 16; d < 30; d++) {
                append({value: d, modelData: translator.translate(qsTr('%1\u00b0'), translator.language).arg(d.toFixed(0))})
            }
            append({value: 30, modelData: translator.translate(qsTr('HI'), translator.language)})
        }
    }
    delegate: Label {
        width: ListView.view.width
        height: 219
        horizontalAlignment: Label.AlignHCenter
        verticalAlignment: Label.AlignVCenter
        text: model.modelData
        font.pixelSize: height * 0.7
        color: (ListView.view.enabled && ListView.isCurrentItem) ? '#00ADDC' : 'white'
    }

    preferredHighlightBegin: 0.5
    preferredHighlightEnd: 0.5
    highlightRangeMode: ListView.StrictlyEnforceRange
    highlight: Rectangle {
        color: 'white'
        opacity: 0.2
    }
}
