\\ Diagonal of the rational function 1 / ((1-x)*(1-y) - (x*y)^4).
a(n) = my(x='x+O('x^(n+1)), y='y+O('y^(n+1)) ); polcoef(polcoef(1 / ((1-x)*(1-y) - (x*y)^4), n), n);
for(n=0, 25, print1(a(n),", "))         