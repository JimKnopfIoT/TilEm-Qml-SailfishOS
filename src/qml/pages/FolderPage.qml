import QtQuick 2.0
import Sailfish.Silica 1.0
import Qt.labs.folderlistmodel 2.1

Page {
    id: page

    property string currentFolder: StandardPaths.home + "/Documents"

    SilicaFlickable {
        anchors.fill: parent

        PullDownMenu {
            MenuItem {
                text: qsTr("About")
                onClicked: pageStack.push(Qt.resolvedUrl("AboutPage.qml"))
            }
        }

        PageHeader {
            id: header
            title: qsTr("TilEm Calculator Emulator")
        }

        SilicaListView {
            id: listView
            anchors {
                top: header.bottom
                left: parent.left
                right: parent.right
                bottom: parent.bottom
            }
            clip: true

            model: FolderListModel {
                id: folderModel
                folder: "file://" + currentFolder
                showDirs: true
                showDotAndDotDot: true
                nameFilters: ["*.rom", "*.ROM", "*.tib", "*.TIB", "*.89u", "*.89U", "*.8xu", "*.8XU"]
            }

            header: Item {
                width: parent.width
                height: Theme.itemSizeMedium

                Label {
                    anchors {
                        left: parent.left
                        leftMargin: Theme.horizontalPageMargin
                        verticalCenter: parent.verticalCenter
                    }
                    text: currentFolder
                    color: Theme.secondaryHighlightColor
                    font.pixelSize: Theme.fontSizeSmall
                    truncationMode: TruncationMode.Fade
                }
            }

            delegate: ListItem {
                id: fileDelegate
                contentHeight: Theme.itemSizeSmall

                Row {
                    anchors {
                        left: parent.left
                        leftMargin: Theme.horizontalPageMargin
                        verticalCenter: parent.verticalCenter
                    }
                    spacing: Theme.paddingMedium

                    Image {
                        width: Theme.iconSizeSmall
                        height: Theme.iconSizeSmall
                        source: folderModel.isFolder(index)
                            ? "image://theme/icon-m-folder"
                            : "image://theme/icon-m-file-document"
                    }

                    Label {
                        text: fileName
                        color: fileDelegate.highlighted ? Theme.highlightColor : Theme.primaryColor
                        anchors.verticalCenter: parent.verticalCenter
                    }
                }

                onClicked: {
                    console.log("=== FOLDERPAGE ITEM CLICKED: " + fileName + " ===")
                    if (folderModel.isFolder(index)) {
                        if (fileName === "..") {
                            var parts = currentFolder.split("/")
                            parts.pop()
                            currentFolder = parts.join("/")
                            if (currentFolder === "") currentFolder = "/"
                        } else if (fileName === ".") {
                            // Stay in current folder
                        } else {
                            currentFolder = currentFolder + "/" + fileName
                        }
                    } else {
                        var romPath = currentFolder + "/" + fileName
                        console.log("=== ROM FILE SELECTED: " + romPath + " ===")
                        console.log("=== PUSHING CALCPAGE TO PAGESTACK ===")
                        pageStack.push(Qt.resolvedUrl("CalcPage.qml"), { romFile: romPath })
                        console.log("=== PAGESTACK PUSH RETURNED ===")
                    }
                }
            }

            VerticalScrollDecorator {}

            ViewPlaceholder {
                enabled: listView.count === 0
                text: qsTr("No ROM files found")
                hintText: qsTr("Place calculator ROM files (.rom, .tib) in Documents folder")
            }
        }
    }
}
