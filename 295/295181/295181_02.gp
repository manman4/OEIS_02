M=11;

\\ A(0,k) = 1, A(1,k) = 0; A(n,k) = (n-1) * (A(n-1,k) + k*A(n-2,k)).
a(n, k) = if(n<2, 1-n, (n-1)*(a(n-1, k) + k*a(n-2, k)));
for(n=0, M, for(k=0, n, print1(a(k, n-k),", ")))


