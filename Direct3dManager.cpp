#include "Direct3dManager.h"

#include "d3d11.h"
#include "dxerr.h"
#include "directxmath.h"
//#include "d3dcompiler.h"

namespace {
#include "gen_vertex_shader.h"
#include "gen_pixel_shader.h"
}

using namespace DirectX;

// include the Direct3D Library file
#pragma comment (lib, "d3d11.lib")
//#pragma comment (lib, "D3DCompiler.lib")

//Per-Device Interfaces
IDXGISwapChain* SwapChain = NULL;
ID3D11Device* d3d11Device = NULL;
ID3D11DeviceContext* d3d11DevCon = NULL;
ID3D11RenderTargetView* renderTargetView = NULL;

//Per-Mesh Interfaces
ID3D11InputLayout* vertLayout;
ID3D11Buffer* triangleVertBuffer;
//ID3D10Blob* VS_Buffer;
ID3D11VertexShader* VS;
//ID3D10Blob* PS_Buffer;
ID3D11PixelShader* PS;


struct Vertex {
  Vertex()
    :pos(0, 0, 0) {
  }

  Vertex(float x, float y, float z)
    : pos(x, y, z) {
  }

  XMFLOAT3 pos;
};

D3D11_INPUT_ELEMENT_DESC layout[] =
{
    { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
};
UINT numElements = ARRAYSIZE(layout);


//Error handling
constexpr auto MAX_ERRORSTRING = 100;
WCHAR szErrMsg[MAX_ERRORSTRING];


//Init test of DirectX working
float red = 0.0f;
float green = 0.0f;
float blue = 0.0f;
int colormodr = 7;
int colormodg = 7;
int colormodb = 7;


bool InitializeDirect3d11App(HINSTANCE hInstance, HWND hwnd) {
  HRESULT hr;

  //Describe our Buffer
  DXGI_MODE_DESC bufferDesc;

  ZeroMemory(&bufferDesc, sizeof(DXGI_MODE_DESC));

  bufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
  bufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
  bufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;

  //Describe our SwapChain
  DXGI_SWAP_CHAIN_DESC swapChainDesc;

  ZeroMemory(&swapChainDesc, sizeof(DXGI_SWAP_CHAIN_DESC));

  swapChainDesc.BufferDesc = bufferDesc;
  swapChainDesc.SampleDesc.Count = 1;
  swapChainDesc.SampleDesc.Quality = 0;
  swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
  swapChainDesc.BufferCount = 1;
  swapChainDesc.OutputWindow = hwnd;
  swapChainDesc.Windowed = TRUE;
  swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;


  //Create our SwapChain
  hr = D3D11CreateDeviceAndSwapChain(NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, NULL, NULL, NULL,
                                     D3D11_SDK_VERSION, &swapChainDesc, &SwapChain, &d3d11Device, NULL, &d3d11DevCon);
  if (FAILED(hr)) {
    DXGetErrorDescriptionW(hr, szErrMsg, MAX_ERRORSTRING);
    MessageBox(NULL, szErrMsg,
               TEXT(" D3D11CreateDeviceAndSwapChain"), MB_OK);
    return 0;
  }

  //Create our BackBuffer
  ID3D11Texture2D* backBuffer = nullptr;
  hr = SwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&backBuffer);
  if (FAILED(hr)) {
    DXGetErrorDescriptionW(hr, szErrMsg, MAX_ERRORSTRING);
    MessageBox(NULL, szErrMsg,
               TEXT("SwapChain->GetBuffer"), MB_OK);
    return 0;
  }

  //Create our Render Target
  hr = d3d11Device->CreateRenderTargetView(backBuffer, NULL, &renderTargetView);
  backBuffer->Release();
  if (FAILED(hr)) {
    DXGetErrorDescriptionW(hr, szErrMsg, MAX_ERRORSTRING);
    MessageBox(NULL, szErrMsg,
               TEXT("d3d11Device->CreateRenderTargetView"), MB_OK);
    return 0;
  }

  //Set our Render Target
  d3d11DevCon->OMSetRenderTargets(1, &renderTargetView, NULL);

  return true;
}

void TerminateDirect3d11App() {
  SwapChain->Release();
  d3d11Device->Release();
  d3d11DevCon->Release();
  renderTargetView->Release();
  vertLayout->Release();
  triangleVertBuffer->Release();
  //VS_Buffer->Release();
  VS->Release();
  //PS_Buffer->Release();
  PS->Release();
}

bool InitScene(float Height, float Width) {
  HRESULT hr;
  //Compile Shaders from shader file
  //hr = D3DCompileFromFile(L"shaders.hlsl", 0, 0, "VS", "vs_5_0", 0, 0, &VS_Buffer, 0);
  //hr = D3DCompileFromFile(L"shaders.hlsl", 0, 0, "PS", "ps_5_0", 0, 0, &PS_Buffer, 0);

  //Visual Studio is compiling shaders to header files

  //Create the Shader Objects
  hr = d3d11Device->CreateVertexShader(g_vertShader, sizeof(g_vertShader), NULL, &VS);
  hr = d3d11Device->CreatePixelShader(g_pixelShader, sizeof(g_pixelShader), NULL, &PS);

  //Set Vertex and Pixel Shaders
  d3d11DevCon->VSSetShader(VS, 0, 0);
  d3d11DevCon->PSSetShader(PS, 0, 0);

  //Create the vertex buffer
  Vertex v[] =
  {
      Vertex(0.0f, 0.5f, 0.5f),
      Vertex(0.5f, -0.5f, 0.5f),
      Vertex(-0.5f, -0.5f, 0.5f),
  };

  D3D11_BUFFER_DESC vertexBufferDesc;
  ZeroMemory(&vertexBufferDesc, sizeof(vertexBufferDesc));

  vertexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
  vertexBufferDesc.ByteWidth = sizeof(Vertex) * 3;
  vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
  vertexBufferDesc.CPUAccessFlags = 0;
  vertexBufferDesc.MiscFlags = 0;

  D3D11_SUBRESOURCE_DATA vertexBufferData;

  ZeroMemory(&vertexBufferData, sizeof(vertexBufferData));
  vertexBufferData.pSysMem = v;
  hr = d3d11Device->CreateBuffer(&vertexBufferDesc, &vertexBufferData, &triangleVertBuffer);

  //Set the vertex buffer
  UINT stride = sizeof(Vertex);
  UINT offset = 0;
  d3d11DevCon->IASetVertexBuffers(0, 1, &triangleVertBuffer, &stride, &offset);

  //Create the Input Layout
  hr = d3d11Device->CreateInputLayout(layout, numElements, g_vertShader,
                                      sizeof(g_vertShader), &vertLayout);

  //Set the Input Layout
  d3d11DevCon->IASetInputLayout(vertLayout);

  //Set Primitive Topology
  d3d11DevCon->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

  //Create the Viewport
  D3D11_VIEWPORT viewport;
  ZeroMemory(&viewport, sizeof(D3D11_VIEWPORT));

  viewport.TopLeftX = 0;
  viewport.TopLeftY = 0;
  viewport.Width = Width;
  viewport.Height = Height;

  //Set the Viewport
  d3d11DevCon->RSSetViewports(1, &viewport);

  return true;
}

void UpdateScene() {
  red += colormodr * 0.00005f;
  green += colormodg * 0.00002f;
  blue += colormodb * 0.00001f;

  if (red >= 1.0f || red <= 0.0f)
    colormodr *= -1;
  if (green >= 1.0f || green <= 0.0f)
    colormodg *= -1;
  if (blue >= 1.0f || blue <= 0.0f)
    colormodb *= -1;

}

void DrawScene() {
  //Clear our backbuffer to remove inter-frame dependency
  //https://docs.nvidia.com/gameworks/content/technologies/desktop/sli_avoiding_dependencies.htm
  FLOAT bgColor[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
  d3d11DevCon->ClearRenderTargetView(renderTargetView, bgColor);

  d3d11DevCon->Draw(3, 0);

  SwapChain->Present(0, 0);
}
