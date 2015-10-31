import QtQuick 2.3
import QtQuick.Controls 1.2

ApplicationWindow {
    id:mainWin
    visible: true
    width: 640
    //height: 480
    title: qsTr("MemType GUI")
    property string xml

    Column{
        anchors.fill:parent
        TopBlock{
        }

         Row{
         width:parent.width
         height:400
            MenuBlock{

            }

            Rectangle{ //MAIN BLOCK
                color:"lightgrey"
                height: parent.height
                width:500
                Row{
                    anchors.fill:parent
                    spacing:15
                    CredentialModel{
                        id:credM

                   }
                    CredentialEdit{
                        id:crdEdit
                        x: parent.width
                        height: parent.height
                        width: parent.width
                        y: parent.y
                        Behavior on x {
                            NumberAnimation{ duration : 100}
                        }

                    }

                }

            }
        }
    }

}
