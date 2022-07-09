M=30;

a(n) = n!*sum(k=0, n\3, (-1)^k*stirling(n-2*k, k, 2)/(n-2*k)!);
for(n=0, M, print1(a(n), ", "));