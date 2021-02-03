\\ A253286
\\ T(n,k) = n!*Sum_{j=1..n} k^j    *binomial(n-1, j-1)/j! for n > 0.
\\ A341033
\\ T(n,k) = n!*Sum_{j=1..n} k^(n-j)*binomial(n-1, j-1)/j! for n > 0.

T(n, k) = if(n==0, 1, n!*sum(j=1, n, k^j*binomial(n-1, j-1)/j!));

\\ upward antidiagonals
for(n=0, 9, for(k=0, n, print1(T(n-k, k), ", ")))