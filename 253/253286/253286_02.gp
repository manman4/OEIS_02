\\ A253286
\\ T(n,k) = (2*n+k-2)     * T(n-1,k) -       (n-1) * (n-2) * T(n-2,k) for n > 1.
\\ A341033
\\ T(n,k) = (2*k*n-2*k+1) * T(n-1,k) - k^2 * (n-1) * (n-2) * T(n-2,k) for n > 1.

T(n, k) = if(n<2, (k-1)*n+1, (2*n+k-2)*T(n-1, k)-(n-1)*(n-2)*T(n-2, k));

\\ upward antidiagonals
for(n=0, 9, for(k=0, n, print1(T(n-k, k), ", ")))