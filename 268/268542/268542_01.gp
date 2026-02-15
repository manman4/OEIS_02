\\ a(n) = Sum_{k=0..n} binomial(n,k) * binomial(n+k,k) * binomial(2*k,n).
a(n) = sum(k=0, n, binomial(n,k)*binomial(n+k,k) * binomial(2*k,n));
for(n=0, 35, print1(a(n),", "));  