#define COBJMACROS
#define WIN32_LEAN_AND_MEAN

#include <stdio.h>
#include <stdlib.h>
#include <d3d11.h>

#define Error(msg, failedReturn) \
    MessageBox(0, "Error!", msg, MB_OK | MB_ICONERROR); \
    printf(msg); \
    return failedReturn;
    

#define CheckHR(msg, failedReturn) \
    if(FAILED(hr)) { \
        Error(msg, failedReturn); \
    }

//WIN32
WNDCLASSEX wnd;
HWND window;
LPCSTR windowClassName = "honkClass";
LPCSTR windowName = "D3D11 in C";
UINT width = 720;
UINT height = 720;

//D3D11 Stuff
ID3D11Device* device;
ID3D11DeviceContext* deviceContext;
ID3D11RenderTargetView* target;
IDXGISwapChain* swapChain;

//D3D11 Shader Binaries Size
UINT psSize;
UINT vsSize;

//D3D11 Shader Binaries
UCHAR *psCode;
UCHAR *vsCode;

//D3D11 Shaders
ID3D11VertexShader* vertexShader;
ID3D11PixelShader* pixelShader;

//D3D11 Buffers
ID3D11Buffer* vertexBuffer;
ID3D11Buffer* indexBuffer;
ID3D11InputLayout* inputLayout;

typedef struct {
    long size;
    unsigned char* bytes;
} ByteDataAndSize;
typedef struct {
    float x, y, z;
    float r, g, b, a;
} Vertex;

D3D11_INPUT_ELEMENT_DESC elements[] = {
    {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
    {"COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0}
};

BOOL readBinaryFile(const char* fileName, ByteDataAndSize *returnVal) {
    ByteDataAndSize out;

    FILE* file = fopen(fileName, "rb");
    if (!file) { return FALSE; };
    fseek(file, 0, SEEK_END);

    vsSize = ftell(file);

    fseek(file, 0, SEEK_SET);
    out.size = vsSize;
    out.bytes = malloc(vsSize);

    fread(out.bytes, 1, vsSize, file);

    fclose(file);

    *returnVal = out;
    return TRUE;
}

LRESULT WndProc(HWND window, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg)
    {
    case WM_CLOSE:
        PostQuitMessage(0);
    }
    return DefWindowProc(window, msg, wParam, lParam);
}

int main(int argc, char* argv[]) {
    
    ZeroMemory(&wnd, sizeof(wnd));
    wnd.cbSize = sizeof(WNDCLASSEX);
    wnd.lpfnWndProc = WndProc;
    wnd.lpszClassName = windowClassName;
    wnd.hInstance = GetModuleHandle(0);
    wnd.style = CS_OWNDC;
    RegisterClassEx(&wnd);
    int x = GetSystemMetrics(SM_CXSCREEN) / 2 - width / 2;
    int y = GetSystemMetrics(SM_CYSCREEN) / 2 - height / 2;
    window = CreateWindowEx(NULL, windowClassName, windowName, WS_SYSMENU | WS_MINIMIZEBOX, x, y, width, height, NULL, NULL, GetModuleHandle(0), NULL);
    ShowWindow(window, SW_SHOW);

    DXGI_SWAP_CHAIN_DESC desc;
    ZeroMemory(&desc, sizeof(desc));
    desc.BufferDesc.RefreshRate.Numerator = 0;
    desc.BufferDesc.RefreshRate.Denominator = 1;
    desc.BufferDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
    desc.SampleDesc.Count = 1;
    desc.SampleDesc.Quality = 0;
    desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    desc.BufferCount = 1;
    desc.OutputWindow = window;
    desc.Windowed = TRUE;
    
    D3D_FEATURE_LEVEL featureLevels[] = { D3D_FEATURE_LEVEL_11_1 };
    D3D_FEATURE_LEVEL featureLevel = D3D_FEATURE_LEVEL_11_1;

    HRESULT hr = D3D11CreateDeviceAndSwapChain(
        NULL,
        D3D_DRIVER_TYPE_HARDWARE,
        NULL,
        D3D11_CREATE_DEVICE_DEBUG,
        featureLevels,
        1,
        D3D11_SDK_VERSION,
        &desc,
        &swapChain,
        &device,
        &featureLevel,
        &deviceContext
    );
    CheckHR("Device Error!", -1);

    ID3D11Texture2D* frameBuff;
    IDXGISwapChain_GetBuffer(swapChain, 0, &IID_ID3D11Texture2D, &frameBuff);
    hr = ID3D11Device_CreateRenderTargetView(device, frameBuff, NULL, &target);
    CheckHR("Render Target View Error!", -1);
    ID3D11Texture2D_Release(frameBuff);

    ByteDataAndSize vs;
    ByteDataAndSize ps;
    if (!readBinaryFile("D:\\vs\\c\\Project1\\fxc\\shader-bin\\vs.dxbc", &vs)) {
        Error("Vertex Shader Not Found", -1);
    }
    if (!readBinaryFile("D:\\vs\\c\\Project1\\fxc\\shader-bin\\ps.dxbc", &ps)) {
        Error("Pixel Shader Not Found", -1);
    }

    hr = ID3D11Device_CreateVertexShader(device, vs.bytes, vs.size, NULL, &vertexShader);
    CheckHR("Vertex Shader Error", -1);

    hr = ID3D11Device_CreatePixelShader(device, ps.bytes, ps.size, NULL, &pixelShader);
    CheckHR("Vertex Shader Error", -1);
    free(ps.bytes);
    
    hr = ID3D11Device_CreateInputLayout(device, elements, 2, vs.bytes, vs.size, &inputLayout);
    CheckHR("Input Layout Error", -1);
    free(vs.bytes);

    Vertex verts[3] = {
        {0.0f,  0.5f,  0.0f, 1.0f, 0.0f, 0.0f, 1.0f},
        {0.5f, -0.5f,  0.0f, 0.0f, 1.0f, 0.0f, 1.0f},
        {-0.5f, -0.5f,  0.0f, 0.0f, 0.0f, 1.0f, 1.0f},
    };
    UINT indicies[3] = { 0, 1, 2 };

    D3D11_BUFFER_DESC vertBufferDesc;
    ZeroMemory(&vertBufferDesc, sizeof(vertBufferDesc));
    vertBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
    vertBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    vertBufferDesc.ByteWidth = sizeof(verts);
    vertBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

    D3D11_SUBRESOURCE_DATA vertData;
    ZeroMemory(&vertData, sizeof(vertData));
    vertData.pSysMem = verts;
    hr = ID3D11Device_CreateBuffer(device, &vertBufferDesc, &vertData, &vertexBuffer);
    CheckHR("Vertex Buffer Creation Error!", -1);

    D3D11_BUFFER_DESC indexBufferDesc;
    ZeroMemory(&indexBufferDesc, sizeof(indexBufferDesc));
    indexBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
    indexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    indexBufferDesc.ByteWidth = sizeof(indicies);
    indexBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    D3D11_SUBRESOURCE_DATA indexData;

    ZeroMemory(&indexData, sizeof(indexData));
    indexData.pSysMem = indicies;
    hr = ID3D11Device_CreateBuffer(device, &indexBufferDesc, &indexData, &indexBuffer);
    CheckHR("Index Buffer Creation Error!", -1);

    UINT offset = 0;
    UINT stride = sizeof(Vertex);
    RECT winRect;
    GetClientRect(window, &winRect);
    D3D11_VIEWPORT viewport = { 0.0f, 0.0f, (FLOAT)(winRect.right - winRect.left), (FLOAT)(winRect.bottom - winRect.top), 0.0f, 1.0f };
    ID3D11DeviceContext_RSSetViewports(deviceContext, 1, &viewport);

    ID3D11DeviceContext_OMSetRenderTargets(deviceContext, 1, &target, NULL);

    ID3D11DeviceContext_IASetPrimitiveTopology(deviceContext, D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    ID3D11DeviceContext_IASetInputLayout(deviceContext, inputLayout);
    ID3D11DeviceContext_IASetVertexBuffers(deviceContext, 0, 1, &vertexBuffer, &stride, &offset);
    ID3D11DeviceContext_IASetIndexBuffer(deviceContext, indexBuffer, DXGI_FORMAT_R32_UINT, 0);

    ID3D11DeviceContext_VSSetShader(deviceContext, vertexShader, NULL, 0);
    ID3D11DeviceContext_PSSetShader(deviceContext, pixelShader, NULL, 0);
    MSG msg;
    while (1) {
        PeekMessage(&msg, NULL, 0, 0, PM_REMOVE);
        if (msg.message == WM_QUIT) {
            break;
        }
        TranslateMessage(&msg);
        DispatchMessage(&msg);

        float background_colour[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
        ID3D11DeviceContext_ClearRenderTargetView(deviceContext, target, background_colour);

        ID3D11DeviceContext_DrawIndexed(deviceContext, 3, 0, 0);
        IDXGISwapChain_Present(swapChain, 1, 0);
    }

    ID3D11Buffer_Release(indexBuffer);
    ID3D11Buffer_Release(vertexBuffer);

    ID3D11InputLayout_Release(inputLayout);

    ID3D11PixelShader_Release(pixelShader);
    ID3D11VertexShader_Release(vertexShader);

    ID3D11RenderTargetView_Release(target);
    IDXGISwapChain_Release(swapChain);
    ID3D11DeviceContext_Release(deviceContext);
    ID3D11Device_Release(device);
    
    return msg.wParam;
}