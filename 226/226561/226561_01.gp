M=20;

a(n) = sum(k=1, n, (n/gcd(k, n))^n); 
for(n=1, M, print1(a(n), ", "));