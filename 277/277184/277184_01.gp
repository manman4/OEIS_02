\\ E.g.f.: Series_Reversion( x * (1 - x) * exp(-x * (1 - x)) ).
my(N=20, x='x+O('x^N)); Vec(serlaplace(serreverse( x * (1 - x) * exp(-x * (1 - x)) )))

  