import QtQuick 2.3

Item {
    id:root
    anchors.horizontalCenter: parent.horizontalCenter
    width: 450
    height: 62
    property string crdName : "Credential"
    property string crdUser
    property string crdHop
    property string crdPass
    property string crdSubmit
    property string crdIdx

    Row{
        anchors.fill:parent

        Rectangle{ //Order
            height:parent.height
            width:30
            color:"grey"
        }
        Rectangle{
            height:parent.height
            width:370
            Text{ //Credential Name
                anchors.fill: parent
                font.bold: true
                font.pixelSize: 25
                text: root.crdName
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment:Text.AlignVCenter
            }

        }

        Rectangle{ // Indicator with 3 states indicating password strenght
            width: 20
            height:parent.height
            color:"green"

        }
        Rectangle{
            width: 30
            height:parent.height
            color:"white"
            Image{
                anchors.verticalCenter: parent.verticalCenter
                source:"icons/fi-trash.svg"
                sourceSize.height: parent.height
                sourceSize.width: 30
            }
        }
    }
    MouseArea{
        anchors.fill:parent
        onClicked: {
            //Fill in values into the edit form
            crdEdit.crdTName = root.crdName;
            crdEdit.crdTUser = root.crdUser;
            crdEdit.crdTHop = root.crdHop;
            crdEdit.crdTPass = root.crdPass;
            crdEdit.crdTSubmit = root.crdSubmit;

            //Show edit form
            crdEdit.x = 0;
        }
    }


}
