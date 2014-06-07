using System;
using System.Threading;
class MeowThread
{
static void Main()
{
Meow m = new Thread(new MeowThreadBegin(MeowGo));
m.Start();
MeowGo();
}
static void MeowGo()
{
for(char c='a';c<='z';c++)
Console.Write(c);
}
}
