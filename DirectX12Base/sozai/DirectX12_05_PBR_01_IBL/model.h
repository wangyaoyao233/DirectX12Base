#pragma once



// �}�e���A���\����
struct MODEL_MATERIAL
{
	char						Name[256];
	//MATERIAL					Material;
	char						TextureName[256];
	//ID3D11ShaderResourceView*	Texture;

};


// �`��T�u�Z�b�g�\����
struct SUBSET
{
	unsigned int	StartIndex;
	unsigned int	IndexNum;
	MODEL_MATERIAL	Material;
};


// ���f���\����
struct MODEL
{
	Vertex3D		*VertexArray;
	unsigned int	VertexNum;

	unsigned int	*IndexArray;
	unsigned int	IndexNum;

	SUBSET			*SubsetArray;
	unsigned int	SubsetNum;
};





class CModel
{
private:

	ComPtr<ID3D12Resource>	m_VertexBuffer;
	ComPtr<ID3D12Resource>	m_IndexBuffer;
	unsigned int	m_VertexNum;
	unsigned int	m_IndexNum;


	SUBSET*	m_SubsetArray;
	unsigned int	m_SubsetNum;

	void LoadObj( const char *FileName, MODEL *Model );
	void LoadMaterial( const char *FileName, MODEL_MATERIAL **MaterialArray, unsigned int *MaterialNum );

public:

	void Draw(ID3D12GraphicsCommandList *CommandList);

	void Load( const char *FileName );
	void Unload();

};