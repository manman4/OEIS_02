a(n) = polcoef( ( (1 - x + x^2) / (1 - x + x*O(x^n))^2 )^n, n); 
for(n=0, 20, print1(a(n), ", "))
