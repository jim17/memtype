import QtQuick 2.3

Rectangle{ //TOP BLOCK
    id:root
    color:"lightgrey"
    width:parent.width
    height:150
    property string version: "Version Unknown"
    property string space: "Memory space Unknown"
    property string locked: "Locked"


    Row{
        anchors.fill:parent
        Image{
            anchors.verticalCenter: parent.verticalCenter
            source:"icons/fi-usb.svg"
            //sourceSize.height:100
            sourceSize.width:140
            MouseArea{
                anchors.fill: parent
                onClicked:{
                    //console.log(Launcher.getInfo());
                    var infomsg = Launcher.getInfo().split(';');
                    root.version = infomsg[0];
                    root.space = infomsg[1];
                    //console.log(Launcher.getError());
                }
            }
        }
        Column{
            width:500
            Text{
                anchors.horizontalCenter: parent.horizontalCenter
                text:"MemType"
                font.pixelSize: 50

            }
            Text{
                anchors.horizontalCenter: parent.horizontalCenter
                textFormat:Text.RichText
                text:"<a href='http://www.area0x33.com/blog/'>www.area0x33.com</a>"
                font.pixelSize: 14
                onLinkActivated: Qt.openUrlExternally(link)


            }
            Rectangle{ //VSpacer
                color:root.color
                width:parent.width
                height:20
            }

            Row{ //Info Row
                width: parent.width

                anchors.horizontalCenter: parent.horizontalCenter
                Text{
                    //color:"red"
                    height:20
                    text:root.version
                    width:parent.width/3
                    horizontalAlignment:Text.AlignHCenter

                }
                Text{
                    //color:"red"
                    height:20
                    text:root.space
                    width:parent.width/3
                    horizontalAlignment:Text.AlignHCenter

                }
                Text{
                    //color:"red"
                    height:20
                    text:root.locked
                    width:parent.width/3
                    horizontalAlignment:Text.AlignHCenter
                }

            }
        }
    }

}
