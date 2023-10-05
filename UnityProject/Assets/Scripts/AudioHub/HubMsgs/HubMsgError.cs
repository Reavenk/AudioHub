using System.Collections;
using System.Collections.Generic;
using UnityEngine;

class HubMsgError : IHubMsg
{
    string actualEvent;

    public override string ExpectedStatus { get => IHubMsg.STATUSKEY_ERROR; }
    public override string ExpectedType { get => IHubMsg.TYPEKEY_ERROR;}
    public override string ExpectedEvent { get => "error"; }

    public HubMsgError(SimpleJSON.JSONNode jsn, string evt, string type, string status, string error)
        : base(jsn, error, type, status, error)
    {
        this.actualEvent = evt;
    }
}