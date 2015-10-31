import QtQuick 2.3
import QtQuick.Controls 1.2

Item {
    id:root
    property string crdTName
    property string crdTUser
    property string crdTHop
    property string crdTPass
    property string crdTSubmit

   // anchors.fill: parent
    Rectangle{
        anchors.fill: parent
        color:"lightgrey"
        Column{
            anchors.fill: parent
            spacing:15

            //EDIT ITEM
            Rectangle{
                color:"white"
                width:parent.width
                height:crdEName.height + crdETName.height
                Column{
                    anchors.fill:parent
                    Text{
                        anchors.horizontalCenter: parent.horizontalCenter
                        id:crdETName
                        font.bold: true
                        font.pixelSize: 25
                        text: "Name:"
                    }
                    TextEdit{
                        anchors.horizontalCenter: parent.horizontalCenter
                        id:crdEName
                        //font.bold: true
                        font.pixelSize: 25
                        text: root.crdTName
                    }
                }
             }

            //EDIT ITEM
            Rectangle{
                color:"white"
                width:parent.width
                height:crdEName.height + crdETName.height
                Column{
                    anchors.fill:parent
                    Text{
                        anchors.horizontalCenter: parent.horizontalCenter
                        id:crdETUser
                        font.bold: true
                        font.pixelSize: 25
                        text: "User:"
                    }
                    TextEdit{
                        anchors.horizontalCenter: parent.horizontalCenter
                        id:crdEUser
                        //font.bold: true
                        font.pixelSize: 25
                        text: root.crdTUser
                    }

                }
             }

            //EDIT ITEM
            Rectangle{
                color:"white"
                width:parent.width
                height:crdEName.height + crdETName.height
                Column{
                    anchors.fill:parent
                    Text{
                        anchors.horizontalCenter: parent.horizontalCenter
                        id:crdETHop
                        font.bold: true
                        font.pixelSize: 25
                        text: "Hop:"
                    }
                    Text{
                        anchors.horizontalCenter: parent.horizontalCenter
                        id:crdEHop
                        //font.bold: true
                        font.pixelSize: 25
                        text: root.crdTHop
                    }
                }
             }

            //EDIT ITEM
            Rectangle{
                color:"white"
                width:parent.width
                height:crdEName.height + crdETName.height
                Column{
                    anchors.fill:parent
                    Text{
                        anchors.horizontalCenter: parent.horizontalCenter
                        id:crdETPass
                        font.bold: true
                        font.pixelSize: 25
                        text: "Pass:"
                    }
                    TextEdit{
                        anchors.horizontalCenter: parent.horizontalCenter
                        id:crdEPass
                        //font.bold: true
                        font.pixelSize: 25
                        text: root.crdTPass
                    }
                }
             }

            //OK BUTTON
            Rectangle{
                color: "grey"
                height:60
                width:parent.width

                Image{
                    id:icon
                    anchors.horizontalCenter: parent.horizontalCenter
                    anchors.verticalCenter: parent.verticalCenter
                    source:"icons/fi-plus.svg"
                    sourceSize.width: 40
                }

                Text {
                    id:btnText
                    width:parent.width
                    anchors.left: icon.right
                    anchors.verticalCenter: icon.verticalCenter
                    text: "OK"
                    font.bold: true
                }

                MouseArea{
                    anchors.fill: parent
                    onClicked:{
                        crdEdit.x = crdEdit.width;
                    }
                }
            }
        }
    }
}
