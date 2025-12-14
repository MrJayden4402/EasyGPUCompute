#include <d3d11.h>
#include <d3dcompiler.h>
#include <iostream>
#include <vector>
#include <fstream>

using namespace std;

typedef ID3D11ComputeShader *EasyGPUCSOCode;

extern ID3D11Device *EasyGPUComputeD3D11Device;
extern ID3D11DeviceContext *EasyGPUComputeD3D11DeviceContest;

void EasyStartUpGPUCompute();
void EasyReleaseGPUCompute();

vector<char> __Easy_LoadCSO(const char *filename);
EasyGPUCSOCode LoadCSOFromFile(string filename);

ID3D11ComputeShader *CreateID3D11ComputeShader(vector<char> &cso);

template <typename DATA_TYPE, typename VECTOR_TYPE>
ID3D11Buffer *CreateID3D11GPUBuffer(VECTOR_TYPE &&input, UINT count);

ID3D11UnorderedAccessView *CreateID3D11UnorderedAccessView(ID3D11Buffer *gpuBuffer, UINT count, UINT UAVIndex);

void EasyDispatchComputeShader(ID3D11ComputeShader *computeShader, UINT ThreadGroupCountX, UINT ThreadGroupCountY, UINT ThreadGroupCountZ);

template <typename DATA_TYPE>
void __Easy_ReadResult(ID3D11Buffer *gpuBuffer, UINT count, vector<DATA_TYPE> &output);

class EasyGPUBuffer
{
private:
    ID3D11Buffer *gpuBuffer = nullptr;
    ID3D11UnorderedAccessView *uav = nullptr;
    int count = 0;

public:
    template <typename DATA_VEC>
    void SetData(DATA_VEC &&data, UINT UAVIndex);

    template <typename DATA_TYPE>
    void ReadData(vector<DATA_TYPE> &data);

    void Release();
};

template <typename DATA_TYPE>
class EasyGPUConstantBuffer
{
private:
    ID3D11Buffer *pConstantBuffer = nullptr;
    UINT UAVIndex = 0;

public:
    void StartUp(UINT UAVIndex);

    void UpdateData(DATA_TYPE data);

    void Release();
};