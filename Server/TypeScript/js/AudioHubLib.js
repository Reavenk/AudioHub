export class AudioHubLib {
    constructor() {
        this.wsclient = null;
        this.fnOnConnected = null;
        this.fnOnDisconnected = null;
        this.fnOnError = null;
        this.fnOnMessage = null;
        this.fnOnAudio = null;
        this.session = "";
        this.username = "";
    }
    Connect(serverLoc, username, session) {
        this.Disconnect();
        this.username = username;
        this.session = session;
        console.log("Connecting to " + serverLoc);
        this.wsclient = new WebSocket(serverLoc);
        this.wsclient.binaryType = "arraybuffer";
        this.wsclient.onopen = (event) => { this.onws_Open(event); };
        this.wsclient.onclose = (event) => { this.onws_Close(event); };
        this.wsclient.onmessage = (event) => { this.onws_Message(event); };
        this.wsclient.onerror = (event) => { this.onws_Error(event); };
    }
    Disconnect() {
        if (this.wsclient != null) {
            console.log("Disconnecting from " + this.wsclient.url);
            this.wsclient.close();
            this.wsclient = null;
        }
    }
    onws_Open(event) {
        console.log("Connected to " + this.wsclient.url);
        if (this.fnOnConnected != null)
            this.fnOnConnected(this);
    }
    onws_Close(event) {
        console.log("Disconnected from " + this.wsclient.url);
        if (this.fnOnDisconnected != null)
            this.fnOnDisconnected(this);
    }
    onws_Message(event) {
        console.log("Message from " + this.wsclient.url + ": " + event.data);
        if (event.data instanceof ArrayBuffer) {
            //https://developer.mozilla.org/en-US/docs/Web/API/WebSocket/binaryType
            if (this.fnOnAudio != null)
                this.fnOnAudio(this, event.data);
        }
        else {
            // Certain messages are reserved for the library and are not passed through the events system.
            let strmsg = event.data;
            let msg = JSON.parse(strmsg);
            let eventTy = msg["event"];
            let msgType = msg["type"];
            let data = msg["data"];
            if (msgType == "error") {
            }
            else if (msgType == "response") {
                if (eventTy == "login") {
                    let msg = this.CreateMsg_Login(this.username, this.session);
                    this.SendJsonMessage(msg);
                }
            }
            else if (msgType == "event") {
            }
            if (this.fnOnMessage != null)
                this.fnOnMessage(this, event.data);
        }
    }
    onws_Error(event) {
        console.log("Error from " + this.wsclient.url + ": " + event);
        if (this.fnOnError != null)
            this.fnOnError(this, "Error");
    }
    CreateMessage(action, data) {
        let msg = {
            "action": action,
            "data": data
        };
        return msg;
    }
    CreateMsg_Login(username, session) {
        return this.CreateMessage("login", {
            "username": username,
            "session": session
        });
    }
    SendJsonMessage(msg) {
        if (this.wsclient != null) {
            let strmsg = JSON.stringify(msg);
            console.log("Sending message to " + this.wsclient.url + ": " + strmsg);
            this.wsclient.send(strmsg);
        }
    }
}
//# sourceMappingURL=AudioHubLib.js.map