T(n, k) = sum(j=0, n, (-1)^(n-j)*binomial(n-1, n-j)*abs(stirling(j, k)));
for(n=0, 9, for(k=0, n, print1(T(n, k), ", ")));
