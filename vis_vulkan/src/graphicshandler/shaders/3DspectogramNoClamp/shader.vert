
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

  float len = length(number) - 1;
  // if (len > 1.0f) len = 1.0f;

  return vec4(x, y, len, 1.0f);
}

float epsilon = 0.001;

float clamp(float num) {
  if (num > 1.0f) return 1.0f;
  if (num < 0.0f) return 0.0f;
  return num;
}

void main()
{
  vec4 vertex = calculateVertex(inComplex);
  gl_Position = applyModelViewProjection(vertex);

  // gradient top to bottom
  vertex = ((vertex)) / 2;
  float green = clamp(-(vertex.z * vertex.z - 0.5f) + 1);
  float blue = clamp(vertex.z * vertex.z);
  float red = clamp(1 / vertex.z);
  // gradient from back to front (desaturate)
  float middle = (red + green + blue) / 3;
  float amount = (rowCount - rowId) / rowCount;

  float redDiff = (middle - red) * amount;
  float greenDiff = (middle - green) * amount;
  float blueDiff = (middle - blue) * amount;

  red   += redDiff;
  green += greenDiff;
  blue  += blueDiff;

  fragColor = vec3(red, green, blue);
  fragColor *= rowId / rowCount;
}
