\\ A(n,k) = n! * LaguerreL(n, k-1, -1).
a(n, k) = n! * pollaguerre(n, k-1, -1);
for(n=0, 9, for(k=0, n, print1(a(k, n-k),", ")))