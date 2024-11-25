\\ a(n) = [x^n] (1 + x + (1 + x)^3)^n.
a(n) = polcoef( (1 + x + (1 + x)^3)^n, n);
for(n=0, 20, print1(a(n), ", "))

