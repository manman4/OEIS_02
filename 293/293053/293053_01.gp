M=10;

A(n, k) = if(n<k+2, ((n==0)||(n==k+1))*n!, 2*(n-1) * A(n-1,k) - (n-1)*(n-2) * A(n-2,k) + (k+1)!*binomial(n-1,k) * A(n-1-k,k) - k*(k+1)!*binomial(n-1,k+1) * A(n-2-k,k));
for(n=0, M, for(k=0, n, print1(A(k, n-k), ", ")));


