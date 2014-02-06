function[] = functionGenerator[frequency,amplitude,type]

digital_voltage =(amplitude-a)./b;

if ~(strcmpi(type,'sine') |strcmpi(type,'triangle')|strcmpi(type,'square')|strcmpi(type,'sawtooth'))
    error('Input must be of type: sine, triangle, square or sawtooth')
end
    
if (strcmpi(type,'sine'))
    analogWriteVector(10.128+)
end 
    
end