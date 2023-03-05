#include "LSEFramework.h"
#include "LSTimer.h"

import Engine.Common;
import D3D11Lib;
import Platform.Win32Window;
import Util.HLSLUtils;
import Helper.LSCommonTypes;
import Helper.PipelineFactory;

using namespace LS;
using namespace LS::Win32;
using namespace LS::Utils;
using namespace Microsoft::WRL;
using namespace std::chrono;
using namespace std::chrono_literals;
using namespace DirectX;

#ifdef DEBUG
static const std::string shader_path = R"(build\x64\Debug\)";
#else
static const std::string shader_path = R"(build\x64\Release\)";
#endif

struct Vertex
{
	std::array<float, 4> Position;
	std::array<float, 4> Color;
	std::array<float, 2> UV;
};

LSTimer<std::uint64_t, 1ul, 1000ul> g_timer;
std::array<float, 4> g_red = { 1.0f, 0.0f, 0.0f, 1.0f };
std::array<float, 4> g_green = { 0.0f, 1.0f, 0.0f, 1.0f };
std::array<float, 4> g_blue = { 0.0f, 0.0f, 1.0f, 1.0f };
std::array<float, 4> g_black = { 0.0f, 0.0f, 0.0f, 1.0f };
std::array<float, 4> g_white = { 1.0f, 1.0f, 1.0f, 1.0f };

static std::array<Vertex, 3> g_triangle{
	Vertex{.Position = {0.0f, 1.0f, 0.0f, 1.0f}, .Color{0.0f, 1.0f, 0.0f, 1.0f}, .UV{0.5f, 0.0f}},
	Vertex{.Position = {1.0f, -1.0f, 0.0f, 1.0f}, .Color{0.0f, 1.0f, 0.0f, 1.0f}, .UV{1.0f, 1.0f}},
	Vertex{.Position = {-1.0f, -1.0f, 0.0f, 1.0f}, .Color{0.0f, 1.0f, 0.0f, 1.0f}, .UV{0.0f, 1.0f}},
};
static std::array<uint32_t, 3> g_triIndices{ 0, 2, 1 };

static std::array<Vertex, 6> g_fullQuad{
	Vertex{.Position = {-1.0f, -1.0f, 0.0f, 1.0f}, .Color{0.0f, 0.0f, 0.0f, 1.0f}, .UV{0.0f, 0.0f}},
	Vertex{.Position = {1.0f, 0.0f, 0.0f, 1.0f}, .Color{0.0f, 0.0f, 0.0f, 1.0f}, .UV{1.0f, 0.0f}},
	Vertex{.Position = {0.0f, 1.0f, 0.0f, 1.0f}, .Color{0.0f, 0.0f, 0.0f, 1.0f}, .UV{0.0f, 1.0f}},
	Vertex{.Position = {1.0f, 0.0f, 0.0f, 1.0f}, .Color{0.0f, 0.0f, 0.0f, 1.0f}, .UV{1.0f, 0.0f}},
	Vertex{.Position = {1.0f, 1.0f, 0.0f, 1.0f}, .Color{0.0f, 0.0f, 0.0f, 1.0f}, .UV{1.0f, 1.0f}},
	Vertex{.Position = {0.0f, 1.0f, 0.0f, 1.0f}, .Color{0.0f, 0.0f, 0.0f, 1.0f}, .UV{0.0f, 1.0f}},
};

static std::array<float, 24> g_positions // POS / UV // Padding added after each entry
{                                    // POSITION              // UV       // PADDING //
 0.0f, 1.0f, 0.0f, 1.0f, 0.5f, 0.0f, 0.0f, 0.0f,
 1.0f, -1.0f, 0.0f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f,
 -1.0f, -1.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f };
static std::array<uint32_t, 3> g_indices{ 0, 2, 1 };

static std::array<float, 36> g_positions2{
	1.0f, 1.0f, 0.20f, 1.0f, 1.0f, 0.0f,  // 0
	1.0f, 0.0f, 0.20f, 1.0f, 1.0f, 1.0f,  // 1
	-1.0f, 0.0f, 0.20f, 1.0f, 0.0f, 1.0f, // 2
	1.0f, 1.0f, 0.20f, 1.0f, 1.0f, 0.0f,  // 3
	-1.0f, 1.0f, 0.20f, 1.0f, 0.0f, 0.0f, // 4
	-1.0f, 0.0f, 0.20f, 1.0f, 0.0f, 1.0f  // 5
};
static std::array<uint32_t, 6> g_indices2{ 0, 2, 1, 3, 5, 4 };

constexpr uint32_t SCREEN_WIDTH = 800;
constexpr uint32_t SCREEN_HEIGHT = 700;

// static std::array<uint32_t, 32 * 32> g_textureData;
static std::array<uint32_t, SCREEN_WIDTH* SCREEN_HEIGHT> g_textureData{};

auto CompileShader(std::filesystem::path path) -> std::optional<std::vector<std::byte>>;
auto CreateVertexShader(const DeviceD3D11& device, ComPtr<ID3D11VertexShader>& shader, std::vector<std::byte>& byteCode) -> bool;
auto CreatePixelShader(const DeviceD3D11& device, ComPtr<ID3D11PixelShader>& shader, std::vector<std::byte>& byteCode) -> bool;

// TODO: Make a cube appear by loading in a .obj or .gltf reader
// TODO: Consider cleaning up the modules a little
// TODO: Add a library module for calls like CreateDevice(DEVICE_API) and CreatePipelineFactory(DEVICE_API)
//  so these can be implemented and more "properly" handled via the library as we might expect
// TODO: Remove the throws and create an "LSExpected" type like C++ 23 Expected type for error handling
// TODO: Examine current setup in main, and consider how now to make things a little easier on the API side to remove
//  some redeundant code or simplify things
// TODO: Start DX12 renderer
// TODO: Start Vulkan renderer
// TODO: Examin needed Interfaces and build them
int main()
{
	std::cout << "Hello Luna Sol Game Engine!\n";
	Ref<LSWindowBase> window = LS::BuildWindow(SCREEN_WIDTH, SCREEN_HEIGHT, L"Hello App");

	std::cout << "Texture size: " << g_textureData.size() << "\n";
	// Device Setup //
	// auto device = LS::BuildDevice(LS::DEVICE_API::DIRECTX_11);
	DeviceD3D11 device;
	device.CreateDevice();

	ComPtr<ID3D11DeviceContext4> immContext = device.GetImmediateContext();
	device.CreateSwapchain(window.get());

	auto rsSolidOptional = CreateRasterizerState2(device.GetDevice().Get(), SolidFill_NoneCull_CCWFront_DCE);
	auto rsWireframeOptional = CreateRasterizerState2(device.GetDevice().Get(), Wireframe_FrontCull_CCWFront_DCE);

	ComPtr<ID3D11RasterizerState2> rsSolid;
	rsSolid.Attach(rsSolidOptional.value_or(nullptr));
	ComPtr<ID3D11RasterizerState2> rsWireframe;
	rsWireframe.Attach(rsWireframeOptional.value_or(nullptr));

	// Create Texture //
	for (auto i = 0; i < g_textureData.size(); ++i)
	{
		g_textureData[i] = 0xFF0000FF;
	}

	D3D11_TEXTURE2D_DESC textureDesc{};
	textureDesc.Width = SCREEN_WIDTH;
	textureDesc.Height = SCREEN_HEIGHT;
	textureDesc.ArraySize = 1;
	textureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
	textureDesc.CPUAccessFlags = 0;
	textureDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	textureDesc.MipLevels = 1;
	textureDesc.MiscFlags = 0;
	textureDesc.Usage = D3D11_USAGE_DEFAULT;
	textureDesc.SampleDesc.Count = 1;
	// TODO: Create 2 or 3 textures (one per context) that is blank and empty
	/*D3D11_SUBRESOURCE_DATA texData{};
	texData.pSysMem = &g_textureData;
	texData.SysMemPitch = sizeof(uint32_t) * textureDesc.Width;
	texData.SysMemSlicePitch = 0;*/
	ComPtr<ID3D11Texture2D> pRenderTargetTexture;
	auto texResult = device.GetDevice()->CreateTexture2D(&textureDesc, NULL, &pRenderTargetTexture);
	if (FAILED(texResult))
		return -20;

	ComPtr<ID3D11Texture2D> pTexture;
	textureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	texResult = device.GetDevice()->CreateTexture2D(&textureDesc, NULL, &pTexture);
	if (FAILED(texResult))
		return -21;

	ComPtr<ID3D11Texture2D> pTexture2;
	texResult = device.GetDevice()->CreateTexture2D(&textureDesc, NULL, &pTexture2);
	if (FAILED(texResult))
		return -22;
	// TODO: Create SRVs that point to the 2 or 3 textures we make from above
	ComPtr<ID3D11ShaderResourceView> pTexturRenderTargetSRV;
	ComPtr<ID3D11ShaderResourceView> pTextureView1;
	ComPtr<ID3D11ShaderResourceView> pTextureView2;

	D3D11_SHADER_RESOURCE_VIEW_DESC texresView{};
	D3D11_TEX2D_SRV tex2dSRV{};
	tex2dSRV.MipLevels = textureDesc.MipLevels;

	texresView.Format = textureDesc.Format;
	texresView.ViewDimension = D3D_SRV_DIMENSION_TEXTURE2D;
	texresView.Texture2D = tex2dSRV;
	auto srvResult = device.GetDevice()->CreateShaderResourceView(pRenderTargetTexture.Get(), &texresView, &pTexturRenderTargetSRV);
	if (FAILED(srvResult))
	{
		return -23;
	}
	srvResult = device.GetDevice()->CreateShaderResourceView(pTexture.Get(), &texresView, &pTextureView1);
	if (FAILED(srvResult))
	{
		return -24;
	}
	srvResult = device.GetDevice()->CreateShaderResourceView(pTexture2.Get(), &texresView, &pTextureView2);
	if (FAILED(srvResult))
	{
		return -25;
	}

	ComPtr<ID3D11SamplerState> pSampler;
	CD3D11_SAMPLER_DESC samplerDesc(CD3D11_DEFAULT{});
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	auto samplerResult = device.GetDevice()->CreateSamplerState(&samplerDesc, &pSampler);
	if (FAILED(samplerResult))
		return -26;

	// END TEXTURE STUFF //

	// Render Target //
	ComPtr<ID3D11Texture2D> buffer;
	device.GetSwapChain()->GetBuffer(0, IID_PPV_ARGS(&buffer));
	D3D11_TEXTURE2D_DESC swpDesc;
	buffer->GetDesc(&swpDesc);
	ComPtr<ID3D11RenderTargetView1> defaultRTView;
	ComPtr<ID3D11RenderTargetView1> pTextureRTView;
	defaultRTView.Attach(CreateRenderTargetView1(device.GetDevice().Get(), buffer.Get()));
	// Render Target as Texture2D //
	D3D11_RENDER_TARGET_VIEW_DESC1 pTextureRTDesc{};
	pTextureRTDesc.Format = textureDesc.Format;
	pTextureRTDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
	D3D11_TEX2D_RTV1 texRTV{};
	texRTV.MipSlice = 0;
	texRTV.PlaneSlice = 0;
	pTextureRTDesc.Texture2D = texRTV;
	auto rtvTexture = CreateRenderTargetView1(device.GetDevice().Get(), pRenderTargetTexture.Get(), &pTextureRTDesc);

	pTextureRTView.Attach(rtvTexture);

	std::array<ID3D11RenderTargetView*, 2> rtViews = { defaultRTView.Get(), pTextureRTView.Get() };

	// Depth Stencil //
	ComPtr<ID3D11DepthStencilView> dsView;
	auto dsResult = device.CreateDepthStencilViewForSwapchain(defaultRTView.Get(), &dsView);
	if (FAILED(dsResult))
		return -3;

	CD3D11_DEPTH_STENCIL_DESC defaultDepthDesc(CD3D11_DEFAULT{});
	ComPtr<ID3D11DepthStencilState> defaultState;
	auto dss = CreateDepthStencilState(device.GetDevice().Get(), defaultDepthDesc).value();
	defaultState.Attach(dss);
	HRESULT result;
	// Blend State
	ComPtr<ID3D11BlendState> blendState;
	CD3D11_BLEND_DESC blendDesc{};
	blendDesc.AlphaToCoverageEnable = false;
	blendDesc.IndependentBlendEnable = false;
	D3D11_RENDER_TARGET_BLEND_DESC rtbd{};
	rtbd.BlendEnable = true;
	rtbd.SrcBlend = D3D11_BLEND_SRC_COLOR;
	rtbd.DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
	rtbd.BlendOp = D3D11_BLEND_OP_ADD;
	rtbd.SrcBlendAlpha = D3D11_BLEND_ONE;
	rtbd.DestBlendAlpha = D3D11_BLEND_ZERO;
	rtbd.BlendOpAlpha = D3D11_BLEND_OP_ADD;
	rtbd.RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

	blendDesc.RenderTarget[0] = rtbd;
	blendDesc.RenderTarget[1] = rtbd;
	blendDesc.RenderTarget[2] = rtbd;
	blendDesc.RenderTarget[3] = rtbd;
	blendDesc.RenderTarget[4] = rtbd;
	blendDesc.RenderTarget[5] = rtbd;
	blendDesc.RenderTarget[6] = rtbd;
	blendDesc.RenderTarget[7] = rtbd;

	result = device.CreateBlendState(blendDesc, &blendState);
	if (FAILED(result))
		return -4;

	auto createDeferredContext = [&]() -> ComPtr<ID3D11DeviceContext>
	{
		ComPtr<ID3D11DeviceContext> pDeferredContext;
		result = device.CreateDeferredContext(pDeferredContext.ReleaseAndGetAddressOf());
		if (FAILED(result))
			return nullptr;
		return pDeferredContext;
	};

	auto context1 = createDeferredContext();
	auto context2 = createDeferredContext();

	// BEGIN SHADER FILE OPERATIONS //
	auto vertexShader = L"VertexShader.cso";
	auto vertexShader2 = L"VertexShader2.cso";
	auto pixelShader = L"PixelShader.cso";
	auto pixelShader2 = L"PixelShader2.cso";
	auto texturePS = L"TextureShader.cso";
	auto fsQuadVS = L"FullScreenQuad.cso";

	std::array<wchar_t, _MAX_PATH> modulePath{};
	if (!GetModuleFileName(nullptr, modulePath.data(), modulePath.size()))
	{
		throw std::runtime_error("Failed to find module path\n");
	}

	auto path = std::wstring(modulePath.data());
	auto lastOf = path.find_last_of(L"\\");
	path.erase(lastOf);

	auto vsPath = path + L"\\" + vertexShader;
	auto vsPath2 = path + L"\\" + vertexShader2;
	auto psPath = path + L"\\" + pixelShader;
	auto psPath2 = path + L"\\" + pixelShader2;
	auto texPSPath = path + L"\\" + texturePS;
	auto fsQuadPath = path + L"\\" + fsQuadVS;

	// Vertex Shaders //
	auto vsDataOpt = CompileShader(vsPath);
	if (!vsDataOpt)
		return -1;
	auto vsData = vsDataOpt.value();
	auto vsData2Opt = CompileShader(vsPath2);
	if (!vsData2Opt)
		return -1;
	auto vsData2 = vsData2Opt.value();
	auto psDataOpt = CompileShader(psPath);
	if (!psDataOpt)
		return -1;
	auto psData = psDataOpt.value();
	auto psData2Opt = CompileShader(psPath2);
	if (!psData2Opt)
		return -1;
	auto psData2 = psData2Opt.value();
	auto texPSDataOpt = CompileShader(texPSPath);
	if (!texPSDataOpt)
		return -1;
	auto texPSData = texPSDataOpt.value();
	auto fsQuadOpt = CompileShader(fsQuadPath);
	if (!fsQuadOpt)
		return -1;
	auto fsQuadData = fsQuadOpt.value();

	// END SHADER FILE OPERATIONS //

	// Compile Shader Objects //
	ComPtr<ID3D11VertexShader> vsShader;
	ComPtr<ID3D11VertexShader> vsShader2;
	ComPtr<ID3D11VertexShader> fsQuadShader;
	ComPtr<ID3D11PixelShader> psShader;
	ComPtr<ID3D11PixelShader> psShader2;
	ComPtr<ID3D11PixelShader> texPS;

	auto shaderResult = CreateVertexShader(device, vsShader, vsData);
	if (!shaderResult)
		return -2;
	shaderResult = CreateVertexShader(device, vsShader2, vsData2);
	if (!shaderResult)
		return -2;
	shaderResult = CreatePixelShader(device, psShader, psData);
	if (!shaderResult)
		return -2;
	shaderResult = CreatePixelShader(device, psShader2, psData2);
	if (!shaderResult)
		return -2;
	shaderResult = CreatePixelShader(device, texPS, texPSData);
	if (!shaderResult)
		return -2;
	shaderResult = CreateVertexShader(device, fsQuadShader, fsQuadData);
	if (!shaderResult)
		return -2;

	LSShaderInputSignature vsSignature;
	vsSignature.AddElement(SHADER_DATA_TYPE::FLOAT4, "POSITION0");
	ShaderElement elementColor = {
		.ShaderData = SHADER_DATA_TYPE::FLOAT4,
		.SemanticName = "COLOR",
		.SemanticIndex = 0,
		.OffsetAligned = D3D11_APPEND_ALIGNED_ELEMENT,
		.InputSlot = 0,
		.InputClass = INPUT_CLASS::VERTEX,
		.InstanceStepRate = 0 };

	ShaderElement elementTexCoord = {
		.ShaderData = SHADER_DATA_TYPE::FLOAT2,
		.SemanticName = "TEXCOORD",
		.SemanticIndex = 0,
		.OffsetAligned = D3D11_APPEND_ALIGNED_ELEMENT,
		.InputSlot = 0,
		.InputClass = INPUT_CLASS::VERTEX,
		.InstanceStepRate = 0 };

	vsSignature.AddElement(elementColor);
	auto layout = vsSignature.GetInputLayout();
	auto inputPos = BuildFromShaderElements(layout);
	auto valInputs = inputPos.value();
	if (!inputPos)
	{
		throw std::runtime_error("Failed to create input layout from shader elements\n");
	}

	ComPtr<ID3D11InputLayout> pInputLayoutPosColor;
	if (FAILED(device.CreateInputLayout(valInputs, vsData2, &pInputLayoutPosColor)))
	{
		throw std::runtime_error("Failed to create input layout from device\n");
	}

	vsSignature.AddElement(elementTexCoord);
	auto layout2 = vsSignature.GetInputLayout();
	auto inputPosTex = BuildFromShaderElements(layout2);
	auto valInputs2 = inputPosTex.value();
	ComPtr<ID3D11InputLayout> pInputLayoutPosColorText;
	if (FAILED(device.CreateInputLayout(valInputs2, vsData, &pInputLayoutPosColorText)))
	{
		throw std::runtime_error("Failed to create input layout from device\n");
	}

	LSShaderInputSignature fsQuadSignature;
	fsQuadSignature.AddElement(SHADER_DATA_TYPE::UINT, "SV_VertexID");
	auto inputIdLayout = fsQuadSignature.GetInputLayout();
	auto inputIdOpt = BuildFromShaderElements(inputIdLayout);
	if (!inputIdOpt)
		return -10;
	auto inputId = inputIdOpt.value();
	ComPtr<ID3D11InputLayout> pInputVertexID;
	if (FAILED(device.CreateInputLayout(inputId, fsQuadData, &pInputVertexID)))
	{
		throw std::runtime_error("Failed to create input layout for FS Quad\n");
	}

	// TODO: Implement missing setups below that don't have an appropriate function call
	ComPtr<ID3D11Buffer> vertexBuffer;
	CD3D11_BUFFER_DESC bufferDesc(sizeof(Vertex) * g_triangle.size(), D3D11_BIND_VERTEX_BUFFER);
	D3D11_SUBRESOURCE_DATA subData{ .pSysMem = g_triangle.data(), .SysMemPitch = 0, .SysMemSlicePitch = 0 };
	result = device.CreateBuffer(&bufferDesc, &subData, &vertexBuffer);
	if (FAILED(result))
	{
		return -2;
	}

	// Camera //
	xmvec posVec = XMVectorSet(0.0f, 0.0f, -5.0f, 1.0f);
	xmvec lookAtVec = XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f);
	xmvec upVec = XMVectorSet(0.0f, 1.0f, 0.0f, 1.0f);

	LSCamera camera(window->GetWidth(), window->GetHeight(), posVec, lookAtVec, upVec, 100.0f);

	// Informs how the GPU about the buffer types - we have two Matrix and Index Buffers here, the Vertex was created earlier above //
	CD3D11_BUFFER_DESC matBD(sizeof(float) * 16, D3D11_BIND_CONSTANT_BUFFER);
	CD3D11_BUFFER_DESC indexBD(g_indices.size() * sizeof(g_indices.front()), D3D11_BIND_INDEX_BUFFER);
	CD3D11_BUFFER_DESC colorBD(sizeof(float) * 4, D3D11_BIND_CONSTANT_BUFFER);

	// Ready the GPU Data //
	D3D11_SUBRESOURCE_DATA viewSRD{}, projSRD{}, modelSRD{}, indexSRD{}, colorRedSRD{}, colorGreenSRD{}, colorBlueSRD{};
	viewSRD.pSysMem = &camera.View;
	projSRD.pSysMem = &camera.Projection;
	indexSRD.pSysMem = g_indices.data();
	colorRedSRD.pSysMem = g_red.data();
	colorGreenSRD.pSysMem = g_green.data();
	colorBlueSRD.pSysMem = g_blue.data();

	// Our Model's Translastion/Scale/Rotation Setup //
	xmmat modelScaleMat = XMMatrixScaling(1.0f, 1.0f, 1.0f);
	xmmat modelRotMat = XMMatrixRotationRollPitchYaw(0.0f, 0.0f, 0.0f);
	xmmat modelTransMat = XMMatrixTranslation(0.0f, 0.0f, 0.0f);
	xmmat modelTransform = XMMatrixIdentity();

	modelTransform = XMMatrixMultiply(modelTransMat, XMMatrixMultiply(modelScaleMat, modelRotMat));
	modelSRD.pSysMem = &modelTransform;

	// Initialize Buffers //
	ComPtr<ID3D11Buffer> viewBuffer, projBuffer, modelBuffer, indexBuffer, colorRedBuffer, colorGreenBuffer, colorBlueBuffer;
	device.CreateBuffer(&matBD, &viewSRD, &viewBuffer);
	device.CreateBuffer(&matBD, &projSRD, &projBuffer);
	device.CreateBuffer(&matBD, &modelSRD, &modelBuffer);
	device.CreateBuffer(&indexBD, &indexSRD, &indexBuffer);
	device.CreateBuffer(&colorBD, &colorRedSRD, &colorRedBuffer);
	device.CreateBuffer(&colorBD, &colorGreenSRD, &colorGreenBuffer);
	device.CreateBuffer(&colorBD, &colorBlueSRD, &colorBlueBuffer);

	UINT stride = sizeof(Vertex);
	/// SET PIPELINE STATE ///

/**********************FIRST CONTEXT BEGIN *****************************/
	// First Context Begin //
	BindVS(context1.Get(), fsQuadShader.Get());
	BindPS(context1.Get(), psShader.Get());
	SetInputlayout(context1.Get(), pInputVertexID.Get());
	SetRasterizerState(context1.Get(), rsSolid.Get());
	SetBlendState(context1.Get(), blendState.Get());
	SetTopology(context1.Get(), D3D11_PRIMITIVE_TOPOLOGY::D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	SetViewport(context1.Get(), static_cast<float>(window->GetWidth()), static_cast<float>(window->GetHeight()));

	// Immediate Context - Draw //
	SetRenderTarget(context1.Get(), pTextureRTView.Get(), dsView.Get());
	ClearRT(context1.Get(), pTextureRTView.Get(), g_blue);
	ClearDS(context1.Get(), dsView.Get());
	Draw(context1.Get(), 6);

/**********************SECOND CONTEXT BEGIN *****************************/
	// Second Context Begin //
	BindVS(context2.Get(), vsShader.Get());
	BindPS(context2.Get(), psShader2.Get());
	context2->PSSetShaderResources(0, 1, pTexturRenderTargetSRV.GetAddressOf());
	context2->PSSetSamplers(0, 1, pSampler.GetAddressOf());
	SetInputlayout(context2.Get(), pInputLayoutPosColorText.Get());
	std::array<ID3D11Buffer*, 4> buffers2{ viewBuffer.Get(), projBuffer.Get(), modelBuffer.Get(),
										   colorGreenBuffer.Get() };
	BindVSConstantBuffers(context2.Get(), 0, buffers2);
	SetVertexBuffers(context2.Get(), vertexBuffer.Get(), 1, 0, stride);
	SetIndexBuffer(context2.Get(), indexBuffer.Get());
	SetRasterizerState(context2.Get(), rsSolid.Get());
	SetBlendState(context2.Get(), blendState.Get());
	SetDepthStencilState(context2.Get(), defaultState.Get(), 1);
	SetTopology(context2.Get(), D3D11_PRIMITIVE_TOPOLOGY::D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	SetViewport(context2.Get(), static_cast<float>(window->GetWidth()), static_cast<float>(window->GetHeight()));

	// 2nd Pass - Use default Render Target, and bind Texture as resource //
	SetRenderTarget(context2.Get(), defaultRTView.Get(), dsView.Get());
	ClearRT(context2.Get(), defaultRTView.Get(), g_blue);
	ClearDS(context2.Get(), dsView.Get());
	DrawIndexed(context2.Get(), g_indices.size(), 0, 0);
	// END SECOND CONTEXT DRAW // 

	FLOAT color[4]{ 0.0f, 0.0f, 0.0f, 0.0f };
	//---------------------- IMMEDIATE CONTEXT ---------------------------//
		// SET Immediate Context State //
	BindVS(immContext.Get(), fsQuadShader.Get());
	BindPS(immContext.Get(), psShader.Get());
	SetInputlayout(immContext.Get(), pInputVertexID.Get());
	SetRasterizerState(immContext.Get(), rsSolid.Get());
	SetBlendState(immContext.Get(), blendState.Get());
	SetTopology(immContext.Get(), D3D11_PRIMITIVE_TOPOLOGY::D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	SetViewport(immContext.Get(), static_cast<float>(window->GetWidth()), static_cast<float>(window->GetHeight()));

	// Immediate Context - Draw //
	//SetRenderTarget(immContext.Get(), pTextureRTView.Get(), dsView.Get());
	SetRenderTarget(immContext.Get(), defaultRTView.Get(), dsView.Get());
	//ClearRT(immContext.Get(), pTextureRTView.Get(), g_blue);
	ClearRT(immContext.Get(), defaultRTView.Get(), g_blue);
	ClearDS(immContext.Get(), dsView.Get());
	//DrawIndexed(immContext.Get(), g_indices.size(), 0, 0);
	Draw(immContext.Get(), 6);

	// SECOND PASS //
	//BindVS(immContext.Get(), fsQuadShader.Get());
	//BindPS(immContext.Get(), psShader.Get());
	//immContext->PSSetShaderResources(0, 1, pTexturRenderTargetSRV.GetAddressOf());
	//immContext->PSSetSamplers(0, 1, pSampler.GetAddressOf());
	//SetInputlayout(immContext.Get(), pInputVertexID.Get());
	//std::array<ID3D11Buffer*, 4> buffersImm{ viewBuffer.Get(), projBuffer.Get(), modelBuffer.Get(),
	//										 colorRedBuffer.Get() };
	//BindVSConstantBuffers(immContext.Get(), 0, buffersImm);
	//SetVertexBuffers(immContext.Get(), vertexBuffer.Get(), 1, 0, stride);
	//SetIndexBuffer(immContext.Get(), indexBuffer.Get());
	//SetRasterizerState(immContext.Get(), rsSolid.Get());
	//SetBlendState(immContext.Get(), blendState.Get());
	//SetTopology(immContext.Get(), D3D11_PRIMITIVE_TOPOLOGY::D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	//SetViewport(immContext.Get(),
	//	static_cast<float>(window->GetWidth()),
	//	static_cast<float>(window->GetHeight()));

	//// Immediate Context - Draw //
	//SetRenderTarget(immContext.Get(), defaultRTView.Get(), dsView.Get());
	//ClearRT(immContext.Get(), defaultRTView.Get(), g_blue);
	//ClearDS(immContext.Get(), dsView.Get());
	//DrawIndexed(immContext.Get(), g_indices.size(), 0, 0);
	//---------------------- IMMEDIATE CONTEXT END ---------------------------//

	ComPtr<ID3D11CommandList> command1, command2;
	auto hr = context1->FinishCommandList(FALSE, &command1);
	if (FAILED(hr))
		return EXIT_FAILURE;

	hr = context2->FinishCommandList(FALSE, &command2);
	if (FAILED(hr))
		return EXIT_FAILURE;

	// Show Window and Start Timer - duh... //
	window->Show();
	g_timer.Start();

	Present1(device.GetSwapChain().Get(), 1);
	while (window->IsRunning())
	{
		g_timer.Tick();
		if (command1)
		{
			immContext->ExecuteCommandList(command1.Get(), false);
		}
		if (command2)
		{
			immContext->ExecuteCommandList(command2.Get(), true);
		}
		Present1(device.GetSwapChain().Get(), 1);
		window->PollEvent();
	}
}

auto CompileShader(std::filesystem::path path) -> std::optional<std::vector<std::byte>>
{
	if (!std::filesystem::exists(path))
	{
		return std::nullopt;
	}

	std::fstream stream{ path, std::fstream::in | std::fstream::binary };
	if (!stream.is_open() && !stream.good())
	{
		return std::nullopt;
	}
	auto fileSize = std::filesystem::file_size(path);
	std::vector<std::byte> shaderData(fileSize);
	stream.read(reinterpret_cast<char*>(shaderData.data()), fileSize);
	stream.close();

	return shaderData;
}

auto CreateVertexShader(const DeviceD3D11& device, ComPtr<ID3D11VertexShader>& shader, std::vector<std::byte>& byteCode) -> bool
{
	auto vsResult = CompileVertexShaderFromByteCode(device.GetDevice().Get(), byteCode, &shader);
	if (FAILED(vsResult))
	{
		return false;
	}
	return true;
}

auto CreatePixelShader(const DeviceD3D11& device, ComPtr<ID3D11PixelShader>& shader, std::vector<std::byte>& byteCode) -> bool
{
	auto psResult = CompilePixelShaderFromByteCode(device.GetDevice().Get(), byteCode, &shader);
	if (FAILED(psResult))
	{
		return false;
	}
	return true;
}