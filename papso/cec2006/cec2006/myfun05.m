function f = myfun05(x),
%G05 (Hock and Schittkowski, 1981, problem 74)
%usage:  [f] = myfun05(x); and [c,ceq] = myfun05(x)
%
%lb = [0;0;-0.55;-0.55];
%ub = [1200;1200;0.55;0.55];
%A = [0 0 1 -1; 0 0 -1 1];
%b = [0.55;0.55];
%FcnCount = 0; % count the number of function evaluations
%for i = 1:100, % or we can use FcnCount as a stop criteria
%  x0 = lb + rand(4,1).*(ub-lb); % random starting point
%  [X05(:,i),F05(i),exitflag,output] = fmincon('myfun05',x0,A,b,[],[],lb,ub,'mycon05');
%  FcnCount = output.funcCount + FcnCount; % increment function evaluation counter
%end
%% now pick out the best solution found from all these hill-climbs
%[dummy,i] = min(F05); x05 = X05(:,i), [f05,g05,h05] = mlbsuite(x05,2,3,'g05'), FcnCount

[f, c, ceq] = mlbsuite(x, 2, 3, 'g05');
