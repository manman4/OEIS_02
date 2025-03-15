\\ E.g.f.: exp( (1/x) * Series_Reversion( x*(1-x) )^2 ).
my(N=20, x='x+O('x^N)); Vec(serlaplace( exp( (1/x) * serreverse(x*(1-x))^2 ) ) )



