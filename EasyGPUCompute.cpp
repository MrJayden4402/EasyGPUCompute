#include "EasyGPUCompute.h"

ID3D11Device *EasyGPUComputeD3D11Device = nullptr;
ID3D11DeviceContext *EasyGPUComputeD3D11DeviceContest = nullptr;

vector<char> __Easy_LoadCSO(const char *filename)
{
    std::ifstream file(filename, std::ios::binary);
    return std::vector<char>((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
}

void EasyStartUpGPUCompute()
{
    D3D_FEATURE_LEVEL level;
    D3D11CreateDevice(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, 0,
                      nullptr, 0, D3D11_SDK_VERSION, &EasyGPUComputeD3D11Device, &level, &EasyGPUComputeD3D11DeviceContest);
}

void EasyReleaseGPUCompute()
{
    EasyGPUComputeD3D11DeviceContest->Release();
    EasyGPUComputeD3D11Device->Release();
}

ID3D11ComputeShader *CreateID3D11ComputeShader(vector<char> &cso)
{
    ID3D11ComputeShader *computeShader = nullptr;
    EasyGPUComputeD3D11Device->CreateComputeShader(cso.data(), cso.size(), nullptr, &computeShader);
    return computeShader;
}

EasyGPUCSOCode LoadCSOFromFile(string filename)
{
    auto cso = __Easy_LoadCSO(filename.c_str());
    return CreateID3D11ComputeShader(cso);
}

template <typename DATA_TYPE, typename VECTOR_TYPE>
ID3D11Buffer *CreateID3D11GPUBuffer(VECTOR_TYPE &&input, UINT count)
{
    // 创建 GPU buffer
    D3D11_BUFFER_DESC bufDesc = {};
    bufDesc.ByteWidth = sizeof(DATA_TYPE) * count;
    bufDesc.Usage = D3D11_USAGE_DEFAULT;
    bufDesc.BindFlags = D3D11_BIND_UNORDERED_ACCESS | D3D11_BIND_SHADER_RESOURCE;
    bufDesc.StructureByteStride = sizeof(DATA_TYPE);
    bufDesc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;

    D3D11_SUBRESOURCE_DATA initData = {};
    initData.pSysMem = input.data();

    ID3D11Buffer *gpuBuffer = nullptr;
    EasyGPUComputeD3D11Device->CreateBuffer(&bufDesc, &initData, &gpuBuffer);
    return gpuBuffer;
}

ID3D11UnorderedAccessView *CreateID3D11UnorderedAccessView(ID3D11Buffer *gpuBuffer, UINT count, UINT UAVIndex)
{
    // 创建 UAV
    D3D11_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
    uavDesc.Format = DXGI_FORMAT_UNKNOWN;
    uavDesc.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;
    uavDesc.Buffer.NumElements = count;

    ID3D11UnorderedAccessView *uav = nullptr;
    EasyGPUComputeD3D11Device->CreateUnorderedAccessView(gpuBuffer, &uavDesc, &uav);
    EasyGPUComputeD3D11DeviceContest->CSSetUnorderedAccessViews(UAVIndex, 1, &uav, nullptr);
    return uav;
}

void EasyDispatchComputeShader(ID3D11ComputeShader *computeShader, UINT ThreadGroupCountX, UINT ThreadGroupCountY, UINT ThreadGroupCountZ)
{
    // 设置着色器和资源
    EasyGPUComputeD3D11DeviceContest->CSSetShader(computeShader, nullptr, 0);

    // Dispatch 线程组
    EasyGPUComputeD3D11DeviceContest->Dispatch(ThreadGroupCountX, ThreadGroupCountY, ThreadGroupCountZ);
}

template <typename DATA_TYPE>
void __Easy_ReadResult(ID3D11Buffer *gpuBuffer, UINT count, vector<DATA_TYPE> &output)
{
    // 创建 staging buffer 读取结果
    D3D11_BUFFER_DESC bufDesc = {};
    bufDesc.ByteWidth = sizeof(DATA_TYPE) * count;
    bufDesc.StructureByteStride = sizeof(DATA_TYPE);
    bufDesc.Usage = D3D11_USAGE_STAGING;
    bufDesc.BindFlags = 0;
    bufDesc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
    bufDesc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;

    ID3D11Buffer *readbackBuffer = nullptr;
    EasyGPUComputeD3D11Device->CreateBuffer(&bufDesc, nullptr, &readbackBuffer);

    EasyGPUComputeD3D11DeviceContest->CopyResource(readbackBuffer, gpuBuffer);

    // 映射结果
    D3D11_MAPPED_SUBRESOURCE mapped;
    EasyGPUComputeD3D11DeviceContest->Map(readbackBuffer, 0, D3D11_MAP_READ, 0, &mapped);

    DATA_TYPE *result = reinterpret_cast<DATA_TYPE *>(mapped.pData);

    output.clear();

    for (int i = 0; i < count; ++i)
        output.push_back(result[i]);

    EasyGPUComputeD3D11DeviceContest->Unmap(readbackBuffer, 0);
    readbackBuffer->Release();
}

template <typename DATA_VEC>
void EasyGPUBuffer::SetData(DATA_VEC &&data, UINT UAVIndex)
{
    if (data.size() == 0)
        return;
    count = data.size();
    this->gpuBuffer = CreateID3D11GPUBuffer<decltype(data[0])>(std::forward<DATA_VEC>(data), count);
    this->uav = CreateID3D11UnorderedAccessView(gpuBuffer, count, UAVIndex);
}

template <typename DATA_TYPE>
void EasyGPUBuffer::ReadData(vector<DATA_TYPE> &data)
{
    __Easy_ReadResult(gpuBuffer, count, data);
}

void EasyGPUBuffer::Release()
{
    gpuBuffer->Release();
    uav->Release();
}

template <typename DATA_TYPE>
void EasyGPUConstantBuffer<DATA_TYPE>::StartUp(UINT UAVIndex)
{
    D3D11_BUFFER_DESC cbDesc = {};
    cbDesc.ByteWidth = sizeof(DATA_TYPE);
    cbDesc.Usage = D3D11_USAGE_DEFAULT;
    cbDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    cbDesc.CPUAccessFlags = 0;

    EasyGPUComputeD3D11Device->CreateBuffer(&cbDesc, nullptr, &pConstantBuffer);

    this->UAVIndex = UAVIndex;
}

template <typename DATA_TYPE>
void EasyGPUConstantBuffer<DATA_TYPE>::UpdateData(DATA_TYPE data)
{
    EasyGPUComputeD3D11DeviceContest->UpdateSubresource(pConstantBuffer, 0, nullptr, &data, 0, 0);

    EasyGPUComputeD3D11DeviceContest->CSSetConstantBuffers(UAVIndex, 1, &pConstantBuffer);
}

template <typename DATA_TYPE>
void EasyGPUConstantBuffer<DATA_TYPE>::Release()
{
    pConstantBuffer->Release();
}
