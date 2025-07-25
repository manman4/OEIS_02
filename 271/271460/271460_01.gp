\\ T(n,m) = -2 if 2*n=3*m otherwise -m/(2*n-3*m)*binomial(3*n-4*m-1,n-m).
T(n, m) = if(2*n==3*m, -2, -m/(2*n-3*m)*binomial(3*n-4*m-1, n-m));
for(n=1, 11, for(k=1, n, print1(T(n, k),", ")));
