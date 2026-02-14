\\ a(n) = Sum_{k=0..n} binomial(n+k,k)^2 * binomial(3*n+k+1,2*n+1).
a(n) = sum(k=0, n, binomial(n+k,k)^2 * binomial(3*n+k+1,2*n+1));
for(n=0, 22, print1(a(n), ", ")); 