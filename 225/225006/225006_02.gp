\\ a(n) = [x^n] 1/((1+x^2) * (1-x)^(2*n)).
a(n) = polcoef(1/((1+x^2) * (1-x)^(2*n) + x*O(x^n)), n);
for(n=0, 19, print1(a(n), ", "))