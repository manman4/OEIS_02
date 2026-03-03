\\ a(0) = 0, a(1) = 1; a(n) = n! * [x^n] exp(n*x) * Sum_{k=1..n-1} a(k)*x^k/k!.
a(n) = if(n<2, n, n!*polcoef(exp(n*x+O(x^(n+5))) * sum(k=1, n-1, a(k)*x^k/k!), n));                                                     
for(n=0, 19, print1(a(n),", ")); 