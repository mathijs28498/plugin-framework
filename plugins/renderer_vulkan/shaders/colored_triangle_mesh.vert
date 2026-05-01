#version 450

layout (location = 0) out vec3 outColor;

void main() 
{
	const vec3 positions[3] = vec3[3](
		vec3(0.3f,0.5f, 0.0f),
		vec3(-0.5f,0.5f, 0.0f),
		vec3(0.0f,-0.7f, 0.0f)
	);

	//const array of colors for the triangle
	const vec3 colors[3] = vec3[3](
		vec3(1.0f, 0.0f, 0.0f), //red
		vec3(0.0f, 1.0f, 0.0f), //green
		vec3(00.f, 0.0f, 1.0f)  //blue
	);

	//output the position of each vertex
	gl_Position = vec4(positions[gl_VertexIndex], 1.0f);
	outColor = colors[gl_VertexIndex];
}

// #version 450
// #extension GL_EXT_buffer_reference : require

// layout (location = 0) out vec3 outColor;
// layout (location = 1) out vec2 outUV;

// struct Vertex {

// 	vec3 position;
// 	float uv_x;
// 	vec3 normal;
// 	float uv_y;
// 	vec4 color;
// }; 

// layout(buffer_reference, std430) readonly buffer VertexBufferReference{ 
// 	Vertex vertices[];
// };

// //push constants block
// layout( push_constant ) uniform constants
// {	
// 	mat4 render_matrix;
// 	VertexBufferReference vertexBufferReference;
// } PushConstants;

// void main() 
// {	
// 	//load vertex data from device adress
// 	Vertex v = PushConstants.vertexBufferReference.vertices[gl_VertexIndex];

// 	//output data
// 	gl_Position = PushConstants.render_matrix *vec4(v.position, 1.0f);
// 	outColor = v.color.xyz;
// 	outUV.x = v.uv_x;
// 	outUV.y = v.uv_y;
// }