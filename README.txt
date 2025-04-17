This program implements shadows, reflection, texture mapping(sphere and box), and a creative modeling

Shadows:
Shadow calculation was implemented in View::shade() function

Reflection:
Reflection calculation was implemented in View::shade() function

Texture Mapping:
Texture mapping calculation was implemented in RayCastVisitor::checkForBoxIntersection() for box texture mapping and
RayCastVisitor::checkForSphereIntersection() for sphere texture mapping.
The calculated color is used in View::shade() function.

Creative Modeling:
Our creative modeling is in finalscene.txt and contains 15 objects, 1 point light, and 1 spot light
Our model shows all the planets in the solar system.
There are 10 sphere objects(9 planets and the sun) each with its own texture
There are 5 box objects to represent the reflective floor and the 4 walls(left, right, back, top) with a night sky texture.
The point light is positioned to be near the camera
The spotlight is positioned right above the sun pointing downwards(you can see the faint spotlight highlight on the reflective floor below the sun).


All works were done together.