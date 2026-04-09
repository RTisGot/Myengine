#version 330 core
layout (location = 0) in vec3 aPos;
uniform mat4 model;      // •¨‘Ì‚ÌˆÊ’u
uniform mat4 view;       // ƒJƒƒ‰‚ÌˆÊ’u
uniform mat4 projection; // ‰æ–Ê”ä—¦
void main() {
   gl_Position = projection * view * model * vec4(aPos, 1.0);
}