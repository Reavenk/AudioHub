using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class HubMsgRespChangeSession : HubMsgRespSessionEnter
{
    public const string MsgEvent = "chsession";

    public override string ExpectedEvent { get => MsgEvent; }

    public HubMsgRespChangeSession(SimpleJSON.JSONNode jsn, string evt, string type, string status, string error)
       : base(jsn, evt, type, status, error)
    {}
}
