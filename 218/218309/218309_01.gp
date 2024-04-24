\\ E.g.f.: A(x) = 1/2 + 1/2 * exp( 4*x - 4/3 * LambertW(-3*x * exp(3*x)) ).
my(x='x+O(x^20)); Vec( serlaplace(1/2 + 1/2 * exp( 4*x - 4/3 * lambertw(-3*x * exp(3*x)) )))


