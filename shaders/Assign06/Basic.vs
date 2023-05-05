#version 430 core
// Change to 410 for macOS

layout(location=0) in vec3 position;
layout(location=1) in vec4 color;
layout(location = 2) in vec3 normal;

out vec4 vertexColor;
uniform mat4 modelMat;
uniform mat4 viewMat;
out vec4 interPos;
uniform mat4 projMat;
uniform mat3 normMat;
out vec3 interNormal;

void main()
{		
	// Get position of vertex (object space)
	vec4 objPos = vec4(position, 1.0);

	vec4 viewPos = viewMat * modelMat * objPos;
	interPos = projMat * viewPos;

	// For now, just pass along vertex position (no transformations)
	interNormal = normMat * normal;

	// Output per-vertex color
	vertexColor = color;
	gl_Position = interPos;

}

