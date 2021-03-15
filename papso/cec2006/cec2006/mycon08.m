function [c,ceq] = mycon08(x),
%G08 (Schoenauer and Xanthakis, 1993)
%usage: [c,ceq] = mycon08(x)

[f, c, ceq] = mlbsuite(x, 2, 0, 'g08');
