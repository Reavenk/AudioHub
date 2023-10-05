using System.Collections.Generic;

/// <summary>
/// Shared class between HubMsgRespLogin and HubMsgRespChangeSession, since
/// they're pretty much the same thing. This makes it possible to combine
/// their message handlers.
/// </summary>
public abstract class HubMsgRespSessionEnter : IHubMsg
{
    public List<UserRecord> users = new List<UserRecord>();
    public string session = string.Empty;
    public int selfID = -1;

    public override string ExpectedType { get => IHubMsg.TYPEKEY_RESPONSE; }

    public HubMsgRespSessionEnter(SimpleJSON.JSONNode jsn, string evt, string type, string status, string error)
        : base(jsn, evt, type, status, error)
    {
        PopulateUserRecords(this.users, jsn, "data", "users");

        SimpleJSON.JSONNode jsnSession = GetObjectInPath(jsn, "data", "session");
        if (jsnSession != null && jsnSession.IsString)
            session = jsnSession.Value;

        SimpleJSON.JSONNode jsnSelfId = GetObjectInPath(jsn, "data", "selfid");
        if (jsnSelfId != null && jsnSelfId.IsNumber)
            selfID = jsnSelfId.AsInt;
    }
}
