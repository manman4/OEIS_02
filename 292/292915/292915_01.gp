a000670(n) = sum(k=0, n, k!*stirling(n, k, 2));
A(n, k) = 2^k*a000670(n)-sum(j=0, k-1, 2^j*(k-1-j)^n);
for(n=0, 10, for(k=0, n,  print1(A(k, n-k), ", ")));

