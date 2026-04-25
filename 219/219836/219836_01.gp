T(n, k) = if(n==0, 0, (k+1)*T(n-1, k)+(n-k)*T(n-1, k-1)+(-1)^n*(k==n-1));
vector(13, n, T(n-1,0))
vector(13, n, T(n-1,1))
vector(13, n, T(n-1,2))
vector(13, n, T(n-1,3))
