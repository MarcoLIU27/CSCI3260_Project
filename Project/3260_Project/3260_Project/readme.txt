CSCI3260 Assignment 2 Texture Mapping/ Lighting Control  

Name: LIU Haoyu
Student ID: 1155141556

Manipulation:

Keyboard:
        Key "Esc": exit
        Key "W": increase the brightness of directional light, within a certain range
        Key "S": reduce the brightness of directional light, within a certain range
        Key "1" & "2": switch two different textures for the tiger
        Key "3" & "4": switch two different textures for the ground surface
        Key "↑"(Up): upward movement of the tiger
        Key "↓"(Down): downward movement of the tiger
        Key "←"(Left): left rotation of the tiger
        Key "→"(Right): right rotation of the tiger
        Key "Q": forward movement of the tiger
        Key "A": backward movement of the tiger
        Key "R": the whole scene will move up vertically, within a certain range
        Key "F": the whole scene will move down vertically, within a certain range
        Key "L": random movements in the horizon plane, within a certain range

Mouse: 
        When the left button clicked and the mouse moves up/down/left/right, the camera position moves up/down/left/right accordingly, thus the whole scene you see moves down/up/right/left.

Scroll:
        Scroll the mouse wheel up: zoom in the entire scene, within a certain range
        Scroll the mouse wheel up: zoom out the entire scene, within a certain range

Lighting:
        Besides the directional light, a point light is also implemented.

# Part of the codes in the FragmentShaderCode.glsl implementing multiple lights is based on the openGL tutorial from https://learnopengl.com/Lighting/Multiple-lights
# Including the functions CalcDirLight() and CalcPointLight().

