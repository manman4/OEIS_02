a(n) = if(n==0, 1, sum(i=1, n, a(n-i)*sum(j=0, 3*i, 2^(j-2*i)*(-1)^(i-j)*binomial(i, 3*i-j)*binomial(i+j-1, i-1)))/n);
for(n=0, 20, print1(a(n),", "))    