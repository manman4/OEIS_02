\\ T(n,m) = -1 if n=2*m otherwise -m/(n-2*m)*binomial(2*n-3*m-1,n-m).
T(n, m) = if(n==2*m, -1, -m/(n-2*m)*binomial(2*n-3*m-1, n-m));
for(n=1, 11, for(k=1, n, print1(T(n, k),", ")));
