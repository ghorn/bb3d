#version 400 core
in vec2 texture_coordinate_;
uniform sampler2D image_texture;
out vec4 FragColor;
void main()
{
  FragColor = texture(image_texture, texture_coordinate_);
}
