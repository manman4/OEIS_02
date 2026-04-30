\\ a(n) = [x^(3*n)] (1-x) * (1+x)^(3*n) * (Sum_{k=0..n} x^k)^3.
a(n) = polcoef((1-x) * (1+x)^(3*n) * sum(k=0, n, x^k)^3, 3*n);
for(n=0, 15, print1(a(n), ", "));