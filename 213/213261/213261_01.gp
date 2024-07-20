a(n) = numbpart(7*n+5);                                                                                                     
for(n=0, 30, print1(a(n),", "))

my(N=30, x='x+O('x^N)); Vec(7 * prod(k=1, N, (1 - x^(7*k))^3 / (1 - x^k)^4 ) + 49 * x * prod(k=1, N, (1 - x^(7*k))^7 / (1 - x^k)^8 ) )