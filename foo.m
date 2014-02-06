function [] = foo()
for i=0:4:255
    analogWrite(10,i);
    fprintf('%d\n',i);
    fprintf('%d\n',i);
end