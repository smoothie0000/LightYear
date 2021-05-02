pedal_angle = [0 0 10 10 20 20 30 30 40 40 60 60 80 80 100 100];
vehicle_speed = [0 50 0 50 0 50 0 50 0 50 0 50 0 50 0 50];
torque = [0 -30 18 -10 35 10 50 30 62 45 82 72 103 95 120 120]';
x = [pedal_angle' vehicle_speed'];
rstool(x, torque, 'linear');

%result
%torque = 2.4437 + 1.3249 * pedal_angle - 0.345 * vehicle_speed