using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class HubMsgRespLogout : IHubMsg
{
    public const string MsgEvent = "logout";

    public override string ExpectedType { get => IHubMsg.TYPEKEY_RESPONSE; }
    public override string ExpectedEvent { get => MsgEvent; }

    public HubMsgRespLogout(SimpleJSON.JSONNode jsn, string evt, string type, string status, string error)
        : base(jsn, evt, type, status, error)
    {}
}
