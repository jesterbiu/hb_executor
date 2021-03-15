function [c,ceq] = mycon(x)
  [f, c, ceq] = mlbsuite(x, 38, 0, 'g16');
