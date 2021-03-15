function f = myfun(x),
  [f, c, ceq] = mlbsuite(x, 38, 0, 'g16');
  f = - f; % because we want to minimize and its a MAX problem
