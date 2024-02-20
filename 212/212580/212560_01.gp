\\ G.f.: Sum_{k>=0} k! * ( x * (1-x^2) )^k.
my(N=30, x='x+O('x^N)); Vec(sum(k=0, N, k! * ( x * (1-x^2) )^k))
