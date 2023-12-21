A094417(n) = polcoef(sum(m=0, n, 4^m*m!*x^m/prod(k=1, m, 1-k*x+x*O(x^n))), n);
for(n=0, 20, print1(A094417(n), ", "));

a(n) = (5/4)*A094417(n) - (1/4)*0^n;
for(n=0, 20, print1(a(n), ", "));