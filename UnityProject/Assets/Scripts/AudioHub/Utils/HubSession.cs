using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class HubSession
{

    private Dictionary<int, HubSessionUser> idToUser = new Dictionary<int, HubSessionUser>();

    public IEnumerable<int> EnumerateIDs()
    { return this.idToUser.Keys; }

    public IEnumerable<HubSessionUser> EnumerateUsers()
    { return this.idToUser.Values; }

    public HubSessionUser this[int id]
    {
        get
        {
            HubSessionUser ret;
            this.idToUser.TryGetValue(id, out ret);
            return ret;
        }
    }

    public HubSessionUser CreateUser(int id, string name)
    {
        if (this.idToUser.ContainsKey(id))
            return null;

        HubSessionUser newUser = new HubSessionUser(id, name);
        this.idToUser.Add(id, newUser);
        return newUser;
    }

    public bool RemoveUser(int id)
    {
        HubSessionUser su = this[id];
        if (su == null)
            return false;

        return this._RemoveUser(su);
    }

    private bool _RemoveUser(HubSessionUser su)
    {
        return this.idToUser.Remove(su.id);
    }

    public void Clear()
    { 
        this.idToUser.Clear();
    }
}