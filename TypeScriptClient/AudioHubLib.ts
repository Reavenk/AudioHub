interface AHLConChangeFn {(audioHubLib: AudioHubLib): void;}
interface AHLErrFn {(audioHubLib: AudioHubLib, error: string): void;}
interface AHLMessage {(audioHubLib: AudioHubLib, data: any): void;}
interface AHLAudio {(audioHubLib: AudioHubLib, byteArray: ArrayBuffer ): void;}

type APIMsg = {[id: string] : any};

export class AudioHubLib 
{
    wsclient: WebSocket | null = null;

    fnOnConnected : AHLConChangeFn | null = null;
    fnOnDisconnected : AHLConChangeFn | null = null;
    fnOnError : AHLErrFn | null = null;
    fnOnMessage : AHLMessage | null = null;
    fnOnAudio : AHLAudio | null = null;

    session : string = "";
    username : string = "";

    constructor()
    {
    }

    Connect(serverLoc: string, username: string, session: string)
    {
        this.Disconnect();

        this.username = username;
        this.session = session;

        console.log("Connecting to " + serverLoc);
        this.wsclient = new WebSocket(serverLoc);
        this.wsclient.binaryType = "arraybuffer";
        this.wsclient.onopen = (event) => { this.onws_Open(event); }
        this.wsclient.onclose = (event) => { this.onws_Close(event); }
        this.wsclient.onmessage = (event) => { this.onws_Message(event); }
        this.wsclient.onerror = (event) => { this.onws_Error(event); }
    }

    ToggleMicrophone(toggle: boolean)
    {}

    TogglePlayback(toggle: boolean)
    {}

    Disconnect()
    {
        if (this.wsclient != null)
        {
            console.log("Disconnecting from " + this.wsclient.url);
            this.wsclient.close();
            this.wsclient = null;
        }
    }

    onws_Open(event: Event)
    {
        console.log("Connected to " + this.wsclient!.url);
        if(this.fnOnConnected != null)
            this.fnOnConnected(this);
    }

    onws_Close(event: CloseEvent)
    {
        console.log("Disconnected from " + this.wsclient!.url);
        if (this.fnOnDisconnected != null)
            this.fnOnDisconnected(this);
    }

    onws_Message(event: MessageEvent)
    {
        console.log("Message from " + this.wsclient!.url + ": " + event.data);

        if (event.data instanceof ArrayBuffer)
        {
            //https://developer.mozilla.org/en-US/docs/Web/API/WebSocket/binaryType
            if(this.fnOnAudio != null)
                this.fnOnAudio(this, event.data);
        }
        else
        {
            // Certain messages are reserved for the library and are not passed through the events system.
            let strmsg = event.data as string;
            let msg = JSON.parse(strmsg);
            let eventTy : string = msg["event"];
            let msgType : string = msg["type"];
            let data : any = msg["data"];

            if (msgType == "error")
            {

            }
            else if(msgType == "response")
            {
                if (eventTy == "reqlogin")
                {
                    let msg : APIMsg = this.CreateMsg_Login(this.username, this.session);
                    this.SendJsonMessage(msg);
                }
                else if(eventTy == "login")
                {

                }
            }
            else if(msgType == "event")
            {

            }

            if (this.fnOnMessage != null)
                this.fnOnMessage(this, event.data);
        }
    }

    onws_Error(event: Event)
    {
        console.log("Error from " + this.wsclient!.url + ": " + event);
        if (this.fnOnError != null)
            this.fnOnError(this, "Error");
    }

    CreateMessage(action: string, data: any) :  APIMsg
    {
        let msg :  APIMsg = {
            "action": action,
            "data": data
        };
        return msg;
    }

    CreateMsg_Login(username: string, session: string) : APIMsg
    {
        return this.CreateMessage(
            "login", {
                "username": username,
                "session": session});
    }

    SendJsonMessage(msg: any)
    {
        if (this.wsclient != null)
        {
            let strmsg = JSON.stringify(msg);
            console.log("Sending message to " + this.wsclient.url + ": " + strmsg);
            this.wsclient.send(strmsg);
        }
    }

}