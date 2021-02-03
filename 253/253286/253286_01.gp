T(n, k) = if(n==0, 1, n!*sum(j=1, n, k^j*binomial(n-1, j-1)/j!));

\\ upward antidiagonals
for(n=0, 9, for(k=0, n, print1(T(n-k, k), ", ")))