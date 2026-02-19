\\ G.f. of column k (k >= 0): G(x, k) = Sum_{i>=0} x^(3*i+k) / ( (Product_{j=1..i} (1-x^(2*j))) * (Product_{j=1..i+k} (1-x^(2*j))) ).
\\ G.f. of column k (k < 0): x^(-k) * G(x, -k).
R(n, k) = my(x='x+O('x^(n+10))); polcoef(         sum(i=0, n, x^(3*i+k)/(prod(j=1, i, (1-x^(2*j)))*prod(j=1, i+k, (1-x^(2*j))))), n);
S(n, k) = my(x='x+O('x^(n+10))); polcoef(x^(-k) * sum(i=0, n, x^(3*i-k)/(prod(j=1, i, (1-x^(2*j)))*prod(j=1, i-k, (1-x^(2*j))))), n);
T(n, k) = if(k>=0, R(n, k), S(n, k));
\\ T(n,k), n>=0, -floor(n/2)+(n mod 2)<=k<=n
for(n=0, 11, for(k=-(n\2)+(n%2), n, print1(T(n, k),", ")));