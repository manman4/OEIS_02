A032033(n) = polcoef(sum(m=0, n, 3^m*m!*x^m/prod(k=1, m, 1-k*x+x*O(x^n))), n);
for(n=0, 20, print1(A032033(n), ", "));

a(n) = (4/3)*A032033(n) - (1/3)*0^n;
for(n=0, 20, print1(a(n), ", "));