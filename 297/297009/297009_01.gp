v(n)={x='x+O('x^(n+10)); asin(x*exp(x))};
M=100;
v=v(M);
for(n=0, M, i=n!*polcoef(v, n); if((i<0)+#digits(i)>1000, break); write("/Users/xxx/Desktop/b297009_1.txt", n, " ", i))