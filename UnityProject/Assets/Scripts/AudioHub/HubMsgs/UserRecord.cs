public struct UserRecord
{
    public int id;
    public string name;

    public UserRecord(int id, string name)
    { 
        this.id = id;
        this.name = name;
    }

    public static UserRecord Invalid()
    {
        return new UserRecord(-1, "");
    }
}
