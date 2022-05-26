#shader vertex
#version 450 core                        
struct vertex {                          
   vec4 position;                        
   vec4 color;                           
};                                       
layout(std430) buffer my_vertices { 
	vertex vertices[];                    
};                                       
out VS_OUT {                             
   vec4 color;						      
} vs_out; 

layout(binding = 1) uniform TransformBlock {                 
	mat4 translation;             // 16     0         0,16,32,48       , binding = 1
	mat4 rotation;                // 16     64        64,80,96,112
	mat4 projection_matrix;       // 16     128       128,144,160,176
};  

void main(void) {                        
   mat4 mvp = projection_matrix * translation * rotation ; 
   gl_Position = mvp * vertices[gl_VertexID].position;     
   vs_out.color = vertices[gl_VertexID].color;                 
}   

#shader fragment
#version 450 core       
                        

in VS_OUT{              
   vec4 color;          
} fs_in;                
out vec4 color;         
                        
void main(void){        
   color = fs_in.color; 
}                       