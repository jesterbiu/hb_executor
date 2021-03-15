function f=myfun15(x)
%G15
%
%lb = [0 0 0]';
%ub = [10 10 10]';
%Aeq = [8 14 7];
%Beq = 56;
%FcnCount = 0; % count the number of function evaluations
%for i = 1:100, % or we can use FcnCount as a stop criteria
%  x0 = lb + rand(3,1).*(ub-lb);
%  [X15(:,i),F15(i),exitflag,output] = fmincon('myfun15',x0,[],[],Aeq,Beq,lb,ub,'mycon15');
%  FcnCount = output.funcCount + FcnCount; % increment function evaluation counter
%end
%% now pick out the best solution found from all these hill-climbs
%[dummy,i] = min(F15); x15 = X15(:,i), [f15,g15,h15] = mlbsuite(x15, 0, 2, 'g15'), FcnCount

% Jane Jing Liang, Nov 2005

[f,g,h] = mlbsuite(x, 0, 2, 'g15');