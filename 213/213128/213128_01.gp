M=18;

a(n) = sum(k=0, n, k!*(-1)^k*6^(n-k)*stirling(n, k, 2));
for(n=0, M, print1(a(n), ", "));