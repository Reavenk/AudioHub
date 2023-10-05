using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class HubMsgRespGate : IHubMsg
{
    UserRecord user;
    bool audioToggle;

    public const string MsgEvent = "gate";

    public override string ExpectedType { get => IHubMsg.TYPEKEY_RESPONSE; }
    public override string ExpectedEvent { get => MsgEvent; }

    public HubMsgRespGate(SimpleJSON.JSONNode jsn, string evt, string type, string status, string error)
        : base(jsn, evt, type, status, error)
    {}
}
