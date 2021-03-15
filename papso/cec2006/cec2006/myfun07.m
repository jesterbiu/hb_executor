function f = myfun07(x),
%G07 (Hock and Schittkowski, 1981, problem 113)
%usage:  [f] = myfun07(x); and [c,ceq] = myfun07(x)
%
%lb = [-10*ones(10,1)];
%ub = [10*ones(10,1)];
%A = [4 5 0 0 0 0 -3 9 0 0;10 -8 0 0 0 0 -17 2 0 0;-8 2 0 0 0 0 0 0 5 -2];
%b = [105; 0; 12];
%FcnCount = 0; % count the number of function evaluations
%for i = 1:100, % or we can use FcnCount as a stop criteria
%  x0 = lb + rand(10,1).*(ub-lb); % random starting point
%  [X07(:,i),F07(i),exitflag,output] = fmincon('myfun07',x0,A,b,[],[],lb,ub,'mycon07');
%  FcnCount = output.funcCount + FcnCount; % increment function evaluation counter
%end
%% now pick out the best solution found from all these hill-climbs
%[dummy,i] = min(F07); x07 = X07(:,i), [f07,g07,h07] = mlbsuite(x07,8,0,'g07'), FcnCount

[f, c, ceq] = mlbsuite(x, 8, 0, 'g07');
