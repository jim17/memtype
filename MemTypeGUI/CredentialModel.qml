import QtQuick 2.3
import QtQuick.XmlListModel 2.0

Item {
    id:root
    anchors.fill: parent
    property string modelSource

    onModelSourceChanged:loadXMLFile();

    XmlListModel{
        id: crdModel

        //Credential Items
        query: "/mt/cred"
        xml:mainWin.xml

        XmlRole{ name:"name"; query:"name/string()"}
        XmlRole{ name:"user"; query:"user/string()"}
        XmlRole{ name:"hop"; query:"hop/string()"}
        XmlRole{ name:"pass"; query:"pass/string()"}
        XmlRole{ name:"submit"; query:"submit/string()"}

        onSourceChanged: reload();

    }

    ListView{
        id: crdList
        anchors.fill: parent
        model:crdModel
        delegate:crdDelegate
        spacing: 15
        clip:true
    }

    Component{
        id:crdDelegate
        Credential{
            crdName:name
            crdUser:user
            crdHop:hop
            crdPass:pass
            crdSubmit:submit
            crdIdx:index
        }
    }

    function loadXMLFile(file){

        //In case there's something already on there
        mainWin.xml = ""

        var xhr = new XMLHttpRequest;
        var pwd = "";
        if (Qt.platform.os === "linux" || Qt.platform.os === "osx" || Qt.platform.os === "unix") {
            pwd = Launcher.launchRun("pwd");
            pwd = pwd.split('\n')[0];
        }else if (Qt.platform.os === "windows"){
            pwd = Launcher.launchRun("echo %cd%");
        }

        var filepath = "file://"+pwd+"/memtype.xml";
        if(file){
          xhr.open("GET", file); //PASSED FILE
        }else{
         xhr.open("GET", filepath); //DEFAULT FILE
        }


        xhr.onreadystatechange = function() {

            if (xhr.readyState == XMLHttpRequest.DONE) {

                mainWin.xml = processInXML(xhr.responseText);
            }
        }
        xhr.send();
    }

    function processInXML(text)
    {
        console.log(text);
        var atext="";
        atext = text.replace(/\t/g,'\\t');
        //atext = atext.replace(/\n/g,'\\n');
        atext = atext.replace(/\x16/g,'\\x16');
        console.log(atext);
        return atext;
    }

 /*   function processOutXML(text)
    {
        var atext="";
        atext = text.replace(/\t/g,'\\t');
        //atext = atext.replace(/\n/g,'\\n');
        atext = atext.replace(/\x16/g,'\\x16');
        return atext;
    }*/

}


