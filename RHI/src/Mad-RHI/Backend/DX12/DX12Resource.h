#pragma once

#include "Mad-RHI/Resource.h"

namespace mad::rhi {

class DX12Texture : public ObjectBase<Texture>
{
protected:
    ~DX12Texture();

public:
    DX12Texture(const TextureDesc& desc);

    virtual RefPtr<TextureView> GetDefaultSRV() override;
    virtual RefPtr<TextureView> GetDefaultRTV() override;
    virtual RefPtr<TextureView> GetDefaultDSV() override;

    virtual const TextureDesc& GetDesc() override;
    virtual ResourceState GetCurrentResourceState() override;

private:
    
};

class DX12Buffer : public ObjectBase<Buffer>
{
protected:
    ~DX12Buffer();

public:
    DX12Buffer(const BufferDesc& desc);

    virtual void* Map() override;
    virtual void Unmap() override;
        
    virtual const BufferDesc& GetDesc() override;
    virtual ResourceState GetCurrentResourceState() override;

private:

};

class DX12Sampler : public ObjectBase<Sampler>
{
protected:
    ~DX12Sampler();

public:
    DX12Sampler(const SamplerDesc& desc);

    virtual const SamplerDesc& GetDesc() override;

private:
    
};

class DX12TextureView : public ObjectBase<TextureView>
{
protected:
    ~DX12TextureView();

public:
    DX12TextureView(RefCounter* sharedCounter, const TextureViewDesc& desc);

private:

};

}