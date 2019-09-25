
#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(binding = 0) uniform UniformBufferObject
{
  mat4 model;
  mat4 view;
  mat4 projection;
  float vertexCount;
  float rowCount;
}
uniformBuffer;

// if using different input change the attribute descriptions as well
layout(location = 0) in vec2 inComplex;

layout(location = 0) out vec3 fragColor;

out gl_PerVertex { vec4 gl_Position; };

vec4 applyModelViewProjection(vec4 vertex)
{
  return uniformBuffer.projection * uniformBuffer.view * uniformBuffer.model * vertex;
}

// int vertexIndex = -gl_VertexIndex + int(uniformBuffer.vertexCount);
int vertexIndex = gl_VertexIndex;

float vertexCount = uniformBuffer.vertexCount;
float rowCount = uniformBuffer.rowCount;

float rowSize = vertexCount / rowCount;
float rowId = floor(vertexIndex / rowSize);
float rowPosition = vertexIndex - rowId * rowSize;

vec4 calculateVertex(vec2 number)
{
  float x = rowPosition / rowSize * 2 - 1;
  float y = rowId / rowCount * 2 - 1;

  float len = length(number);
  if (len > 1.0f)
    len = 1.0f;

  return vec4(x, y, len * 2 - 1, 1.0f);
}

float epsilon = 0.001;

void main()
{
  vec4 vertex = calculateVertex(inComplex);
  gl_Position = applyModelViewProjection(vertex);

  // gradient from back to front
  float red = vertexIndex / uniformBuffer.vertexCount;
  float green = 1.0f - (vertexIndex / uniformBuffer.vertexCount);
  float blue = 1.0f;
  // gradient top to bottom
  vertex = (vertex + 1) / 2; // normalize coordinates to [0,1]
  red *= vertex.z;
  green *= -abs(vertex.z - 0.5f) + 1;
  blue *= 1.0f - vertex.z;

  fragColor = vec3(red, green, blue);
  fragColor *= rowId / rowCount;
}
