using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class HubMsgEvtEntered : IHubMsg
{
    public const string MsgEvent = "userenter";

    public List<UserRecord> users = new List<UserRecord>();

    public override string ExpectedType { get => IHubMsg.TYPEKEY_EVENT; }
    public override string ExpectedEvent { get => MsgEvent; }

    public HubMsgEvtEntered(SimpleJSON.JSONNode jsn, string evt, string type, string status, string error)
        : base(jsn, evt, type, status, error)
    {
        PopulateUserRecords(this.users, jsn, "data", "enter");
    }
}
