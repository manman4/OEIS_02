Bell_poly(n) = sum(k=0, n, stirling(n, k, 2)*x^k);

\\ a(n) = n! * [x^n] B_n(x) * exp(x) / (1-x), where B_n(x) = Bell polynomials.
a(n) = my(x='x+O('x^(n+1))); n!*polcoef(Bell_poly(n) * exp(x) / (1-x), n);
for(n=0, 18, print1(a(n), ", "));       