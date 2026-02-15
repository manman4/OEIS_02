\\ a(n) = Sum_{k=0..n} binomial(n,k) * binomial(2*n-k,n) * Sum_{j=0..k} binomial(k,j) * binomial(2*n-j,n).
a(n) = sum(k=0, n, binomial(n,k)*binomial(2*n-k,n) * sum(j=0, k, binomial(k,j)*binomial(2*n-j,n)));
for(n=0, 35, print1(a(n),", "));  