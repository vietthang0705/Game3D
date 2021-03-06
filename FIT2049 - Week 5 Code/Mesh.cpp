/*	FIT2049 - Example Code
*	Mesh.cpp
*	Created by Elliott Wilson - 2015 - Monash Univeristy
*	Implementation of Mesh.h
*/

#include "Mesh.h"
#include <fstream>

using namespace std;

Mesh::Mesh()
{
	m_vertexBuffer = NULL;
	m_indexBuffer = NULL;
	m_meshShader = NULL;
	m_meshTexture = NULL;
}

Mesh::~Mesh()
{

}

bool Mesh::Initialise(Direct3D* direct3D, Shader* meshShader)
{
	Vertex* verts;				//A pointer to our Vertex array
	unsigned long* indices;		//A pointer to our index array

	if (!meshShader)			//If there is no shader, return false
		return false;

	m_meshShader = meshShader;	//Set the shader for this mesh

	m_vertexCount = 6;			//As we going to make a two sided triangle we need 6 verts
	m_indexCount = 6;			//And we going to draw each of them once so we need 6 indices

	verts = new Vertex[m_vertexCount];		//Allocate space for our verts
	if (!verts)								//Big models could run out of memory, we check for that
		return false;

	indices = new unsigned long[m_indexCount];	//Allocated space for our indices
	if (!indices)								//Big models could run out of memory, we check for that
		return false;

	//Fill out our vertex data!
	verts[0].position = Vector3(-1.0f, -1.0f, 0.0f);
	verts[0].colour = Color(0.0f, 1.0f, 0.0f, 1.0f);
	verts[0].normal = Vector3(0.0f, 0.0f, -1.0f);		//For the first triangle we have the normals facing out
	verts[0].texCoord = Vector2(0, 0);

	verts[1].position = Vector3(0.0f, 1.0f, 0.0f);
	verts[1].colour = Color(1.0f, 0.0f, 0.0f, 1.0f);
	verts[1].normal = Vector3(0.0f, 0.0f, -1.0f);
	verts[1].texCoord = Vector2(0, 0);

	verts[2].position = Vector3(1.0f, -1.0f, 0.0f);
	verts[2].colour = Color(0.0f, 0.0f, 1.0f, 1.0f);
	verts[2].normal = Vector3(0.0f, 0.0f, -1.0f);
	verts[2].texCoord = Vector2(0, 0);

	verts[3].position = Vector3(-1.0f, -1.0f, 0.01f);	//For the second triangle we move it back a small amount on the Z axis
	verts[3].colour = Color(0.0f, 1.0f, 0.0f, 1.0f);
	verts[3].normal = Vector3(0.0f, 0.0f, 1.0f);		//For the second triangle we have the normals facing in
	verts[3].texCoord = Vector2(0, 0);

	verts[4].position = Vector3(0.0f, 1.0f, 0.01f);
	verts[4].colour = Color(1.0f, 0.0f, 0.0f, 1.0f);
	verts[4].normal = Vector3(0.0f, 0.0f, 1.0f);
	verts[4].texCoord = Vector2(0, 0);

	verts[5].position = Vector3(1.0f, -1.0f, 0.01f);
	verts[5].colour = Color(0.0f, 0.0f, 1.0f, 1.0f);
	verts[5].normal = Vector3(0.0f, 0.0f, 1.0f);
	verts[5].texCoord = Vector2(0, 0);

	for (int i = 0; i < m_indexCount; i++)		//Fill out our indices!
	{
		indices[i] = i;
	}

	if (!InitialiseBuffers(direct3D, verts, indices))	//Now that we have our vertex and index data, we need to copy it into buffers
	{
		return false;
	}

	//Now that the buffers are created we can delete all of the data we loaded!
	delete[] verts;
	verts = 0;

	delete[] indices;
	indices = 0;

	return true;
}

bool Mesh::Initialise(Direct3D* direct3D, const char* filename, Color defaultColour, Texture* texture, Shader* meshShader)
{
	//A face is made of three points, each one containing a position, normal and text coord (uv)
	//This struct is used during the load to store this data
	struct Face
	{
		int vert1, normal1, uv1;
		int vert2, normal2, uv2;
		int vert3, normal3, uv3;
	};

    Vector3* verts;		//This array stores raw positional data from the OBJ file. 
    Vector3* normals;	//This array stores the raw normals from the OBJ file
    Vector2* uvs;		//This array stores the raw texture coords from the OBJ file
    Face* faces;		//This is an array of Face structs that collects the different 
						//vert, normal and uv indices into a single place
    
    int vertexCount = 0;	//We will work out the total number of verts in the file
	int normalCount = 0;	//The total number of normals	
	int textureCount = 0;	//The total number of texture coords
	int faceCount = 0;		//The total number of faces

    int vertIndex = 0;		//These are used to keep track of where we are upto in our data arrays
    int normalIndex = 0;
    int uvIndex = 0;
    int faceIndex = 0;
    
	ifstream fileIn;		//OBJ files are just text files so we use a simple input file stream


	if (!meshShader)		//The shader for the mesh cannot be NULL
		return false;

	m_meshShader = meshShader;	//Each mesh is rendered using a shader, we set that shader here
	m_meshTexture = texture;	//Set the texture for the mesh
	m_originalTexture = texture;

	//The first read of the file is just to determine the number of verts, normals and uvs in the file
	fileIn.open(filename);		//Open our OBJ file for reading
	
	if (!fileIn.good())			//If we can't open it return false
		return false;

	char input;					//We're going to read through char by char, each char we read will go in here
	while (!fileIn.eof())		//Keep reading until the end of the file
	{
		fileIn.get(input);		//Read the first char on the line

		if (input == 'v')		//If the line starts with v then we...
		{
			fileIn.get(input);	//...want to chack the next char after it...
			if (input == ' ') { vertexCount++; }	//...if it's a blank space then we have a vertex position!
			if (input == 't') { textureCount++; }	//...if it's a 't' then we have a texture coord!
			if (input == 'n') { normalCount++; }	//...if it's an 'n' then we have a normal!
		}

		if (input == 'f')		//If the first char of the line is 'f' then it's a face
		{
			fileIn.get(input);
			if (input == ' ') { faceCount++; }
		}

		while (input != '\n')	//Now that we have checked this line, we'll skip to the end of it.
		{
			fileIn.get(input);
		}
	}
	fileIn.close();		//After reading the counts we then close the file to reset it

	//Now that we know how much memory to allocate we can read the Data from the OBJ file
	fileIn.open(filename);
	if (!fileIn.good())
		return false;

	//Allocate our arrys
	verts = new Vector3[vertexCount];
	normals = new Vector3[normalCount];
	uvs = new Vector2[textureCount];
	faces = new Face[faceCount];

	while (!fileIn.eof())		//We will read the whole file again
	{
		fileIn.get(input);		//Char by Char
		if (input == 'v')		//If it is starts with v...
		{
			fileIn.get(input);

			if (input == ' ')	//...and is a vert position...
			{
				//...then read from the file in to the vert array at vertIndex...
				fileIn >> verts[vertIndex].x >> verts[vertIndex].y >> verts[vertIndex].z;
				vertIndex++;	//...and increase the vertIndex
			}
			if (input == 't')	//...and is a tex coord...
			{
				//...then read from the file in to the uv array at uvIndex...
				fileIn >> uvs[uvIndex].x >> uvs[uvIndex].y;
				uvs[uvIndex].x = -uvs[uvIndex].x;
				uvIndex++;	//...and increase the uvIndex
			}
			if (input == 'n')	//...and is a normal...
			{
				//...then read from the file in to the normal array at normalIndex...
				fileIn >> normals[normalIndex].x >> normals[normalIndex].y >> normals[normalIndex].z;
				normalIndex++;	//...and increase the normalIndex
			}
		}
		if (input == 'f')	//If we're reading a line describing a face then we need to fill out one of the Face structs
		{
			char junk;		//The face lines are in the format of "index/index/index" which means that the / chars are junk

			//Here you can see us reading three verts worth of information for each face. When we expect a / char we put it into "junk"
			//(NOTE: OBJ can support faces with more then 3 points, we only read for 3. This means that our models must be made up of
			//		 only triangles (i.e. they must be "triangulated"))
			fileIn >> faces[faceIndex].vert1 >> junk >> faces[faceIndex].uv1 >> junk >> faces[faceIndex].normal1
				   >> faces[faceIndex].vert2 >> junk >> faces[faceIndex].uv2 >> junk >> faces[faceIndex].normal2
				   >> faces[faceIndex].vert3 >> junk >> faces[faceIndex].uv3 >> junk >> faces[faceIndex].normal3;
			faceIndex++;	//After we read a face we increase the face index
		}

		while (input != '\n')	//Now that we have checked this line, we'll skip to the end of it.
		{
			fileIn.get(input);
		}
	}
	fileIn.close();		//Close the file, we are done with our file IO now

	//Each face is made up of three verts so the total number of indices is 3 * the number of faces
	m_indexCount = faceCount * 3;
	//We will also allocate the same number of vertices
	m_vertexCount = faceCount * 3;
	//(NOTE: There is a bit of an inefficiency in this loading code, the number of vertices is the same of the number of indices
	//		 this means that there is a lot of repeated vertex data. It's difficult because so position have different normals which
	//		 means you need to duplicate some vertices but not others.
	//		 This could be optimised by working out the final list of unique verts and creating an index buffer drawing them in the right order.)

	Vertex* vertexData = new Vertex[m_vertexCount];		//We'll allocate our vertex memory
	if (!vertexData)									//Big models could run out memory, we check for that
		return false;

	int vertexCounter = 0;
	for (int i = 0; i < faceCount; i++)					//For each face
	{
		int vertIndex = faces[i].vert1 - 1;				//Look up the vertex index,
		int normIndex = faces[i].normal1 - 1;			//normal index
		int uvIndex = faces[i].uv1 - 1;					//and uv index for the first point in the face

		vertexData[vertexCounter].position = verts[vertIndex];		//Fill out our vertex with the correct vert,
		vertexData[vertexCounter].colour = defaultColour;			//colour (this was a method parameter),
		vertexData[vertexCounter].texCoord = uvs[uvIndex];			//uv,
		vertexData[vertexCounter].normal = normals[normIndex];		//and normal data
		vertexCounter++;

		vertIndex = faces[i].vert2 - 1;		//Move to the next point
		normIndex = faces[i].normal2 - 1;	
		uvIndex = faces[i].uv2 - 1;

		vertexData[vertexCounter].position = verts[vertIndex];	//and read the data for that point
		vertexData[vertexCounter].colour = defaultColour;
		vertexData[vertexCounter].texCoord = uvs[uvIndex];
		vertexData[vertexCounter].normal = normals[normIndex];
		vertexCounter++;

		vertIndex = faces[i].vert3 - 1;		//Move to the last point
		normIndex = faces[i].normal3 - 1;
		uvIndex = faces[i].uv3 - 1;

		vertexData[vertexCounter].position = verts[vertIndex];	//and read the last point for the face
		vertexData[vertexCounter].colour = defaultColour;
		vertexData[vertexCounter].texCoord = uvs[uvIndex];
		vertexData[vertexCounter].normal = normals[normIndex];
		vertexCounter++;
	}
	
	unsigned long* indexData = new unsigned long[m_indexCount];		//Allocate our index buffer
	if (!indexData)													//Big models could run out of memory, we check for that
		return false;

	for (int i = 0; i < m_indexCount; i++)		//Fill it out as just a linear array of numbers (NOTE: as mentioned above this could be optimised!)
	{
		indexData[i] = i;
	}

	if (!InitialiseBuffers(direct3D, vertexData, indexData))	//Now that we have our vertex and index data, we need to copy it into buffers
	{
		return false;
	}

	//Now that the buffers are created we can delete all of the data we loaded!
	if (verts)
	{
		delete[] verts;
		verts = NULL;
	}

	if (normals)
	{
		delete[] normals;
		normals = NULL;
	}
	
	if (uvs)
	{
		delete[] uvs;
		uvs = NULL;
	}

	if (faces)
	{
		delete[] faces;
		faces = NULL;
	}

	if (vertexData)
	{
		delete[] vertexData;
		vertexData = NULL;
	}

	if (indexData)
	{
		delete[] indexData;
		indexData = NULL;
	}

	return true;
}

bool Mesh::InitialiseBuffers(Direct3D* direct3D, Vertex* vertexData, unsigned long* indexData)
{
	//Here we need to create and initialise a Direct3D buffer to hold and vertices and one to hold our indices

	D3D11_BUFFER_DESC vertexBufferDescription;		//Each buffer has information (stored in a struct) describing the buffer itself...
	D3D11_BUFFER_DESC indexBufferDescription;		
	D3D11_SUBRESOURCE_DATA vertexDataDescription;	//...and information describing the data that needs to be copied into it,...
	D3D11_SUBRESOURCE_DATA indexDataDescription;	//...these structs are then passed to the Create buffer method so that Direct3D can allocate things correctly

	vertexBufferDescription.Usage = D3D11_USAGE_DEFAULT;					//The buffer description struct needs to know what the buffer will be used for...
	vertexBufferDescription.ByteWidth = sizeof(Vertex) * m_vertexCount;		//...how big the buffer should be in bytes (for us it is the size of one vertex * number of verts)...
	vertexBufferDescription.BindFlags = D3D11_BIND_VERTEX_BUFFER;			//...what the buffer is for (ours is a vertex buffer)...
	vertexBufferDescription.CPUAccessFlags = 0;								//...the rest a misc setting for various things that we won't need to worry about.
	vertexBufferDescription.MiscFlags = 0;									//If you want more info look up D3D11_BUFFER_DESC on the MSDN
	vertexBufferDescription.StructureByteStride = 0;

	vertexDataDescription.pSysMem = vertexData;		//The most important piece of information for the data description struct is a pointer 
													//to the system memory that we are coping into the buffer
	vertexDataDescription.SysMemPitch = 0;			//These values allow us to shift and offset the data but we won't be needing that!
	vertexDataDescription.SysMemSlicePitch = 0;

	//After we have the buffer and data descriptions complete we ask our device to create the buffer for us. If successful the buffer will be availabe through m_vertexBuffer!
	if (FAILED(direct3D->GetDevice()->CreateBuffer(&vertexBufferDescription, &vertexDataDescription, &m_vertexBuffer)))
	{
		return false;
	}

	//Creating the index buffer is pretty much the same as the vertex buffer
	indexBufferDescription.Usage = D3D11_USAGE_DEFAULT;
	indexBufferDescription.ByteWidth = sizeof(unsigned long) * m_indexCount;
	indexBufferDescription.BindFlags = D3D11_BIND_INDEX_BUFFER;
	indexBufferDescription.CPUAccessFlags = 0;
	indexBufferDescription.MiscFlags = 0;
	indexBufferDescription.StructureByteStride = 0;

	indexDataDescription.pSysMem = indexData;
	indexDataDescription.SysMemPitch = 0;
	indexDataDescription.SysMemSlicePitch = 0;

	if (FAILED(direct3D->GetDevice()->CreateBuffer(&indexBufferDescription, &indexDataDescription, &m_indexBuffer)))
	{
		return false;
	}

	return true;
}

void Mesh::Render(Direct3D* direct3D, Matrix world, Camera* cam)
{
	//To render a mesh we need to set the buffers that we created as the current rendering buffers and tell the shader to render them.
	
	unsigned int stride = sizeof(Vertex);	//We need to specify how many btyes to move through the vertex buffer each time.
	unsigned int offset = 0;				//We could also start at somewhere other then the start of the buffer if we liked

	direct3D->GetDeviceContext()->IASetVertexBuffers(0, 1, &m_vertexBuffer, &stride, &offset);		//This says the we are rendering from our vertex buffer
	direct3D->GetDeviceContext()->IASetIndexBuffer(m_indexBuffer, DXGI_FORMAT_R32_UINT, 0);			//and from our index buffer
	direct3D->GetDeviceContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);	//and that what we are rendering is a list of individual triangles

	if (direct3D->GetCurrentShader() != m_meshShader)		//To avoid wasting time setting the shader multiple times
	{														//we check to see if the last used shader is our shader
		m_meshShader->Begin(direct3D->GetDeviceContext());	//If it isn't then we set the shader
		direct3D->SetCurrentShader(m_meshShader);			//Doing this means that if we draw things using the same shader one after each other then we should 
	}														//see an increase in performance, as it is quite slow to set the vertex and pixel shaders

	m_meshShader->SetConstants(direct3D->GetDeviceContext(), world, cam->GetView(), cam->GetProjection());	//The shader needs to know the world matirx to use to draw the
																											//mesh as well as the view and projection matrices from the camera
	
	if (m_meshTexture)		//If there is a texture to use then set it in the shader
		m_meshShader->SetTexture(direct3D->GetDeviceContext(), m_meshTexture->GetShaderResourceView());

	direct3D->GetDeviceContext()->DrawIndexed(m_indexCount, 0, 0);		//Once the buffers, shaders and matrices are set then we are ready to render,
																		//we tell Direct3D how many indices we want to render
}

void Mesh::Release()
{
	if (m_indexBuffer)
	{
		m_indexBuffer->Release();
		m_indexBuffer = NULL;
	}

	if (m_vertexBuffer)
	{
		m_vertexBuffer->Release();
		m_vertexBuffer = NULL;
	}
}

void Mesh::setHitTexture (Texture* hitTexture) {
	m_hitTexture = hitTexture;
}

void Mesh::changeToHitTex() {
	m_meshTexture = m_hitTexture;
}

void Mesh::changeToOriTex() {
	m_meshTexture = m_originalTexture;
}