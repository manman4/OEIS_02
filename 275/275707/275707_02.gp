M=30;

a(n) = sum(k=0, n, 2^k*k^(n-k)*binomial(n, k)); 
for(n=0, M, print1(a(n), ", "));