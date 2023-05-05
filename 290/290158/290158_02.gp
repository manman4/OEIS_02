M=15;

a(n) = (-1)^n*n!* polcoef(exp(n * x * (exp(x) - 1)), n);
for(n=0, M, print1(a(n),", "))
