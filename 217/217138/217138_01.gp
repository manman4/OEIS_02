M=15;

\\ a(0) = 1; a(n) = Sum_{i, j, k>=0 and i+j+2*k=n-1} a(i) * a(j) * a(2*k).
a(n) = if(n==0, 1, sum(k=0, (n-1)\2, sum(j=0, n-1-2*k, a(n-1-2*k-j) * a(j) * a(2*k))));
for(n=0, M, print1(a(n),", "));