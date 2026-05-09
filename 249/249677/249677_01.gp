\\ T(n,k) = T(n-1,k) + n^3 * T(n-1,k-1).
T(n, k) = if(n*k==0, n^k, T(n-1, k)+n^3*T(n-1, k-1));
for(n=0, 10, for(k=0, n, print1(T(n, k),", ")));