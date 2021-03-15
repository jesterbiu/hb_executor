function [c,ceq] = mycon07(x),
%G07 (Hock and Schittkowski, 1981, problem 113)
%usage: [c,ceq] = mycon07(x)

[f, c, ceq] = mlbsuite(x, 8, 0, 'g07');
c(1:3) = []; % the first 3 are linear, so remove them!