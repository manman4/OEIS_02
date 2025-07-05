default(parisize, 1200000000)

M=25;

\\ Diagonal of the rational function 1/(1 - (x^2 + y^2 + z^2 + 3*x*y*z)).
a(n) = my(x='x+O('x^(n+1)), y='y+O('y^(n+1)), z='z+O('z^(n+1))); polcoef(polcoef(polcoef(1/(1-(x^2+y^2+z^2+3*x*y*z)), n), n), n);
for(n=0, M, print1(a(n),", "));

