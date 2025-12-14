#include <d3d11.h>
#include <d3dcompiler.h>
#include <fstream>
#include <iostream>
#include <vector>

bool LoadFile(const char *filename, std::string &out)
{
    std::ifstream file(filename, std::ios::binary);
    if (!file)
        return false;
    file.seekg(0, std::ios::end);
    size_t size = file.tellg();
    out.resize(size);
    file.seekg(0);
    file.read(&out[0], size);
    return true;
}

bool SaveFile(const char *filename, const void *data, size_t size)
{
    std::ofstream file(filename, std::ios::binary);
    if (!file)
        return false;
    file.write(reinterpret_cast<const char *>(data), size);
    return true;
}

int main(int argc, char **argv)
{
    if (argc < 3)
    {
        std::cout << "Usage: compiler.exe input.hlsl output.cso\n";
        return 1;
    }

    const char *inputFile = argv[1];
    const char *outputFile = argv[2];

    std::string shaderCode;
    if (!LoadFile(inputFile, shaderCode))
    {
        std::cerr << "Failed to read input file.\n";
        return 1;
    }

    ID3DBlob *shaderBlob = nullptr;
    ID3DBlob *errorBlob = nullptr;

    UINT compileFlags = D3DCOMPILE_OPTIMIZATION_LEVEL3;

    HRESULT hr = D3DCompile(
        shaderCode.c_str(), shaderCode.size(),
        inputFile,
        nullptr,
        nullptr,
        "CSMain",
        "cs_5_0",
        compileFlags, 0,
        &shaderBlob,
        &errorBlob);

    if (FAILED(hr))
    {
        if (errorBlob)
        {
            std::cerr << "Shader compilation failed:\n";
            std::cerr << static_cast<const char *>(errorBlob->GetBufferPointer()) << "\n";
            errorBlob->Release();
        }
        else
        {
            std::cerr << "Unknown compilation error.\n";
        }
        return 1;
    }

    if (!SaveFile(outputFile, shaderBlob->GetBufferPointer(), shaderBlob->GetBufferSize()))
    {
        std::cerr << "Failed to write output file.\n";
        shaderBlob->Release();
        return 1;
    }

    std::cout << "Shader compiled successfully to " << outputFile << "\n";
    shaderBlob->Release();
    return 0;
}
