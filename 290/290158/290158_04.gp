M=20;

a(n) = sum(k=0, n, (-n)^(n-k) * k^k * binomial(n,k));
for(n=0, M, print1(a(n),", "))
