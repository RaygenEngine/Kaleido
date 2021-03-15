#include "D3D11PixelShader.h"

#include "renderer/renderers/contextual/d3d11/Error.h"

#include <d3dcompiler.h>

namespace { // WIP: hlsl gpu shader asset?
HRESULT CompileShader(_In_ LPCWSTR srcFile, _In_ LPCSTR entryPoint, _In_ LPCSTR profile, _Outptr_ ID3DBlob** blob)
{
	if (!srcFile || !entryPoint || !profile || !blob)
		return E_INVALIDARG;

	*blob = nullptr;

	UINT flags = D3DCOMPILE_ENABLE_STRICTNESS;
#if defined(DEBUG) || defined(_DEBUG)
	flags |= D3DCOMPILE_DEBUG;
#endif

	const D3D_SHADER_MACRO defines[] = { "EXAMPLE_DEFINE", "1", NULL, NULL };

	ID3DBlob* shaderBlob = nullptr;
	ID3DBlob* errorBlob = nullptr;
	HRESULT hr = D3DCompileFromFile(
		srcFile, defines, D3D_COMPILE_STANDARD_FILE_INCLUDE, entryPoint, profile, flags, 0, &shaderBlob, &errorBlob);
	if (FAILED(hr)) {
		if (errorBlob) {
			OutputDebugStringA((char*)errorBlob->GetBufferPointer());
			errorBlob->Release();
		}

		if (shaderBlob)
			shaderBlob->Release();

		return hr;
	}

	*blob = shaderBlob;

	return hr;
}
} // namespace

D3D11PixelShader::D3D11PixelShader(D3D11Graphics& gfx, const std::wstring& path)
	: D3D11Bindable(gfx)
{
	CompileShader(path.c_str(), "main", "ps_4_0_level_9_1", &m_d3dBlob);
	ABORT_IF_FAILED(GetDevice()->CreatePixelShader(
		m_d3dBlob->GetBufferPointer(), m_d3dBlob->GetBufferSize(), nullptr, &m_d3d11PixelShader));
}

void D3D11PixelShader::Bind()
{
	GetContext()->PSSetShader(m_d3d11PixelShader.Get(), nullptr, 0u);
}

ID3DBlob* D3D11PixelShader::GetBytecode() const
{
	return m_d3dBlob.Get();
}
