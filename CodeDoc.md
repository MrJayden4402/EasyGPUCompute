# EasyGPUCompute 1.0 代码文档

## 1. 库概述

该库是基于DirectX实现的GPU计算库，封装了DirectX 11的API，提供了简单易用的接口，方便用户进行GPU计算。

声明包含在 EasyGPUCompute.h 中。\
实现包含在 EasyGPUCompute.cpp 中。

实现中包含一些以__Easy_开头的函数，这些函数是库的内部函数，除非你知道你在做什么，否则你不应该直接调用它们。

使用这个库需要链接一系列lib，具体为加入以下参数：

- -ld3d11

使用这个库需要了解一些DirectX 11的知识，包括但不限于Shader语言（HLSL）。


## 2. 库函数/类

### 2.1 EasyStartUpGPUCompute EasyReleaseGPUCompute 函数

这两个函数用于初始化和释放库，必须在程序开始和结束时调用。

### 2.2 EasyGPUCSOCode 类

代表了GPU代码，即Shader代码。

使用LoadCSOFromFile函数从文件中加载，其头部如下：

```cpp
EasyGPUCSOCode LoadCSOFromFile(string filename);
```

仅需传入文件名即可。

### 2.3 EasyDispatchComputeShader 函数

这个函数用于调用GPU进行计算。

头部如下：

```cpp
void EasyDispatchComputeShader(EasyGPUCSOCode computeShader, UINT ThreadGroupCountX, UINT ThreadGroupCountY, UINT ThreadGroupCountZ);
```

传入计算代码和三个轴上的线程组数量即可。

### 2.4 EasyGPUBuffer 类

代表了GPU上的一个读写缓冲区。

#### 2.4.1 EasyGPUBuffer::SetData 函数

用于绑定缓冲区的内容。

```cpp
template <typename DATA_VEC>
void SetData(DATA_VEC &&data, UINT UAVIndex);
```

第一个参数需要传入一个vector，第二个参数是UAV的索引，表示插槽位置。

传入的参数会绑定到hlsl中的 

```hlsl
RWStructuredBuffer<Type> name : register(u#)
```

注意，绑定后是不会自动取消绑定的，即使Dispath多次，仍然会保持绑定。

!!!!重要，此处传入的Type必须是16字节对齐的，否则可能会出现数据错位。但是如果你使用一些基本的类，如int，float，则不需要

#### 2.4.2 EasyGPUBuffer::ReadData 函数

用于在调用结束后读取缓冲区的内容。

```cpp
template <typename DATA_TYPE>
void ReadData(vector<DATA_TYPE> &data);
```

会将缓冲区的内容读取到传入的vector中。

#### 2.4.3 EasyGPUBuffer::Release 函数

用于释放缓冲区。

```cpp
void Release();
```

### 2.5 EasyGPUConstantBuffer 类

代表了GPU上的一个常量缓冲区。

这是个模板类，模板参数是缓冲区中存储的数据类型。

#### 2.5.1 EasyGPUConstantBuffer::StartUp 函数

用于初始化，并指定绑定到哪个槽位。

```cpp
void StartUp(UINT UAVIndex);
```

#### 2.5.2 EasyGPUConstantBuffer::UpdateData 函数

用于更新常量缓冲区的内容。
```cpp
void UpdateData(DATA_TYPE data);
```
这个函数会把数据绑定到hlsl重的
```hlsl
cbuffer name : register(b#)
{
    Type data;
}
```

#### 2.5.3 EasyGPUConstantBuffer::Release 函数

用于释放常量缓冲区。

```cpp
void Release();
```

## 3. hlsl代码编译

可以使用微软的fxc，dxc等等。

也可以使用EasyGPUCompute给的compiler。

使用前需要先编译，请链接如下库：

- d3dcompiler.lib
- d3d11.lib

命令行使用方法:

```bash
compiler.exe input.hlsl output.cso
```

## 4. 历史

暂无历史版本。

By MrJayden.