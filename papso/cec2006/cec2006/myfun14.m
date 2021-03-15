function f=myfun14(x)
%G14
%usage:  [f] = myfun14(x);
%
%opts=optimset('fmincon');
%opts.MaxFunEvals=100000;
%opts.MaxIter=10000;
%lb=zeros(10,1);
%ub=10.*ones(10,1);
%Aeq = [1 2 2 0 0 1 0 0 0 1;
%       0 0 0 1 2 1 1 0 0 0;
%       0 0 1 0 0 0 1 1 2 1];
%Beq = [2 1 1]';
%FcnCount = 0; % count the number of function evaluations
%for i = 1:100, % or we can use FcnCount as a stop criteria
%  x0 = lb + rand(10,1).*(ub-lb);
%  [X14(:,i),F14(i),exitflag,output] = fmincon('myfun14',x0,[],[],Aeq,Beq,lb,ub,[],opts);
%  FcnCount = output.funcCount + FcnCount; % increment function evaluation counter
%end
%% now pick out the best solution found from all these hill-climbs
%[dummy,i] = min(F14); x14 = X14(:,i), [f14,g14,h14] = mlbsuite(x14,0,3,'g14'), FcnCount

% Jane Jing Liang, Nov 2005

[f,g,h] = mlbsuite(x, 0, 3, 'g14');
