function f=myfun23(x)
%G23
%usage:  [f] = myfun23(x); and [c,ceq] = myfun23(x)
%
%lb=[0 0 0 0 0 0 0 0 0.01]';
%ub=[300 300 100 200 100 300 100 200 0.03]';
%Aeq=[1 1 -1 -1 0 0 0 0 0;
%     0 0 1 0 -1 1 0 0 0;
%     0 0 0 1 0 0 1 -1 0];
%beq=[0 0 0]';
%FcnCount = 0; % count the number of function evaluations
%for i = 1:100, % or we can use FcnCount as a stop criteria
%  x0 = lb + rand(9,1).*(ub-lb); % random starting point
%  [X23(:,i),F23(i),exitflag,output] = fmincon('myfun23',x0,[],[],Aeq,beq,lb,ub,'mycon23');
%  FcnCount = output.funcCount + FcnCount; % increment function evaluation counter
%end
%% now pick out the best solution found from all these hill-climbs
%[dummy,i] = min(F23); x23 = X23(:,i),[f23,g23,h23]=mlbsuite(x23,2,4,'g23'), FcnCount

% Jane Jing Liang, Nov 2005

[f,g,h] = mlbsuite(x, 2, 4, 'g23');
