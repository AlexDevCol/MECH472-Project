
this example shows how to acquire images in order to apply
computer vision functions to a simulation with the 
3D graphics library (ie the car simulation example)

this type of example can be used as a starting point
for optional "crossover" topics between MECH 471 / 6621
(self driving cars, range finding sensor simulation, etc.)
and MECH 472 / 6631 (automated car systems, etc.)

it can also be used for MECH 472 / 6631 for developing 
your own vision simulator with 3D vision / 1st person
vision capability, etc.

note that the car in the simulation starts off the screen
for the 640x480 window -- just press the upward arrow
key and the car will appear in 3 seconds

also make sure you select the 3D graphics window when
you use keyboard input or the key input may not respond

changes to the car simulation example have a * marked beside them

note that if the 3D graphics window is too large the 
computer might not be fast enough to peform all the 
operations required for both 3D graphics and vision

the vision library files "image_transfer7.h/cpp" used with
this example are a simplified version of vision_lib_version_7.53,
where the image acquistion and video functions have been removed
