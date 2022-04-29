M=23;

a(n) = if(n==0, 1, (n-1)!*sum(k=1, sqrtint(n), a(n-k^2)/(n-k^2)!));
for(n=0, M, print1(a(n), ", "));