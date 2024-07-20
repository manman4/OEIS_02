a(n) = numbpart(5*n+4);                                                                                                     
for(n=0, 30, print1(a(n),", "))

my(N=30, x='x+O('x^N)); Vec(5 * prod(k=1, N, (1 - x^(5*k))^5 / (1 - x^k)^6 ))