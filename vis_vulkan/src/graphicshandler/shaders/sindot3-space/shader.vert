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

int vertexIndex = gl_VertexIndex;

float vertexCount = uniformBuffer.vertexCount;
float rowCount = uniformBuffer.rowCount;

float rowSize = vertexCount / rowCount;
float rowId = floor(vertexIndex / rowSize);
float rowPosition = vertexIndex - rowId * rowSize;

const float PI = 3.1415926535897932384626433832795;

vec4 calculateVertex(vec2 number)
{
  vec2 normalized = number;
  vec4 result = vec4(normalized, sin(dot(number, number) * 2 * PI), 1);
  result *= 2;
  return result;
}

float epsilon = 0.001;

void main()
{
  vec4 vertex = calculateVertex(inComplex);
  gl_Position = applyModelViewProjection(vertex);
  float len = length(vertex);
  fragColor = vec3(len, 
sin(vertexIndex / vertexCount * 2 * PI), atan(inComplex));
  fragColor *= rowId/rowCount;
}
