\\ a(n) = (n!)^3 * [(x*y*z)^n] 1 / (1 - (exp(x) - 1) * (exp(y) - 1) * (exp(z) - 1)).
b(n, k, m) =  my(x='x+O('x^(n+1)), y='y+O('y^(k+1)), z='z+O(z^(m+1))); n!*k!*m!*polcoef(polcoef(polcoef(1/(1 - (exp(x)-1)*(exp(y)-1)*(exp(z)-1)), n), k), m); 

a(n) = sum(k=0, n, (k!*stirling(n, k, 2))^3); 
for(n=0, 20, print1(a(n), ", "));
for(n=0, 20, print1(a(n)-b(n, n, n), ", "));