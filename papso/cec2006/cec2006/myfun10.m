function f = myfun10(x),
%G10 (Hock and Schittkowski, 1981, problem 106, heat exchanger design)
%usage:  [f] = myfun10(x); and [c,ceq] = myfun10(x)
%
%lb = [100;1000;1000;10*ones(5,1)];
%ub = [10000*ones(3,1);1000*ones(5,1)];
%A = [0 0 0 0.0025 0 0.0025 0 0;0 0 0 -0.0025 0.0025 0 0.0025 0; 0 0 0 0 -0.01 0 0 0.01];
%b = ones(3,1);
%FcnCount = 0; % count the number of function evaluations
%for i = 1:100, % or we can use FcnCount as a stop criteria
%  x0 = lb + rand(8,1).*(ub-lb); % random starting point
%  [X10(:,i),F10(i),exitflag,output] = fmincon('myfun10',x0,A,b,[],[],lb,ub,'mycon10');
%  FcnCount = output.funcCount + FcnCount; % increment function evaluation counter
%end
%% now pick out the best solution found from all these hill-climbs
%[dummy,i] = min(F10); x10 = X10(:,i), [f10,g10,h10] = mlbsuite(x10,6,0,'g10'), FcnCount

[f, g, h] = mlbsuite(x,6,0,'g10');
