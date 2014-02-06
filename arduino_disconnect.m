function [ success ] = arduino_disconnect ( )
% arduino_disconnect () closes connection to arduino
  rc = arduinoIO([ 0 1 1 ]);
  success = 1;
end
