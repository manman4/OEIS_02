M=15;

a(n, k) = my(x='x+O('x^(n+1)), y='y+O('y^(k+1))); polcoef(polcoef(1/(1-x-y-x^2-y^2-x^3-y^3), n), k);
for(n=0, M, for(k=0, n, print1(a(k, n-k),", ")));

a(n, k) = my(x='x+O('x^(n+1)), y='y+O('y^(k+1))); polcoef(polcoef(prod(j=1, n+k, 1+x^j+y^(2*j)/(1-y^j)), n), k);
for(n=0, M, for(k=0, n, print1(a(k, n-k),", ")));
matrix(11, 11, n, k, a(n-1, k-1)) 


for(n=0, 30, print1(a(n, 0),", "));
for(n=0, 30, print1(a(n, 1),", "));
for(n=0, 30, print1(a(n, 2),", "));
for(n=0, 30, print1(a(n, 3),", "));
for(n=0, 30, print1(a(n, 4),", "));
for(n=0, 30, print1(a(n, 5),", "));

print

for(n=0, 30, print1(a(0, n),", "));
for(n=0, 30, print1(a(1, n),", "));
for(n=0, 30, print1(a(2, n),", "));
for(n=0, 30, print1(a(3, n),", "));
for(n=0, 30, print1(a(4, n),", "));
for(n=0, 30, print1(a(5, n),", "));