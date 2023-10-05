public class HubMsgReqLogin : IHubMsg
{
    public const string MsgEvent = "reqlogin";

    public override string ExpectedType { get => IHubMsg.TYPEKEY_HANDSHAKE; }
    public override string ExpectedEvent { get => MsgEvent; }

    public HubMsgReqLogin(SimpleJSON.JSONNode jsn, string evt, string type, string status, string error)
        : base(jsn, evt, type, status, error)
    {}
}