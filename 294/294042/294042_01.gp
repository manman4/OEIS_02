bell(n) = sum(k=0, n, stirling(n, k, 2));
\\ A(n,k) = Sum_{j=0..n} k^j * Stirling1(n,j) * Bell(j). 
A(n,k) = sum(j=0, n, k^j * stirling(n, j, 1) * bell(j));
for(n=0, 10, for(k=0, n, print1(A(k,n-k), ", "))); 