M=13;

\\ a(n) = (4*n)! * [x^(4*n)] (-log(1 - x))^n / n!. 
a(n) = my(x='x+O('x^(4*n+1))); (4*n)! * polcoef( (-log(1 - x))^n / n!, 4*n);
for(n=0, M, print1(a(n),", "));
