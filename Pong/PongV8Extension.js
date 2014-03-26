
function GetPongVersion(onsuccess, onfailure){
    // Get it asynchronously //
    window.cefQuery({request: "PongVersion", persistent: false, 
        onSuccess: onsuccess,
        onFailure: onfailure
        });
}

