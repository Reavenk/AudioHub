using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class HubMsgEvtBroadcast : IHubMsg
{
    public const string MsgEvent = "broadcast";

    int senderId = -1;
    SimpleJSON.JSONNode jsonNode;

    public override string ExpectedType { get => IHubMsg.TYPEKEY_EVENT; }
    public override string ExpectedEvent { get => MsgEvent; }

    public HubMsgEvtBroadcast(SimpleJSON.JSONNode jsn, string evt, string type, string status, string error)
        : base(jsn, evt, type, status, error)
    {
        this.jsonNode = jsn;

        SimpleJSON.JSONNode jsnSender = GetObjectInPath(jsn, "data", "senderid");
        if(jsnSender != null && jsnSender.IsNumber)
            senderId = jsnSender.AsInt;
    }

}
