\\ a(n) = Sum_{k=0..n} binomial(n,k)^2 * binomial(2*k,k).
a002893(n) = sum(k=0, n, binomial(n, k)^2 * binomial(2*k, k));
\\ a(n) = Sum_{k=0..n} binomial(n,k) * binomial(n+k,k) * A002893(k).
a(n) = sum(k=0, n, binomial(n, k) * binomial(n+k, k) * a002893(k));
for(n=0, 22, print1(a(n), ", ")); 