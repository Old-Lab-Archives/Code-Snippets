#include<iostream>
#include<string>
#include<ctime>
int main()
{
std::string MeowBot[]={"Meow! Meow!","Yo! I'm listening you","Keep talking buddy!","Woo! Interesting","Talk more"};
MeowBotRandom((unsigned) time(NULL));
std::string MeowInput="";
std::string MeowResponse="";
while(1)
{
std::cout << ">";
std::getline(std::cin, MeowInput);
int MeowSelection=MeowRandom() % 5;
MeowResponse=MeowBot[MeowSelection];
std::cout << MeowResponse << std::endl;
}
return 0;
}
