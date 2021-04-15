c(n) = binomial(2*n, n)/(n+1);
a(n) = local(A=1+x*O(x^n)); for(i=1, n, A=1-c(n-i+1)*x/A); polcoeff(1/A, n);
M=100;
for(n=0, M, i=a(n); if((i<0)+#digits(i)>1000, break); write("/Users/seiichimanyama/Desktop/b268646_1.txt", n, " ", i))