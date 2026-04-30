
a(n) = 2^(3*n)-3*sum(k=0, n-1, binomial(3*n, k));
for(n=0, 15, print1(a(n), ", "));