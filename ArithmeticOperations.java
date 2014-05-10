import java.io.*;
import java.net.*;
import java.lang.*;
class ArithmeticOperations
{
public static void main(String args[]) throws IOException
{
BufferedReader number=new BufferedReader(new InputStreamReader(System.in));
System.out.println("Enter the value for 1st number \n");
int a=Integer.parseInt(number.readLine());
System.out.println("Enter the value for 2nd number \n");
int b=Integer.parseInt(number.readLine());
System.out.println("Type 1 for add \n 2 for subtract\n 3 for multiply \n 4 for divide \n");
int c=Integer.parseInt(number.readLine());
switch(c)
{
case 1:
System.out.println("The value is ="+ (a+b));
break;
case 2:
System.out.println("The value is ="+ (a-b));
break;
case 3:
System.out.println("The value is ="+ (a*b));
break;
case 4:
System.out.println("The value is ="+ (a/b));
break;
default:
System.out.println("error \n");
break;
}
}
}
