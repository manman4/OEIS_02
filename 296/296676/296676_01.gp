M=19;

T(n, k) = my(x='x+O('x^(n+1))); n!*polcoef(atanh(x)^k/k!, n);
\\ a(n) = Sum_{k=0..n} k! * A111594(n,k).
a(n) = sum(k=0, n, k! * T(n, k));
for(n=0, M, print1(a(n),", "));