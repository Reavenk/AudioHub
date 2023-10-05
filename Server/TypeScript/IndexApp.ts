import { AudioHubLib } from "./AudioHubLib.js";

export class IndexApp
{
    audioHub : AudioHubLib = new AudioHubLib();
    constructor()
    {
        this.audioHub.fnOnConnected = this.onConnect.bind(this);
        this.audioHub.fnOnDisconnected = this.onDisconnect.bind(this);
        this.audioHub.fnOnMessage = this.onMessage.bind(this);

        
        let connectButton : HTMLButtonElement = document.getElementById("connect") as HTMLButtonElement;

        connectButton.onclick = (event) => { 
            let usernameInput : HTMLInputElement = document.getElementById("username") as HTMLInputElement;
            let sessionInput : HTMLInputElement = document.getElementById("session") as HTMLInputElement;

            this.audioHub.Connect("ws://localhost:8080/hub", usernameInput.value, sessionInput.value); 
        };

        let divLogin : HTMLDivElement = document.getElementById("login") as HTMLDivElement;
        let divConnecting : HTMLDivElement = document.getElementById("connecting") as HTMLDivElement;
        let divConnected : HTMLDivElement = document.getElementById("connected") as HTMLDivElement;

        divConnecting.style.display = "none";
        divConnected.style.display = "none";
    }

    onConnect(audioHub: AudioHubLib)
    {
        console.log("Connected to " + audioHub.wsclient!.url);
    }

    onDisconnect(audioHub: AudioHubLib)
    {
        console.log("Disconnected from " + audioHub.wsclient!.url);
    }

    onMessage(audioHub: AudioHubLib, message: string)
    {
        console.log("Message from " + audioHub.wsclient!.url + ": " + message);
    }
}
