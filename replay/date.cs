using System;

public class OsuDate
{
    public static void Main(string[] args)
    {
	if (args.Length == 1)
	{
	    long ticks = Convert.ToInt64(args[0]);
	    DateTime myDate = new DateTime(ticks);
	    Console.WriteLine(myDate.ToString("MMMM dd, yyyy hh:mm:ss"));
	}
    }
}
