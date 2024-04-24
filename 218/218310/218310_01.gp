\\ E.g.f.: A(x) = 1/2 + 1/2 * exp( x - 1/5 * LambertW(-5*x * exp(5*x)) ).
my(x='x+O(x^20)); Vec( serlaplace(1/2 + 1/2 * exp( x - 1/5 * lambertw(-5*x * exp(5*x)) ) ))


