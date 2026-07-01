import QtQuick 2.0
import Sailfish.Silica 1.0

Page {
    id: page

    SilicaFlickable {
        anchors.fill: parent
        contentHeight: column.height

        Column {
            id: column
            width: page.width
            spacing: Theme.paddingLarge

            PageHeader {
                title: qsTr("About TilEm")
            }

            Image {
                anchors.horizontalCenter: parent.horizontalCenter
                source: "/usr/share/icons/hicolor/172x172/apps/harbour-tilem.png"
                width: Theme.iconSizeLarge
                height: Theme.iconSizeLarge
            }

            Label {
                anchors.horizontalCenter: parent.horizontalCenter
                text: "TilEm"
                font.pixelSize: Theme.fontSizeLarge
                font.bold: true
            }

            Label {
                anchors.horizontalCenter: parent.horizontalCenter
                text: "v0.3.0"
                color: Theme.secondaryColor
                font.pixelSize: Theme.fontSizeSmall
            }

            SectionHeader {
                text: qsTr("Description")
            }

            Label {
                anchors {
                    left: parent.left
                    right: parent.right
                    leftMargin: Theme.horizontalPageMargin
                    rightMargin: Theme.horizontalPageMargin
                }
                wrapMode: Text.WordWrap
                color: Theme.highlightColor
                font.pixelSize: Theme.fontSizeSmall
                text: qsTr("TilEm is an emulator for Texas Instruments Z80-based graphing calculators. " +
                          "It supports TI-73, TI-76.fr, TI-81, TI-82, TI-83, TI-83+, TI-84+, TI-85, and TI-86 models.")
            }

            SectionHeader {
                text: qsTr("Usage")
            }

            Label {
                anchors {
                    left: parent.left
                    right: parent.right
                    leftMargin: Theme.horizontalPageMargin
                    rightMargin: Theme.horizontalPageMargin
                }
                wrapMode: Text.WordWrap
                color: Theme.secondaryHighlightColor
                font.pixelSize: Theme.fontSizeExtraSmall
                text: qsTr("You need a calculator ROM image to use this emulator. ROM images are copyrighted by Texas Instruments and cannot be distributed. " +
                          "Place your ROM files (.rom, .tib, .8xu) in the Documents folder.")
            }

            SectionHeader {
                text: qsTr("Credits")
            }

            Label {
                anchors {
                    left: parent.left
                    right: parent.right
                    leftMargin: Theme.horizontalPageMargin
                    rightMargin: Theme.horizontalPageMargin
                }
                wrapMode: Text.WordWrap
                color: Theme.secondaryColor
                font.pixelSize: Theme.fontSizeExtraSmall
                text: "TilEm Core: Benjamin Moody\n" +
                      "TilEm-Qt: Hugues Luc Bruant\n" +
                      "Ubuntu Touch Port: Sam Segers\n" +
                      "SailfishOS Port: Community"
            }

            SectionHeader {
                text: qsTr("License")
            }

            Label {
                anchors {
                    left: parent.left
                    right: parent.right
                    leftMargin: Theme.horizontalPageMargin
                    rightMargin: Theme.horizontalPageMargin
                }
                wrapMode: Text.WordWrap
                color: Theme.secondaryColor
                font.pixelSize: Theme.fontSizeExtraSmall
                text: "GPL v3 / LGPL 2.1"
            }
        }

        VerticalScrollDecorator {}
    }
}
