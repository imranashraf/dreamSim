clear all,clc


colArray = [ 'r-*' , 'b-^', 'k-o','g-s', 'm-d','c-v','b:p','r:x'];
col=1;
dirname=fopen('dname.txt','r');

for j=1:8
    D1=fscanf(dirname,'%s /n')
    filename=fopen('fname.txt','r');
    cd(D1) 
    for k=1:16
        S1=fscanf(filename,'%s /n')
        fid=fopen(S1,'r');
        for i=1:23
            if(i>5&&i<10)
                tline = fgetl(fid);
            else
                S=fscanf(fid,'%s /t')
                A=fscanf(fid,'%f /t /n')
            end
        if(i==1)
            TTG(k,j)=A;
        end
        if(i==23)
            AWTPT(k,j)=A; %Avg waiting time per task
        end
        end %end i loop
    fclose(fid);
    end %end k loop
    cd ..
    %semilogx(TTG(:,j),ASSPT(:,j)) %this can be used for plotting
                                        %without markers

     loglog(TTG(:,j),AWTPT(:,j),colArray(col:col+2) )
     col=col+3;
    str(j) = cellstr(D1);
    hold on
end %end j loop

leg=legend(str{:});
grid on,shg

xlabel('Number of tasks')
%ylabel('Total simulation time')
%ylabel('Avg. waiting time per task')
%ylabel('Avg. scheduling steps per task')
ylabel('Avg. configuration count per node')

set(gca,'FontSize',16)
xlhand = get(gca,'xlabel')
set(xlhand,'fontsize',16)
ylhand = get(gca,'ylabel')
set(ylhand,'fontsize',16)

set(leg,'FontSize',8)

hold off
fclose('all');
