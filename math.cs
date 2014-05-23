using System;
public class math {
    public static int Puzzle(int x) {
        if(x==0) return 0;
		if(x==1) return 0;
		if(x%3==0) {
		x = (x-((x/3)*2));
		return x;
		} if(x%3==1) {
			x =(x-1)-((x/3)*2);
			return x;
		} if(x%3==2) {
			x =(x-2)-((x/3)*2);
			return x;
		} if((-(-x))%3==0) {
		x = ((-(-x))-(((-(-x))/3)*2));
		return x;
		} if((-(-x))%3==(-1)) {
			x =((-(-x))-(-1))-(((-(-x))/3)*2);
			return x;
		} else {
			x =((-(-x))-(-2))-(((-(-x))/3)*2);
			return x;
		}}}
