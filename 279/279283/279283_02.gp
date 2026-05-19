\\ a(n) = Sum_{k=1..n} binomial(k+2,3) * binomial(n+3*k-1,4*k-1).
a(n) = sum(k=1, n, binomial(k+2,3) * binomial(n+3*k-1,4*k-1));
for(n=0, 25, print1(a(n),", "));