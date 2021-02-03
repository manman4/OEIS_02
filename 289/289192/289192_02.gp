T(n, k) = if(n<2, k*n+1, (2*n+k-1)*T(n-1, k)-(n-1)^2*T(n-2, k));
for(n=0, 9, for(k=0, n, print1(T(k, n-k), ", ")))