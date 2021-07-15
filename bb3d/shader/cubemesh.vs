#version 400 core
layout (location = 0) in vec3 position;
layout (location = 1) in vec3 color;
out vec3 vs_color;
uniform mat4 view;
uniform mat4 proj;
uniform vec3 pos0;
void main()
{
  vs_color = color;
  gl_Position = proj * view * vec4(position, 1.0);
}
