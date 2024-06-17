\\ a(0) = 1; a(n) = (n-1)! * Sum_{k=4..n} k * a(n-k)/(n-k)!. 
a(n) = if(n==0, 1, (n-1)! * sum(k=4, n, k * a(n-k)/(n-k)! ));
for(n=0, 21, print1(a(n),", "))


