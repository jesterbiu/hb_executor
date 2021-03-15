function [c,ceq] = mycon02(x),
%G02 (Keane, 1994)
%usage: [c,ceq] = mycon02(x)

[f, c, ceq] = mlbsuite(x, 2, 0, 'g02');
c = c(1); ceq = []; % the second constraint is linear!