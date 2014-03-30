
function GetPongVersion(onsuccess, onfailure){
    // Get it asynchronously //
    window.cefQuery({request: "PongVersion", persistent: false, 
        onSuccess: onsuccess,
        onFailure: onfailure
        });
}

function ConnectToServer(serverinfo, onsuccess, onfailure){
    window.cefQuery({request: "PongConnect(\""+serverinfo+"\")", persistent: false, 
        onSuccess: onsuccess,
        onFailure: onfailure
        });
}

function Disconnect(){
    window.cefQuery({request: "PongDisconnect", persistent: false, 
        onSuccess: function(){},
        onFailure: function(){}
        });
}
