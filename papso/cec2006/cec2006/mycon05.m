function [c,ceq] = mycon05(x),
%G05 (Hock and Schittkowski, 1981, problem 74)
%usage: [c,ceq] = mycon05(x)

[f, c, ceq] = mlbsuite(x, 2, 3, 'g05');
c = []; % they are linear so delete them