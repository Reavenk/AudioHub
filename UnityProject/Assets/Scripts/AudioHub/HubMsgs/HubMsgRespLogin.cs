using System.Collections.Generic;

public class HubMsgRespLogin : HubMsgRespSessionEnter
{
    public const string MsgEvent = "login";

    public override string ExpectedEvent { get => MsgEvent; }

    public HubMsgRespLogin(SimpleJSON.JSONNode jsn, string evt, string type, string status, string error)
        : base(jsn, evt, type, status, error)
    {}
}
