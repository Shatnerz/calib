function [ value ] = digitalRead (pin)
% analogRead(pin) returns value read from digital pin
  [ rows cols ] = size (pin);
  if ((rows ~= 1) || (cols ~= 1)) 
    error('pin must be a single value')
  end
  if ((pin < 2)||(pin > 13))
    error('pin must be in the range 0 through 13');
  end
  rc = arduinoIO([ 4 1 pin ]);
  if ((rc(1) ~= 4) || (rc(2) ~= 1))
    error('some thing bad has happened');
  end
  value = rc(3);
end
