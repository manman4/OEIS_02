\\ E.g.f.: (1/(1+x)) * Sum_{k=0..4} Stirling2(5,k+1) * (-log(1+x))^k.
my(N=30, x='x+O('x^N)); Vec(serlaplace(1/(1+x) * sum(k=0, 4, stirling(5,k+1,2) * (-log(1+x))^k )))


