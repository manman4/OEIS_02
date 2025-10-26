\\ E.g.f.: 1 + Series_Reversion( x * exp(-x/(1+x)) ).
my(N=20, x='x+O('x^N)); Vec(1 + serlaplace( serreverse( x * exp(-x/(1+x)) )))

