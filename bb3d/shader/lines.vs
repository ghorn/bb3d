#version 400 core
layout (location = 0) in vec3 position;
uniform mat4 view;
uniform mat4 proj;
uniform float point_size;
void main()
{
  gl_Position = proj * view * vec4(position, 1.0);
  gl_PointSize = point_size;
}
