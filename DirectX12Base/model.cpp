
#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <shlwapi.h>
#pragma comment(lib, "shlwapi.lib")

#include "main.h"
#include "renderer.h"
#include "model.h"



void CModel::Draw(ID3D12GraphicsCommandList* CommandList)
{


	//頂点バッファ設定
	D3D12_VERTEX_BUFFER_VIEW vertexView{};
	vertexView.BufferLocation = m_VertexBuffer->GetGPUVirtualAddress();
	vertexView.StrideInBytes = sizeof(Vertex3D);
	vertexView.SizeInBytes = sizeof(Vertex3D) * m_VertexNum;
	CommandList->IASetVertexBuffers(0, 1, &vertexView);


	//インデックスバッファ設定
	D3D12_INDEX_BUFFER_VIEW indexView{};
	indexView.BufferLocation = m_IndexBuffer->GetGPUVirtualAddress();
	indexView.SizeInBytes = sizeof(unsigned int) * m_IndexNum;
	indexView.Format = DXGI_FORMAT_R32_UINT;
	CommandList->IASetIndexBuffer(&indexView);


	//トポロジ設定
	CommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	for (unsigned int i = 0; i < m_SubsetNum; i++)
	{
		// マテリアル設定
		//CRenderer::SetMaterial( m_SubsetArray[i].Material.Material );

		// テクスチャ設定
		//CRenderer::GetDeviceContext()->PSSetShaderResources( 0, 1, &m_SubsetArray[i].Material.Texture );

		// ポリゴン描画
		//CRenderer::GetDeviceContext()->DrawIndexed( m_SubsetArray[i].IndexNum, m_SubsetArray[i].StartIndex, 0 );
		CommandList->DrawIndexedInstanced(m_SubsetArray[i].IndexNum, 1, m_SubsetArray[i].StartIndex, 0, 0);

	}

}



void CModel::Load(const char* FileName)
{

	MODEL model;
	LoadObj(FileName, &model);



	ComPtr<ID3D12Device> device = CRenderer::GetInstance()->GetDevice();

	HRESULT hr;
	D3D12_HEAP_PROPERTIES heapProperties{};
	D3D12_RESOURCE_DESC   resourceDesc{};

	heapProperties.Type = D3D12_HEAP_TYPE_UPLOAD;
	heapProperties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	heapProperties.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
	heapProperties.CreationNodeMask = 0;
	heapProperties.VisibleNodeMask = 0;

	resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	resourceDesc.Height = 1;
	resourceDesc.DepthOrArraySize = 1;
	resourceDesc.MipLevels = 1;
	resourceDesc.Format = DXGI_FORMAT_UNKNOWN;
	resourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
	resourceDesc.SampleDesc.Count = 1;
	resourceDesc.SampleDesc.Quality = 0;




	//頂点バッファの作成
	m_VertexNum = model.VertexNum;
	resourceDesc.Width = sizeof(Vertex3D) * m_VertexNum;
	hr = device->CreateCommittedResource(&heapProperties, D3D12_HEAP_FLAG_NONE, &resourceDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&m_VertexBuffer));
	assert(SUCCEEDED(hr));

	Vertex3D* vertex;
	hr = m_VertexBuffer->Map(0, nullptr, (void**)&vertex);
	assert(SUCCEEDED(hr));

	memcpy(vertex, model.VertexArray, sizeof(Vertex3D) * m_VertexNum);

	m_VertexBuffer->Unmap(0, nullptr);




	//インデックスバッファの作成
	m_IndexNum = model.IndexNum;
	resourceDesc.Width = sizeof(unsigned int) * m_IndexNum;
	hr = device->CreateCommittedResource(&heapProperties, D3D12_HEAP_FLAG_NONE, &resourceDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&m_IndexBuffer));
	assert(SUCCEEDED(hr));

	unsigned int* index;
	hr = m_IndexBuffer->Map(0, nullptr, (void**)&index);
	assert(SUCCEEDED(hr));

	memcpy(index, model.IndexArray, sizeof(unsigned int) * m_IndexNum);

	m_IndexBuffer->Unmap(0, nullptr);





	// サブセット設定
	{
		m_SubsetArray = new SUBSET[model.SubsetNum];
		m_SubsetNum = model.SubsetNum;

		for (unsigned int i = 0; i < model.SubsetNum; i++)
		{
			m_SubsetArray[i].StartIndex = model.SubsetArray[i].StartIndex;
			m_SubsetArray[i].IndexNum = model.SubsetArray[i].IndexNum;
			/*
						m_SubsetArray[i].Material.Material = model.SubsetArray[i].Material.Material;

						m_SubsetArray[i].Material.Texture = NULL;

						D3DX11CreateShaderResourceViewFromFile(CRenderer::GetDevice(),
							model.SubsetArray[i].Material.TextureName,
							NULL,
							NULL,
							&m_SubsetArray[i].Material.Texture,
							NULL);

						assert(m_SubsetArray[i].Material.Texture);
			*/
		}
	}

	delete[] model.VertexArray;
	delete[] model.IndexArray;
	delete[] model.SubsetArray;

}





void CModel::Unload()
{
	/*
		m_VertexBuffer->Release();
		m_IndexBuffer->Release();

		for (unsigned int i = 0; i < m_SubsetNum; i++)
		{
			m_SubsetArray[i].Material.Texture->Release();
		}
	*/
	delete[] m_SubsetArray;

}





//モデル読込////////////////////////////////////////////
void CModel::LoadObj(const char* FileName, MODEL* Model)
{

	char dir[MAX_PATH];
	strcpy(dir, FileName);
	PathRemoveFileSpec(dir);





	XMFLOAT3* positionArray;
	XMFLOAT3* normalArray;
	XMFLOAT2* texcoordArray;

	unsigned int	positionNum = 0;
	unsigned int	normalNum = 0;
	unsigned int	texcoordNum = 0;
	unsigned int	vertexNum = 0;
	unsigned int	indexNum = 0;
	unsigned int	in = 0;
	unsigned int	subsetNum = 0;

	MODEL_MATERIAL* materialArray = NULL;
	unsigned int	materialNum = 0;

	char str[256];
	char* s;
	char c;


	FILE* file;
	file = fopen(FileName, "rt");
	assert(file);



	//要素数カウント
	while (true)
	{
		fscanf(file, "%s", str);

		if (feof(file) != 0)
			break;

		if (strcmp(str, "v") == 0)
		{
			positionNum++;
		}
		else if (strcmp(str, "vn") == 0)
		{
			normalNum++;
		}
		else if (strcmp(str, "vt") == 0)
		{
			texcoordNum++;
		}
		else if (strcmp(str, "usemtl") == 0)
		{
			subsetNum++;
		}
		else if (strcmp(str, "f") == 0)
		{
			in = 0;

			do
			{
				fscanf(file, "%s", str);
				vertexNum++;
				in++;
				c = fgetc(file);
			} while (c != '\n' && c != '\r');

			//四角は三角に分割
			if (in == 4)
				in = 6;

			indexNum += in;
		}
	}


	//メモリ確保
	positionArray = new XMFLOAT3[positionNum];
	normalArray = new XMFLOAT3[normalNum];
	texcoordArray = new XMFLOAT2[texcoordNum];


	Model->VertexArray = new Vertex3D[vertexNum];
	Model->VertexNum = vertexNum;

	Model->IndexArray = new unsigned int[indexNum];
	Model->IndexNum = indexNum;

	Model->SubsetArray = new SUBSET[subsetNum];
	Model->SubsetNum = subsetNum;




	//要素読込
	XMFLOAT3* position = positionArray;
	XMFLOAT3* normal = normalArray;
	XMFLOAT2* texcoord = texcoordArray;

	unsigned int vc = 0;
	unsigned int ic = 0;
	unsigned int sc = 0;


	fseek(file, 0, SEEK_SET);

	while (true)
	{
		fscanf(file, "%s", str);

		if (feof(file) != 0)
			break;

		if (strcmp(str, "mtllib") == 0)
		{
			//マテリアルファイル
			fscanf(file, "%s", str);

			char path[256];
			strcpy(path, dir);
			strcat(path, "\\");
			strcat(path, str);

			LoadMaterial(path, &materialArray, &materialNum);
		}
		else if (strcmp(str, "o") == 0)
		{
			//オブジェクト名
			fscanf(file, "%s", str);
		}
		else if (strcmp(str, "v") == 0)
		{
			//頂点座標
			fscanf(file, "%f", &position->x);
			fscanf(file, "%f", &position->y);
			fscanf(file, "%f", &position->z);
			position++;
		}
		else if (strcmp(str, "vn") == 0)
		{
			//法線
			fscanf(file, "%f", &normal->x);
			fscanf(file, "%f", &normal->y);
			fscanf(file, "%f", &normal->z);
			normal++;
		}
		else if (strcmp(str, "vt") == 0)
		{
			//テクスチャ座標
			fscanf(file, "%f", &texcoord->x);
			fscanf(file, "%f", &texcoord->y);
			texcoord->y = 1.0f - texcoord->y;
			texcoord++;
		}
		else if (strcmp(str, "usemtl") == 0)
		{
			//マテリアル
			fscanf(file, "%s", str);

			if (sc != 0)
				Model->SubsetArray[sc - 1].IndexNum = ic - Model->SubsetArray[sc - 1].StartIndex;

			Model->SubsetArray[sc].StartIndex = ic;


			for (unsigned int i = 0; i < materialNum; i++)
			{
				if (strcmp(str, materialArray[i].Name) == 0)
				{
					//Model->SubsetArray[ sc ].Material.Material = materialArray[i].Material;
					strcpy(Model->SubsetArray[sc].Material.TextureName, materialArray[i].TextureName);
					strcpy(Model->SubsetArray[sc].Material.Name, materialArray[i].Name);

					break;
				}
			}

			sc++;

		}
		else if (strcmp(str, "f") == 0)
		{
			//面
			in = 0;

			do
			{
				fscanf(file, "%s", str);

				s = strtok(str, "/");
				Model->VertexArray[vc].Position = positionArray[atoi(s) - 1];
				if (s[strlen(s) + 1] != '/')
				{
					//テクスチャ座標が存在しない場合もある
					s = strtok(NULL, "/");
					Model->VertexArray[vc].TexCoord = texcoordArray[atoi(s) - 1];
				}
				s = strtok(NULL, "/");
				Model->VertexArray[vc].Normal = normalArray[atoi(s) - 1];

				//Model->VertexArray[vc].Diffuse = D3DXVECTOR4( 1.0f, 1.0f, 1.0f, 1.0f );

				Model->IndexArray[ic] = vc;
				ic++;
				vc++;

				in++;
				c = fgetc(file);
			} while (c != '\n' && c != '\r');

			//四角は三角に分割
			if (in == 4)
			{
				Model->IndexArray[ic] = vc - 4;
				ic++;
				Model->IndexArray[ic] = vc - 2;
				ic++;
			}
		}
	}


	if (sc != 0)
		Model->SubsetArray[sc - 1].IndexNum = ic - Model->SubsetArray[sc - 1].StartIndex;





	delete[] positionArray;
	delete[] normalArray;
	delete[] texcoordArray;
	//delete[] materialArray;
}




//マテリアル読み込み///////////////////////////////////////////////////////////////////
void CModel::LoadMaterial(const char* FileName, MODEL_MATERIAL** MaterialArray, unsigned int* MaterialNum)
{
	/*
		char dir[MAX_PATH];
		strcpy(dir, FileName);
		PathRemoveFileSpec(dir);



		char str[256];

		FILE *file;
		file = fopen( FileName, "rt" );
		assert(file);

		MODEL_MATERIAL *materialArray;
		unsigned int materialNum = 0;

		//要素数カウント
		while( true )
		{
			fscanf( file, "%s", str );

			if( feof( file ) != 0 )
				break;


			if( strcmp( str, "newmtl" ) == 0 )
			{
				materialNum++;
			}
		}


		//メモリ確保
		materialArray = new MODEL_MATERIAL[ materialNum ];


		//要素読込
		int mc = -1;

		fseek( file, 0, SEEK_SET );

		while( true )
		{
			fscanf( file, "%s", str );

			if( feof( file ) != 0 )
				break;


			if( strcmp( str, "newmtl" ) == 0 )
			{
				//マテリアル名
				mc++;
				fscanf( file, "%s", materialArray[ mc ].Name );
				strcpy( materialArray[ mc ].TextureName, "" );

				materialArray[mc].Material.Emission.r = 0.0f;
				materialArray[mc].Material.Emission.g = 0.0f;
				materialArray[mc].Material.Emission.b = 0.0f;
				materialArray[mc].Material.Emission.a = 0.0f;
			}
			else if( strcmp( str, "Ka" ) == 0 )
			{
				//アンビエント
				fscanf( file, "%f", &materialArray[ mc ].Material.Ambient.r );
				fscanf( file, "%f", &materialArray[ mc ].Material.Ambient.g );
				fscanf( file, "%f", &materialArray[ mc ].Material.Ambient.b );
				materialArray[ mc ].Material.Ambient.a = 1.0f;
			}
			else if( strcmp( str, "Kd" ) == 0 )
			{
				//ディフューズ
				fscanf( file, "%f", &materialArray[ mc ].Material.Diffuse.r );
				fscanf( file, "%f", &materialArray[ mc ].Material.Diffuse.g );
				fscanf( file, "%f", &materialArray[ mc ].Material.Diffuse.b );
				materialArray[ mc ].Material.Diffuse.a = 1.0f;
			}
			else if( strcmp( str, "Ks" ) == 0 )
			{
				//スペキュラ
				fscanf( file, "%f", &materialArray[ mc ].Material.Specular.r );
				fscanf( file, "%f", &materialArray[ mc ].Material.Specular.g );
				fscanf( file, "%f", &materialArray[ mc ].Material.Specular.b );
				materialArray[ mc ].Material.Specular.a = 1.0f;
			}
			else if( strcmp( str, "Ns" ) == 0 )
			{
				//スペキュラ強度
				fscanf( file, "%f", &materialArray[ mc ].Material.Shininess );
			}
			else if( strcmp( str, "d" ) == 0 )
			{
				//アルファ
				fscanf( file, "%f", &materialArray[ mc ].Material.Diffuse.a );
			}
			else if( strcmp( str, "map_Kd" ) == 0 )
			{
				//テクスチャ
				fscanf( file, "%s", str );

				char path[256];
				strcpy( path, dir );
				strcat( path, "\\" );
				strcat( path, str );

				strcat( materialArray[ mc ].TextureName, path );
			}
		}


		*MaterialArray = materialArray;
		*MaterialNum = materialNum;
	*/
}

