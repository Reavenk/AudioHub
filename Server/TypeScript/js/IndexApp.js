import { AudioHubLib } from "./AudioHubLib.js";
export class IndexApp {
    constructor() {
        this.audioHub = new AudioHubLib();
        this.audioHub.fnOnConnected = this.onConnect.bind(this);
        this.audioHub.fnOnDisconnected = this.onDisconnect.bind(this);
        this.audioHub.fnOnMessage = this.onMessage.bind(this);
        let connectButton = document.getElementById("connect");
        connectButton.onclick = (event) => {
            let usernameInput = document.getElementById("username");
            let sessionInput = document.getElementById("session");
            this.audioHub.Connect("ws://localhost:8080/hub", usernameInput.value, sessionInput.value);
        };
        let divLogin = document.getElementById("login");
        let divConnecting = document.getElementById("connecting");
        let divConnected = document.getElementById("connected");
        divConnecting.style.display = "none";
        divConnected.style.display = "none";
    }
    onConnect(audioHub) {
        console.log("Connected to " + audioHub.wsclient.url);
    }
    onDisconnect(audioHub) {
        console.log("Disconnected from " + audioHub.wsclient.url);
    }
    onMessage(audioHub, message) {
        console.log("Message from " + audioHub.wsclient.url + ": " + message);
    }
}
//# sourceMappingURL=IndexApp.js.map