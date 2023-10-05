using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class HubMsgEvtDisconnected : IHubMsg
{
    public const string MsgEvent = "disconnect";

    string reason;

    public override string ExpectedType { get => IHubMsg.TYPEKEY_EVENT; }
    public override string ExpectedEvent { get => MsgEvent; }

    public HubMsgEvtDisconnected(SimpleJSON.JSONNode jsn, string evt, string type, string status, string error)
        : base(jsn, evt, type, status, error)
    {
        SimpleJSON.JSONNode jsnReason = GetObjectInPath("data", "reason");
        if(jsnReason == null || !jsnReason.IsString)
            return;

        reason = jsnReason.Value;
    }
}
