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

float epsilon = 0.001;

float vertexCount = uniformBuffer.vertexCount;

int vertexIndex = int(vertexCount) - gl_VertexIndex;
// int vertexIndex = gl_VertexIndex;

float passCount =  vertexCount / uniformBuffer.rowCount / 2;
float passId = floor(vertexIndex / passCount);
float passPosition = vertexIndex - passId * passCount;

const float PI = 3.1415926535897932384626433832795;

// float radiusUnitOdd = (4./3.) / (vertexCount / passCount);
float radiusUnit = 2 / (vertexCount / passCount / 2);

vec4 calculateVertex(vec2 number)
{
  float i = vertexIndex / 2;
  
  float r = radiusUnit + i * (radiusUnit / passCount);
  if (mod(vertexIndex,2) != 0)
    r +=  0.1 * radiusUnit;

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

void main()
{
  vec4 vertex = calculateVertex(inComplex);
  gl_Position = applyModelViewProjection(vertex);
  fragColor = vec3(vertexIndex / vertexCount, inComplex);
  fragColor *= 1 - vertexIndex / vertexCount;
  // if (mod(vertexIndex, 2) == 0)
  //   fragColor = vec3(1,0,0);
  // else
  //   fragColor = vec3(0,1,1);
}
