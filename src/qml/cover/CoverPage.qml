import QtQuick 2.0
import Sailfish.Silica 1.0

CoverBackground {
    id: cover

    Image {
        anchors.centerIn: parent
        source: "/usr/share/icons/hicolor/172x172/apps/harbour-tilem.png"
        width: parent.width * 0.6
        height: width
        fillMode: Image.PreserveAspectFit
    }

    Label {
        id: label
        anchors {
            top: parent.top
            topMargin: Theme.paddingLarge
            horizontalCenter: parent.horizontalCenter
        }
        text: "TilEm"
        font.pixelSize: Theme.fontSizeLarge
    }

    Label {
        anchors {
            bottom: parent.bottom
            bottomMargin: Theme.paddingLarge
            horizontalCenter: parent.horizontalCenter
        }
        text: "Calculator"
        font.pixelSize: Theme.fontSizeSmall
        color: Theme.secondaryColor
    }
}
