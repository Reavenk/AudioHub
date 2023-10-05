using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class HubSessionUser
{
    public readonly int id;
    public readonly string name;
    public float volume = 0.75f;
    public bool gate = true;

    public HubSessionUser(int id, string name)
    {
        this.id = id;
        this.name = name;
    }
}