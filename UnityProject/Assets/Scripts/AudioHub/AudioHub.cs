using System.Collections.Generic;
using UnityEngine;
using System.Runtime.InteropServices;

public class AudioHub : MonoBehaviour
{
    /// <summary>
    /// Represent the AudioHub's connection state with a server.
    /// </summary>
    public enum ConnectionState 
    { 
        Disconnected,
        Connecting,
        Connected
    }

    /// <summary>
    /// Used for login events to describe the different ways the
    /// AudioHub's login state can change.
    /// </summary>
    public enum LoginNotice
    { 
        Logged,
        ChangeSession,
        Unlogged,
        Failed
    }

    //      SERVER BRIDGE MANAGEMENT
    //
    //////////////////////////////////////////////////
    HubSession clientSideSessionRecord = new HubSession();

    Queue<IHubMsg> pumpMessage = new Queue<IHubMsg>();

    /// <summary>
    /// When connected, what is our registered user ID on the server?
    /// </summary>
    private int selfID = -1;

    /// <summary>
    /// The username we logged in as.
    /// If not connected, this is the username we intend to log in as.
    /// </summary>
    public string username = "L. J. Silver";

    /// <summary>
    /// The session we're logged into.
    /// If not connected, this is the session we intend to log into.
    /// </summary>
    public string session = "TestSession";

    /// <summary>
    /// The server we're connected to.
    /// If not connected, this is the server we're targeting to connect to.
    /// </summary>
    public string server = "ws://localhost:8080/hub";

    /// <summary>
    /// The authorization state of the connection. If true, not only is the
    /// user connected, but is also logged in as a user in a session.
    /// 
    /// Will default to true if not connected.
    /// </summary>
    public bool Authorized {get{ return this.isAuthorized; } }
    private bool isAuthorized = false;

    /// <summary>
    /// The last error encountered on the previous connection. This is 
    /// reset so it only represents the error for the last connection
    /// session (and not the current). If empty, there was no error.
    /// </summary>
    public string LastError {get{return this.lastError; } }
    private string lastError = string.Empty;


    /// <summary>
    /// The WebSocket to connect the AudioHub to the server.
    /// </summary>
    WebSocketSharp.WebSocket websocket = null;

    /// <summary>
    /// The connection state of the AudioHub.
    /// </summary>
    public ConnectionState ConState { get{ return this.connectionState; } }
    ConnectionState connectionState = ConnectionState.Disconnected;

    //      MICROPHONE STREAMING MEMBERS
    // 
    //////////////////////////////////////////////////

    ClipStreamUtil micStreamer = null;

    public bool IsStreamingMic {get{ return this.micStreamer != null; } }
    //
    public string MicStreamingDevice 
    {
        get
        {
            if(this.micStreamer == null)
                return string.Empty;

            return this.micStreamer.DeviceName;
        }
    }

    //      STREAMING PLAYBACK MEMBERS
    // 
    //////////////////////////////////////////////////
    
    public AudioSource playbackAudioSource;
    private AudioClip playbackClip;

    private Queue<short[]> playbackSamples = new Queue<short[]>();
    private int playbackPos = 0;

    public bool IsDoingPlayback { get{ return this.saveForPlayback; } }
    bool saveForPlayback = false;

    // Web streaming
#if UNITY_WEBGL
    private float [] pcmOutStageToBrowser = null;

    [DllImport("__Internal")]
    public static extern int AH_ReportPlaybackFloatSamples(float [] idx);

    [DllImport("__Internal")]
    public static extern void AH_PlaybackStart();

    [DllImport("__Internal")]
    public static extern void AH_PlaybackStop();

    [DllImport("__Internal")]
    public static extern void AH_PlaybackInitialize();

    [DllImport("__Internal")]
    public static extern void AH_PlaybackShutdown();

    public void AudioHub_FillBuffer(int samples)
    { 
        this.pcmOutStageToBrowser = new float[samples];
        //
        // We can elegantly repurpose the FMOD streaming function to 
        // a browser streaming function.
        this.PCMPlaybackCallback(this.pcmOutStageToBrowser);
        AH_ReportPlaybackFloatSamples(this.pcmOutStageToBrowser);
    }

    public void AudioHub_ClearBuffer()
    { 
        this.pcmOutStageToBrowser = null;
    }
#endif

    //      EVENTS
    //
    //////////////////////////////////////////////////
    public System.Action<AudioHub, LoginNotice> onLogin;
    public System.Action<AudioHub, HubMsgEvtBroadcast> onBroadcastMsg;
    public System.Action<AudioHub, ConnectionState> onConnectionChange;

    public int SelfID {get{ return this.selfID; } }

    //////////////////////////////////////////////////
    
    void Start()
    {
        if(this.playbackAudioSource == null)
            this.playbackAudioSource = this.gameObject.AddComponent<AudioSource>();
    }

    // Update is called once per frame
    void Update()
    {
        Queue<IHubMsg> messages = null;
        lock(this.pumpMessage)
        { 
            messages = this.pumpMessage;
            this.pumpMessage = new Queue<IHubMsg>();
        }

        while(messages.Count > 0)
        { 
            IHubMsg msg = messages.Dequeue();
            switch(msg.evt)
            { 
                case HubMsgReqLogin.MsgEvent:
                    this.SendLogin(this.username, this.session);
                    break;

                case HubMsgRespChangeSession.MsgEvent:
                case HubMsgRespLogin.MsgEvent:
                    {
                        this.isAuthorized = true;
                        HubMsgRespSessionEnter msgEnterSession = msg as HubMsgRespSessionEnter;
                        this.selfID = msgEnterSession.selfID;
                        this.session = msgEnterSession.session;
                        this.clientSideSessionRecord.Clear();
                        foreach (UserRecord ur in msgEnterSession.users)
                        {
                            if(ur.id == this.selfID)
                                continue;

                            this.clientSideSessionRecord.CreateUser(ur.id, ur.name);
                        }

                        LoginNotice noticeTy = 
                            (msg.evt == HubMsgRespLogin.MsgEvent) ?
                                LoginNotice.Logged :
                                LoginNotice.ChangeSession;
                        //
                        onLogin?.Invoke(this, noticeTy);
                    }
                    break;

                case HubMsgRespLogout.MsgEvent:
                    this.isAuthorized = false;
                    break;

                case HubMsgEvtEntered.MsgEvent:
                    {
                        HubMsgEvtEntered msgUserEntered = msg as HubMsgEvtEntered;
                        foreach(UserRecord ur in msgUserEntered.users)
                            this.clientSideSessionRecord.CreateUser(ur.id, ur.name);
                    }
                    break;

                case HubMsgEvtUserLeft.MsgEvent:
                    {
                        HubMsgEvtUserLeft msgUserLeft = msg as HubMsgEvtUserLeft;
                        foreach(UserRecord ur in msgUserLeft.users)
                            this.clientSideSessionRecord.RemoveUser(ur.id);
                    }
                    break;

            }
        }

        if(this.connectionState == ConnectionState.Connected)
        {
            if(this.micStreamer != null)
            { 
                short[] pcm = this.micStreamer.GetNewPCM();
                
                if(pcm != null && pcm.Length > 0)
                { 
                    // NOTE, We may want to defer the conversion until the PCM handler,
                    // we would get a small bit of multi-threading for free.
                    int byteCt = sizeof(short) * pcm.Length;
                    byte [] bytePCM = new byte[byteCt];
                    System.Buffer.BlockCopy(pcm, 0, bytePCM, 0, byteCt);

                    this.SendPCMBytes(bytePCM);
                }
            }
        }
    }
    float delme = 0.0f;

    public void SendLogin(string username, string session)
    { 
        SimpleJSON.JSONObject jsoRet = new SimpleJSON.JSONObject();
        jsoRet["action"] = HubMsgRespLogin.MsgEvent;
        jsoRet["data"] = new SimpleJSON.JSONObject();
        jsoRet["data"]["username"] = username;
        jsoRet["data"]["session"] = session;
        this.websocket.Send(jsoRet.ToString());
    }

    public void SendLogout()
    {
        SimpleJSON.JSONObject jsoRet = new SimpleJSON.JSONObject();
        jsoRet["action"] = HubMsgRespLogout.MsgEvent;
        jsoRet["data"] = new SimpleJSON.JSONObject();
        this.websocket.Send(jsoRet.ToString());
    }

    protected void SendChangeSession(string newSessionName)
    {
        SimpleJSON.JSONObject jsoRet = new SimpleJSON.JSONObject();
        jsoRet["action"] = HubMsgRespChangeSession.MsgEvent;
        jsoRet["data"] = new SimpleJSON.JSONObject();
        jsoRet["data"]["session"] = newSessionName;
        this.websocket.Send(jsoRet.ToString());
    }

    protected void SendGate(int userId, bool allowAudio)
    {
        SimpleJSON.JSONObject jsoRet = new SimpleJSON.JSONObject();
        jsoRet["action"] = HubMsgRespGate.MsgEvent;
        jsoRet["data"] = new SimpleJSON.JSONObject();
        jsoRet["data"]["userid"] = userId;
        jsoRet["data"]["gate"] = allowAudio;
        this.websocket.Send(jsoRet.ToString());
    }

    protected void SendChangeVolume(int userId, float newVolume)
    {
        SimpleJSON.JSONObject jsoRet = new SimpleJSON.JSONObject();
        jsoRet["action"] = HubMsgRespVolume.MsgEvent;
        jsoRet["data"] = new SimpleJSON.JSONObject();
        jsoRet["data"]["userid"] = userId;
        jsoRet["data"]["volume"] = newVolume;
        this.websocket.Send(jsoRet.ToString());
    }

    protected void SendBroadcastMessage(string message)
    {
        SimpleJSON.JSONObject jsoRet = new SimpleJSON.JSONObject();
        jsoRet["action"] = "broadcast";
        jsoRet["data"] = new SimpleJSON.JSONObject();
        jsoRet["data"]["message"] = message;
        this.websocket.Send(jsoRet.ToString());
    }

    protected void SendForwardMessage(int userid, string message)
    {
        SimpleJSON.JSONObject jsoRet = new SimpleJSON.JSONObject();
        jsoRet["action"] = "forward";
        jsoRet["data"] = new SimpleJSON.JSONObject();
        jsoRet["data"]["message"] = message;
        jsoRet["data"]["recipid"] = userid;
        this.websocket.Send(jsoRet.ToString());
    }

    protected void SendRebuffer(IEnumerable<int> userids, int samples = ClipStreamUtil.bufferSize/2)
    { 
        SimpleJSON.JSONObject jsoRet = new SimpleJSON.JSONObject();
        jsoRet["action"] = "rebuffer";
        jsoRet["data"] = new SimpleJSON.JSONObject();
        jsoRet["data"]["samples"] = samples;

        SimpleJSON.JSONArray jsaUsers = new SimpleJSON.JSONArray();
        if(userids != null)
        { 
            foreach(int uid in userids)
                jsaUsers.Add(uid);
        }
        jsoRet["data"]["users"] = jsaUsers;
        this.websocket.Send(jsoRet.ToString());
    }

    void SendPCMBytes(byte [] pcmBytes)
    {
        Debug.Log($"{pcmBytes.Length} bytes");
        this.websocket.Send(pcmBytes);
    }

    public void Login(string username, string session)
    { 
        this.SendLogin(this.username, this.session);
    }

    public void Login()
    { 
        this.Login(this.username, this.session);
    }

    public bool Logout()
    { 
        if(this.connectionState == ConnectionState.Disconnected)
            return false;

        this.SendLogout();
        return true;

    }

    public bool SetUserGate(int userid, bool allowAudio)
    {
        if(this.clientSideSessionRecord[userid] == null)
            return false;

        this.SendGate(userid, allowAudio);
        return true;
    }

    public bool SetUserVolume(int userid, float volume)
    {
        if (this.clientSideSessionRecord[userid] == null)
            return false;

        this.SendChangeVolume(userid, volume);
        return true;
    }

    public void ChangeSession(string newSessionName)
    { 
        this.SendChangeSession(newSessionName);
    }

    public float GetUserVolume(int id)
    {
        HubSessionUser hsu = this.clientSideSessionRecord[id];
        if (hsu == null)
            return float.NaN;

        return hsu.volume;
    }

    public void Connect(string username, string session, string server)
    {
        this.Disconnect();

        this.username = username;
        this.session = session;
        this.server = server;

        this.lastError = string.Empty;

        this.websocket = new WebSocketSharp.WebSocket(this.server);
        this.websocket.OnOpen       += this.OnWS_Open;
        this.websocket.OnMessage    += this.OnWS_Message;
        this.websocket.OnClose      += this.OnWS_Close;
        this.websocket.OnError      += this.OnWS_Error;
        this.websocket.Connect();
    }

    public void Disconnect()
    { 
        if(this.websocket == null)
            return;

        this.StopRecording();
        this.StopPlayback();

        this.websocket.Close();
        this.websocket              = null;
        this.connectionState        = ConnectionState.Disconnected;
        this.isAuthorized           = false;
        this.pumpMessage.Clear();
        this.onLogin?.Invoke(this, LoginNotice.Unlogged);

        this.playbackPos = 0;
        this.playbackSamples.Clear();

        this.clientSideSessionRecord.Clear();

        this.selfID = -1;
    }

    public bool StartRecording(int device)
    { 
        if(this.connectionState != ConnectionState.Connected || !this.Authorized)
            return false;

        this.StopRecording();
        this.micStreamer = new ClipStreamUtil();
        this.micStreamer.Start(device);
        return true;
    }

    public bool StopRecording()
    { 
        if(this.micStreamer != null)
        { 
            this.micStreamer.Stop();
            this.micStreamer = null;
            return true;
        }
        return false;
    }

    public void RestartPlayback(float bufferInSec)
    {
        this.StopPlayback();
        lock (this.playbackSamples)
        {
            this.playbackPos = 0;

            int bufferSampleCt = (int)(bufferInSec * ClipStreamUtil.sampleRate);
            short[] buffer = new short[bufferSampleCt];
            System.Array.Clear(buffer, 0, bufferSampleCt);
            this.playbackSamples.Enqueue(buffer);
        }

        if(this.playbackClip == null)
        {
            this.playbackClip =
                AudioClip.Create(
                    "PlaybackClip",
                    ClipStreamUtil.sampleRate,
                    1,
                    ClipStreamUtil.sampleRate,
                    true,
                    this.PCMPlaybackCallback);

            this.playbackAudioSource.clip = this.playbackClip;
        }

        this.saveForPlayback = true;
        this.playbackAudioSource.loop = true;
        this.playbackAudioSource.Play();
    }

    public void StopPlayback()
    {
        this.saveForPlayback = false;
        lock(this.playbackSamples)
        { 
            this.playbackSamples.Clear();
            this.playbackPos = 0;
        }
        this.playbackAudioSource.Stop();
    }

    public bool BroadcastHubMessage(string message)
    { 
        if(this.connectionState != ConnectionState.Connected)
            return false;

        this.SendBroadcastMessage(message);

        return true;
    }

    public bool ForwardHubMessage(int userid, string message)
    { 
        if(this.connectionState != ConnectionState.Connected)
            return false;

        this.SendForwardMessage(userid, message);

        return true;
    }

    public bool RebufferAllUsers(int samples = ClipStreamUtil.sampleRate / 2)
    {
        if (this.connectionState != ConnectionState.Connected)
            return false;

        this.SendRebuffer(null, samples);
        return true;
    }

    public bool RebufferUser(int user, int samples = ClipStreamUtil.sampleRate / 2)
    {
        if (this.connectionState != ConnectionState.Connected)
            return false;

        this.SendRebuffer(new int[]{ user }, samples);
        return true;
    }

    /// <summary>
    /// Websocket message for when a connection with the server is sucessful.
    /// </summary>
    void OnWS_Open(object sender, System.EventArgs e)
    { 
        this.connectionState = ConnectionState.Connected;
    }
    
    /// <summary>
    /// Websocket message for when a connection from the server is received.
    /// </summary>
    void OnWS_Message(object sender, WebSocketSharp.MessageEventArgs e)
    { 
        if(e.IsBinary)
        {
            lock(this.playbackSamples)
            { 
                // Convert byte data back to a PCM array of shorts
                short[] pcmShort = new short[e.RawData.Length / sizeof(short)];
                System.Buffer.BlockCopy(e.RawData, 0, pcmShort, 0, e.RawData.Length);
                this.playbackSamples.Enqueue(pcmShort);
            }
        }
        else
        {
            // Web payload
            Debug.Log("Got message " + e.Data);
            SimpleJSON.JSONNode jsn = SimpleJSON.JSON.Parse(e.Data);
            if(jsn == null)
                return;

            IHubMsg parsedMsg = null;
            string evt;
            string type;
            string status;
            string error;
            if(!IHubMsg.Parse(jsn, out evt, out type, out status, out error))
                return;

            try
            {
                if (type == IHubMsg.TYPEKEY_ERROR)
                {
                    parsedMsg = new HubMsgError(jsn, evt, type, status, error);
                }
                else if(type == IHubMsg.TYPEKEY_HANDSHAKE)
                {
                    if (evt == HubMsgReqLogin.MsgEvent)
                        parsedMsg = new HubMsgReqLogin(jsn, evt, type, status, error);
                }
                else if(type == IHubMsg.TYPEKEY_RESPONSE)
                { 
                    if(evt == HubMsgRespLogin.MsgEvent)
                        parsedMsg = new HubMsgRespLogin(jsn, evt, type, status, error);
                    else if(evt == HubMsgRespLogout.MsgEvent)
                        parsedMsg = new HubMsgRespLogout(jsn, evt, type, status, error);
                    else if(evt == HubMsgRespVolume.MsgEvent)
                        parsedMsg = new HubMsgRespVolume(jsn, evt, type, status, error);
                    else if(evt == HubMsgRespGate.MsgEvent)
                        parsedMsg = new HubMsgRespGate(jsn, evt, type, status, error);
                    else if(evt == HubMsgRespChangeSession.MsgEvent)
                        parsedMsg = new HubMsgRespChangeSession(jsn, evt, type, status, error);
                            
                }
                else if(type == IHubMsg.TYPEKEY_EVENT)
                { 
                    if(evt == HubMsgEvtEntered.MsgEvent)
                        parsedMsg = new HubMsgEvtEntered(jsn, evt, type, status, error);
                    else if(evt == HubMsgEvtUserLeft.MsgEvent)
                        parsedMsg = new HubMsgEvtUserLeft(jsn, evt, type, status, error);
                    else if(evt == HubMsgEvtBroadcast.MsgEvent)
                        parsedMsg = new HubMsgEvtBroadcast(jsn, evt, type, status, error);
                    else if(evt == HubMsgEvtForward.MsgEvent)
                        parsedMsg = new HubMsgEvtForward(jsn, evt, type, status, error);
                }
            }
            catch(System.Exception /*ex*/)
            { }

            if(parsedMsg != null)
            { 
                lock(this.pumpMessage)
                { 
                    this.pumpMessage.Enqueue(parsedMsg);
                }
            }
        }
    }

    public IEnumerable<HubSessionUser> EnumerateUsers()
    { 
        return this.clientSideSessionRecord.EnumerateUsers();
    }

    /// <summary>
    /// Websocket callback for when an error occurs for the connection.
    /// A seperate Close message is expected afterwards.
    /// </summary>
    void OnWS_Error(object sender, WebSocketSharp.ErrorEventArgs e)
    {
        this.lastError = e.Message;
    }

    /// <summary>
    /// Websocket callback for when the connection is closed.
    /// </summary>
    void OnWS_Close(object sender, WebSocketSharp.CloseEventArgs e)
    {
        this.Disconnect();
    }

    /// <summary>
    /// The AudioClip callback to stream playback audio.
    /// </summary>
    public void PCMPlaybackCallback(float[] data)
    {
        int writePos = 0;
        lock(this.playbackSamples)
        {
            while (writePos < data.Length && this.playbackSamples.Count > 0)
            {
                short[] rs = this.playbackSamples.Peek();
                int samplesTransferred = Mathf.Min(data.Length - writePos, rs.Length - this.playbackPos);
                for(int i = 0; i < samplesTransferred; ++i)
                { 
                    data[writePos + i] = (float)rs[this.playbackPos + i] / (float)short.MaxValue;
                }
                this.playbackPos += samplesTransferred;
                writePos += samplesTransferred;

                if (this.playbackPos >= rs.Length)
                {
                    this.playbackSamples.Dequeue();
                    this.playbackPos = 0;
                }
            }
        }

        for(int i = writePos; i < data.Length; ++i)
            data[i] = 0.0f;
    }

    private void OnDestroy()
    {
        this.Disconnect();
    }

    
}
