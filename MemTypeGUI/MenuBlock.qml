import QtQuick 2.3
import QtQuick.Dialogs 1.2

Rectangle{ //LEFT MENU
    id:root
    color:"darkgrey"
    height:parent.height
    width:140


    ListModel { //Menu buttons
       id: menuModel
       ListElement{ name: "New"; imgSource:"icons/fi-plus.svg"}
       ListElement{ name: "Read"; imgSource:"icons/fi-upload.svg"}
       ListElement{ name: "Write"; imgSource:"icons/fi-download.svg"}
       ListElement{ name: "Open"; imgSource:"icons/fi-page.svg"}
       ListElement{ name: "Save"; imgSource:"icons/fi-page-export.svg"}
       ListElement{ name: "Settings"; imgSource:"icons/fi-widget.svg"}
    }

    FileDialog{
        id:openDialog
        title:"Load credentials file"
        onAccepted:{
            credM.loadXMLFile(openDialog.fileUrl);
        }
    }

    FileDialog{
        id:saveDialog
        title:"Save credentials file"
        selectExisting : false
        onAccepted:{
            var savefile = saveDialog.fileUrl.toString();
            savefile = savefile.substring(7); //remove file://
            Launcher.writeFile(mainWin.xml,savefile);
        }
    }

    Column{
        anchors.fill: parent
        spacing:2
        Repeater{
            model:menuModel
            Rectangle{
                color: "grey"
                height:60
                width:parent.width

                Image{
                    id:icon
                    source:imgSource
                    //sourceSize.height: 60
                    sourceSize.width: 40
                }

                Text {
                    width:parent.width
                    anchors.left: icon.right
                    anchors.verticalCenter: icon.verticalCenter
                    text: name
                    font.bold: true
                }


                MouseArea{
                    anchors.fill: parent
                    onClicked: menuClick(index);
                }
            }
        }

    }

    function menuClick(idx){
        switch(idx)
        {
            case 0://NEW
                break;
            case 1://READ
                Launcher.launchRun("./hidtool 2 0 512 memtype.xml");
                credM.loadXMLFile();
                break;
            case 2://WRITE
                Launcher.writeFile(mainWin.xml,"memtype.xml");
                Launcher.launchRun("./hidtool 3 memtype.xml");
                break;
            case 3://OPEN
                openDialog.open();
                break;
            case 4://SAVE
                saveDialog.open();
                break;
            case 5://SETTINGS
                break;

        }
    }
}
