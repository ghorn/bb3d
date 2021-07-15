#version 400 core
layout (location = 0) in vec3 position;
layout (location = 1) in vec4 vert_color;
out vec4 frag_color_in;
uniform mat4 view;
uniform mat4 proj;
uniform float point_size;
void main()
{
  gl_Position = proj * view * vec4(position, 1.0);
  frag_color_in = vert_color;
  gl_PointSize = point_size;
}
