\\ a(n) = n! * Sum_{k=0..n} (-n)^(n-k) * binomial(n+k-1,k)/(n-k)!.
a(n) = n! * sum(k=0, n, (-n)^(n-k) * binomial(n+k-1,k)/(n-k)!)
for(n=0, 20, print1(a(n),", "))   

