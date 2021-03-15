function f=myfun22(x)
%G22
%usage:  [f] = myfun22(x); and [c,ceq] = myfun22(x)
%
%lb=[0 0 0 0 0 0 0 100 100 100.01 100 100 0 0 0 0.01 0.01 -4.7 -4.7 -4.7 -4.7 -4.7]';
%ub=[20000 1e+6 1e+6 1e+6 4e+7 4e+7 4e+7 299.99 399.99 300 400 600 500 500 500 300 400 6.25 6.25 6.25 6.25 6.25]';
%Aeq=[0 0 0 0 1 0 0 -100000 zeros(1,14);
%     0 0 0 0 0 1 0 100000 -100000 zeros(1,13);
%     0 0 0 0 0 0 1 0 100000 zeros(1,13);
%     0 0 0 0 1 0 0 0 0 100000 zeros(1,12);
%     0 0 0 0 0 1 0 0 0 0 100000 zeros(1,11);
%     0 0 0 0 0 0 1 0 0 0 0 100000 zeros(1,10);
%     0 0 0 0 0 0 0 1 0 0 -1 0 0 0 0 1 zeros(1,6);
%     0 0 0 0 0 0 0 0 1 0 0 -1 0 0 0 0 1 zeros(1,5)];
%beq=[-1e+7, 0, 5e+7, 3.3e+7, 4.4e+7, 6.6e+7 0 0]';
%FcnCount = 0; % count the number of function evaluations
%for i = 1:100, % or we can use FcnCount as a stop criteria
%  x0 = lb + rand(22,1).*(ub-lb); % random starting point
%  [X22(:,i),F22(i),exitflag,output] = fmincon('myfun22',x0,[],[],Aeq,beq,lb,ub,'mycon22');
%  FcnCount = output.funcCount + FcnCount; % increment function evaluation counter
%end
%% now pick out the best solution found from all these hill-climbs
%[dummy,i] = min(F22); x22 = X22(:,i),[f22,g22,h22]=mlbsuite(x22,1,19,'g22'), FcnCount

% Jane Jing Liang, Nov 2005
[f, g, h] = mlbsuite(x, 1, 19, 'g22');
