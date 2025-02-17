a136630(n, k) = 1/(2^k*k!)*sum(j=0, k, (-1)^(k-j)*(2*j-k)^n*binomial(k, j));

\\ a(n) = Sum_{k=0..n} k! * A185951(n,k).
a(n) = sum(k=0, n, k!*a185951(n, k));   
for(n=0, 20, print1(a(n), ", "))



