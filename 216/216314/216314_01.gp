M=21;

\\ a(n) = Sum_{k=0..n} 2^k * binomial(n+k+1,k) * binomial(n+k+1,n-k) / (n+k+1). 
a(n) = sum(k=0, n, 2^k * binomial(n+k+1,k) * binomial(n+k+1,n-k) / (n+k+1));
for(n=0, M, print1(a(n)", "))