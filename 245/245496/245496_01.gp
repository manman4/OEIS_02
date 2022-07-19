M=30;

a(n) = n!*sum(k=0, n, k^k/k!*binomial(n, k));
for(n=0, M, print1(a(n), ", "));