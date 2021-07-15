#version 400 core
layout (location = 0) in vec3 position;
layout (location = 1) in vec2 texture_coordinate;
out vec2 texture_coordinate_;
uniform mat4 view;
uniform mat4 proj;
void main()
{
  texture_coordinate_ = texture_coordinate;
  gl_Position = proj * view * vec4(position, 1.0);
}
