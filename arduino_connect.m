function [ success ] = arduino_connect ( )
% arduino_connect () opens connection to arduino
  rc = arduinoIO([ 0 1 0 ]);
  if ((rc(1) ~= 0) || (rc(2) ~= 1) || (rc(3) ~= 0)) 
    error('something very bad happened');
    success = 0;
  end
  success = 1;
end
