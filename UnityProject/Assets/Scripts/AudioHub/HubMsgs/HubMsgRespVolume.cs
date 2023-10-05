using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class HubMsgRespVolume : IHubMsg
{
    public const string MsgEvent = "volume";

    UserRecord user = UserRecord.Invalid();
    float volume = 0.0f;

    public override string ExpectedType { get => IHubMsg.TYPEKEY_RESPONSE; }
    public override string ExpectedEvent { get => MsgEvent; }

    public HubMsgRespVolume(SimpleJSON.JSONNode jsn, string evt, string type, string status, string error)
        : base(jsn, evt, type, status, error)
    {
        SimpleJSON.JSONNode jsnVol = GetObjectInPath(jsn, "data", "volume");
        if(jsnVol != null && jsnVol.IsNumber)
            this.volume = Mathf.Clamp01(jsnVol.AsFloat);


        SimpleJSON.JSONNode jsnUser = GetObjectInPath(jsn, "data", "user");
        if(jsnUser != null)
        { 
            UserRecord? uropt = ParseRecord(jsnUser);
            if(uropt.HasValue)
                user = uropt.Value;
        }
    }
}
