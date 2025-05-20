M=13;

\\ a(n) = (3*n)! * [x^(3*n)] (-log(1 - x))^n / n!.
a(n) = my(x='x+O('x^(3*n+1))); (3*n)! * polcoef( (-log(1 - x))^n / n!, 3*n);    
for(n=0, M, print1(a(n),", "));
