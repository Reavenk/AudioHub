using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class ClientTester : MonoBehaviour
{
    public AudioHub audioHub;

    public string username = "L. J. Silver";
    public string session = "TestSession";
    public string server = "ws://localhost:8080/hub";

    public string arbitraryMessage = string.Empty;
    public string msgRecipientID = string.Empty;

    public string changeSessionTo = "OtherSession";

    void OnGUI()
    {
        GUILayout.BeginVertical(GUILayout.MinWidth(300.0f));
        switch(this.audioHub.ConState)
        { 
            case AudioHub.ConnectionState.Disconnected:
                this.OnGUI_WhenDisconnected();
                break;

            case AudioHub.ConnectionState.Connecting:
                this.OnGUI_WhenConnecting();
                break;

            case AudioHub.ConnectionState.Connected:
                if(this.audioHub.Authorized)
                    this.OnGUI_WhenConnected_Authed();
                else
                    this.OnGUI_WhenConnected_Unauthed();
                break;
        }
        GUILayout.EndVertical();
    }

    private void OnGUI_WhenDisconnected()
    {
        this.username = GUILayout.TextField(this.username);
        this.session = GUILayout.TextField(this.session);
        this.server = GUILayout.TextField(this.server);

        if (GUILayout.Button("Connect"))
            this.audioHub.Connect(this.username, this.session, this.server);
    }

    private void OnGUI_WhenConnecting()
    {
        GUILayout.Label("Connecting...");
        if (GUILayout.Button("Cancel"))
            this.audioHub.Disconnect();
    }

    private void OnGUI_WhenConnected_Unauthed()
    {
        this.username = GUILayout.TextField(this.username);
        this.session = GUILayout.TextField(this.session);

        if (GUILayout.Button("Login"))
            this.audioHub.Login(this.username, this.session);
    }

    private void OnGUI_WhenConnected_Authed()
    {
        GUILayout.Label(this.username);
        GUILayout.Label(this.session);
        GUILayout.Label(this.server);

        if (GUILayout.Button("Disconnect"))
            this.audioHub.Disconnect();

        if(GUILayout.Button("LogOut"))
            this.audioHub.Logout();

        GUILayout.Label($"Authorized: {this.audioHub.Authorized}");

        if (!this.audioHub.Authorized)
            return;
        
        if (this.audioHub.IsStreamingMic)
        {
            if (GUILayout.Button("Stop Streaming"))
            {
                this.audioHub.StopRecording();
            }

            GUILayout.Label(this.audioHub.MicStreamingDevice);
        }
        else
        {
            for(int i = 0; i < Microphone.devices.Length; ++i)
            {
                if (GUILayout.Button($"Start Streaming {Microphone.devices[i]}"))
                    this.audioHub.StartRecording(i);
            }
        }

        if (this.audioHub.IsDoingPlayback)
        {
            if (GUILayout.Button("Stop Playback"))
                this.audioHub.StopPlayback();
        }
        else
        {
            if (GUILayout.Button("Restart Playback"))
                this.audioHub.RestartPlayback(0.5f);
        }

        GUILayout.Label("Self ID: " + this.audioHub.SelfID.ToString());

        //
        //      LIST USERS
        //
        //////////////////////////////////////////////////
        GUILayout.BeginVertical("box");
        foreach (HubSessionUser hsu in this.audioHub.EnumerateUsers())
        {
            GUILayout.BeginVertical("box", GUILayout.ExpandWidth(true));
            GUILayout.Label("NAME: " + hsu.name);
            GUILayout.Label("ID: " + hsu.id.ToString());
            float modVol = GUILayout.HorizontalSlider(hsu.volume, 0.0f, 1.0f, GUILayout.ExpandWidth(true));
            if (modVol != hsu.volume)
            {
                this.audioHub.SetUserVolume(hsu.id, modVol);
                hsu.volume = modVol;
            }

            bool modGate = GUILayout.Toggle(hsu.gate, "Gate");
            if (modGate != hsu.gate)
            {
                this.audioHub.SetUserGate(hsu.id, modGate);
                hsu.gate = modGate;
            }

            GUILayout.BeginHorizontal(GUILayout.ExpandWidth(true));
                if(GUILayout.Button("Rebuffer"))
                    this.audioHub.RebufferUser(hsu.id);

                if(GUILayout.Button("Fwd Targ"))
                this.msgRecipientID = hsu.id.ToString();
            GUILayout.EndHorizontal();

            GUILayout.EndVertical();
        }
        GUILayout.EndVertical();

        //
        //      BROADCASTING AND FORWARDING
        //
        //////////////////////////////////////////////////
        GUILayout.Label("Messaging");
        this.arbitraryMessage = GUILayout.TextArea(this.arbitraryMessage, GUILayout.Height(50.0f), GUILayout.ExpandWidth(true));
        this.msgRecipientID = GUILayout.TextField(this.msgRecipientID, GUILayout.ExpandWidth(true));
        //
        GUILayout.BeginHorizontal();
            if(GUILayout.Button("Broadcast"))
            {
                this.audioHub.BroadcastHubMessage(this.arbitraryMessage);
            }
            if(GUILayout.Button("Forward"))
            { 
                int fwdId;
                if(int.TryParse(this.msgRecipientID, out fwdId))
                    this.audioHub.ForwardHubMessage(fwdId, this.arbitraryMessage);
                else
                    Debug.Log("Could not parse message recipience to an int representing the user's server ID");
            }
        GUILayout.EndHorizontal();

        //
        //      CHANGE SESSION
        //
        //////////////////////////////////////////////////
        GUILayout.Label("Change Session");
        this.changeSessionTo = GUILayout.TextField(this.changeSessionTo, GUILayout.ExpandWidth(true));
        if(GUILayout.Button("Change"))
            this.audioHub.ChangeSession(this.changeSessionTo);
    }


}
