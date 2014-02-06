data2=[];
for i=0:255
    
    a = -0.001321569994774;
    b=0.019198139388147;
    analogWrite(10,i);
    data1(i+1,1)=analogRead(5);
    data1(i+1,2)=a+b*i;
    pause(1)
end