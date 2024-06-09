v(n)={x='x+O('x^(n+10)); serlaplace( exp(x/(x^3-1)) ) };
M=447;
v=v(M);
for(n=0, M, write("/Users/xxx/Desktop/b293565_1.txt", n, " ", polcoef(v, n)))
