## QUESTION

# https://plus.google.com/+TerenceTao27/posts/CR1ZoNe9ojQ

## SOLUTION

# Price before lunch: x+y
# Price after lunch: y
# 26 chicken farmer sold a before lunch, 26-a after lunch
# 16 chicken farmer sold a+b before lunch, 16-a-b after lunch (so b >= 0)
# 10 chicken farmer sold a+c before lunch, 10-a-c (so c >= b)
 
# Have some obvious inequalities:
# 0 <= a <= 26
# 0 <= a+b <= 16
# 0 <= a+c <= 10
 
# and, since a,b,c all >= 0:
# 0 <= a <= 10
# 0 <= b <= 16
# 0 <= c <= 10
 
# Problem conditions are:
# a(x+y) + (26-a)y = 35
# (a+b)(x+y) + (16-a-b)y = 35
# (a+c)(x+y) + (10-a-c)y = 35
 
# Multiply out:
# ax + 26y = 35
# ax + ay + bx + by + 16y - ay - by = 35
# ax + ay + cx + cy + 10y - ay - cy = 35
 
# Simplify:
# A: ax + 26y = 35
# B: ax + bx + 16y = 35
# C: ax + cx + 10y = 35
 
# These give:
# D: bx = 10y (from B-A)
# E: cx = 16y (from C-A)
 
# Case i: y == 0:
# ax = 35 and b = c = 0
# Solutions for a = 1..10, with x = 35/x
 
# Case ii: y != 0, (so b,c and x != 0) so can divide D by E:
# b/c = 10/16 = 5/8 => b = 5, c = 8 (b = 10, c = 16 out of bounds)
# and from D:
# 5x = 10y => x = 2y
 
# Substituting and rearranging A:
# y = 35/(2a+26)
# and a+c <= 10 so have solutions for a = 0,1,2
 
from fractions import Fraction;
 
def check(x,y,a,b,c):
    assert(a*(x+y) + ((26-a)*y) == 35)
    assert((a+b)*(x+y) + ((16-a-b)*y) == 35)
    assert((a+c)*(x+y) + ((10-a-c)*y) == 35)
    print(x+y,y,a,a+b,a+c)
     
for a in range(1,11): 
    check(Fraction(35,a),0,a,0,0)
 
for a in range(0,3): 
    y = Fraction(35,2*a+26);
    x = 2*y
    b = 5
    c = 8
    check(x,y,a,b,c)
