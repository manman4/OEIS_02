M=30;

a(n) = n!*sum(k=0, n\7, 1/7!^k*binomial(n-6*k, k)/(n-6*k)!);
for(n=0, M, print1(a(n), ", "));