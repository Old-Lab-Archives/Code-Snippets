using System;
using System.Net;
class MeowCheckDNS
{
static void Main(string[] args)
{
IPHostEntry Meow = Dns.GetHostByName(args[0]);
IPAddress[] MeowAddr = Meow.AddressList;
foreach(IPAddress MeowAddress in MeowAddr)
Console.WriteLine(MeowAddress);
}
}
