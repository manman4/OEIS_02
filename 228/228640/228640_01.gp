M=20;

a(n) = sum(k=1, n, n^gcd(k, n)); 
for(n=0, M, print1(a(n), ", "));