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

float passCount = uniformBuffer.rowCount;
float passId = floor(vertexIndex / passCount);
float passPosition = vertexIndex - passId * passCount;

const float PI = 3.1415926535897932384626433832795;

float radiusUnit = 2 / (vertexCount / passCount);

vec4 calculateVertex(vec2 number)
{
  // radius gets bigger with the index
  // index == 1 -> radius == radiusUnit
  // index == 2 -> radius == radiusUnit + 1 * radiusUnit/passCount
  // ...
  // index == 1+passCount -> radius == 2*radiusUnit
  // radius =
  // calculate upwards vector with the correct length, then rotate by 
  //   (index-1) mod passCount * 2pi/passCount
  float i = vertexIndex - 1;
  float r = radiusUnit + i * (radiusUnit / passCount);
  
  float phase = atan(number.y,number.x);
  if (phase > PI / passCount) { phase = PI / passCount; }
  if (phase < - PI / passCount) { phase = - PI / passCount; }
  
  float len = length(number);
  if (len > 1. ) len = 1.;

  float rad = mod(i,passCount) * 2 * PI / passCount;
  rad += phase;
  vec2 up = vec2(0.0f,r);

  float sr = sin(rad);
  float cr = cos(rad);
  float x = up.x * cr - up.y * sr;
  float y = up.x * sr + up.y * cr;
  float z = phase/abs(phase) * len;
  
  return vec4(x, y, z, 1.);
}

float epsilon = 0.001;

void main()
{
  vec4 vertex = vec4(0., 0., 0., 1.);
  if (vertexIndex != 0) {
    vertex = calculateVertex(inComplex);
  }
  gl_Position = applyModelViewProjection(vertex);
  fragColor = vec3(inComplex, vertexIndex / vertexCount);
  fragColor *= vertexIndex / vertexCount;
}
