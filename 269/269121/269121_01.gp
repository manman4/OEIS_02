
a(n) = sum(k=0, 3*n, (3*n+1-k)*binomial(3*n, k))-3*sum(k=0, 2*n-1, (2*n-k)*binomial(3*n, k))+3*sum(k=0, n-2, (n-1-k)*binomial(3*n, k));
for(n=0, 15, print1(a(n), ", "));