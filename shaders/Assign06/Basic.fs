#version 430 core
// Change to 410 for macOS

layout(location=0) out vec4 out_color;
 
in vec4 vertexColor; // Now interpolated across face
in vec4 interPos;
in vec3 interNormal;
uniform vec4 lightPos;

void main()
{	
	vec3 N = normalize(interNormal);
	vec3 L = normalize((interPos.xyz-lightPos.xyz));

	float diffuseCo = max(0.0, dot(N,L));
	vec3 diffColor = vec3 (diffuseCo) * vertexColor.rgb;

	// Just output interpolated color
	out_color = vec4(diffColor, 1.0);
}
