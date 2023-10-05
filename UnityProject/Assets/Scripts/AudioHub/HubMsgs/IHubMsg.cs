using System.Collections.Generic;

public abstract class IHubMsg
{
    public const string TYPEKEY_RESPONSE    = "response";
    public const string TYPEKEY_HANDSHAKE   = "handshake";
    public const string TYPEKEY_ERROR       = "error";
    public const string TYPEKEY_EVENT       = "event";

    public const string STATUSKEY_SUCCESS   = "succes";
    public const string STATUSKEY_ERROR     = "error";

    public string evt;
    public string type;
    public string status;
    public string error;

    public virtual string ExpectedStatus {get => "success"; } 
    public abstract string ExpectedType { get; }
    public abstract string ExpectedEvent { get; }

    public static bool Parse(SimpleJSON.JSONNode jsn, out string evt, out string type, out string status, out string error)
    {
        evt = string.Empty;
        type = string.Empty;
        status = string.Empty;
        error = string.Empty;

        SimpleJSON.JSONNode jsEvt = jsn["event"];
        if (jsEvt == null || !jsEvt.IsString)
            return false;

        SimpleJSON.JSONNode jsType = jsn["type"];
        if (jsType == null || !jsType.IsString)
            return false;

        SimpleJSON.JSONNode jsStatus = jsn["status"];
        if (jsStatus == null || !jsStatus.IsString)
            return false;

        evt = jsEvt.Value;
        type = jsType.Value;
        status = jsStatus.Value;

        if (jsn.HasKey("error"))
            error = jsn["error"].Value;

        return true;
    }

    protected IHubMsg(SimpleJSON.JSONNode jsn, string evt, string type, string status, string error)
    {
        this.evt = evt;
        this.type = type;
        this.status = status;
        this.error = error;

        // Runtime network messaging checks
        //
        // To disable a check, have its Expected* property return an empty string.
        string subclassExpType = this.ExpectedType;
        if(!string.IsNullOrEmpty(subclassExpType))
        {
            if(type != subclassExpType)
                throw new System.Exception( $"Mismatched message type for class {this.GetType().Name} and {type}");
        }

        string subclassExpStatus = this.ExpectedStatus;
        if(!string.IsNullOrEmpty(subclassExpStatus))
        {
            if (status != subclassExpStatus)
                throw new System.Exception($"Mismatched message status for class {this.GetType().Name} and {status}");
        }

        string subclassExpEvent = this.ExpectedEvent;
        if(!string.IsNullOrEmpty(subclassExpEvent))
        {
            if (evt != subclassExpEvent)
                throw new System.Exception($"Mismatched message event for class {this.GetType().Name} and {evt}");
        }
    }

    public static SimpleJSON.JSONNode GetObjectInPath(SimpleJSON.JSONNode root, params string[] DAG)
    { 
        SimpleJSON.JSONNode ret = root;
        for(int i = 0; i < DAG.Length; ++i)
        { 
            string key = DAG[i];
            if(ret == null)
                return null;

            SimpleJSON.JSONNode jsn = ret[key];
            if(jsn == null)
                return null;

            if(i == DAG.Length - 1)
                return jsn;

            if(!jsn.IsObject)
                return null;

            ret = jsn.AsObject;
        }
        return ret;
    }

    public static UserRecord? ParseRecord(SimpleJSON.JSONNode jsn)
    { 
        if(jsn == null || !jsn.IsObject)
            return null;

        SimpleJSON.JSONObject jsObj = jsn.AsObject;
        //
        SimpleJSON.JSONNode jsID = jsObj["id"];
        if(jsID == null || !jsID.IsNumber)
            return null;
        SimpleJSON.JSONNode jsName = jsObj["name"];
        if(jsName == null || !jsName.IsString)
            return null;

        return new UserRecord(jsID.AsInt, jsName.Value);
    }

    public static bool PopulateUserRecords(List<UserRecord> users, SimpleJSON.JSONNode root, params string[] DAG)
    { 
        SimpleJSON.JSONNode jsnArrayLoc = GetObjectInPath(root, DAG);
        if(jsnArrayLoc == null)
            return false;

        if(!jsnArrayLoc.IsArray)
            return false;

        SimpleJSON.JSONArray asArray = jsnArrayLoc.AsArray;

        bool addedAny = false;
        foreach(SimpleJSON.JSONNode jsn in asArray)
        { 
            UserRecord? uropt = ParseRecord(jsn);
            if(uropt.HasValue)
            {
                users.Add(uropt.Value);
                addedAny = true;
            }
        }
        return addedAny;
    }
}