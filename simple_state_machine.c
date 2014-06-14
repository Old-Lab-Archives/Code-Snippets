/* This is a snippet file... Not a complete source program */
/* Aim: To code a snippet of a "simple state machine implementation" */
/*---------------*/
switch(MeowEvent)
{
case meow1:
{
/* Initialization */
if(current_meow_state==DISCONNECTED)
InitializeMeowProtocol();
current_meow_state=CONNECTED;
}
break;
case meow2:
/* Protocol Messages */
if(current_meow_state==CONNECTED)
{
ProcessMeowMessages();
}
break;
case meow3:
/* Timer Events */
if(current_meow_state==CONNECTED)
{
ProcessMeowTimers();
}
break;
case meow4:
/* Disconnecting events */
if(current_meow_state==CONNECTED)
{
ShutdownMeowProtocol();
current_meow_state=DISCONNECTED;
}
break;
default:
MeowLogError("Error... Meow Meow!");
break;
}
