// ------------------------------------ //
#include "WindowRenderingResources.h"

#include "Graphics.h"

using namespace Leviathan;
// ------------------------------------ //
WindowRenderingResources::WindowRenderingResources(Graphics& owner) : Owner(&owner) {}
// ------------------------------------ //
DLLEXPORT void WindowRenderingResources::ResizeSwapChain(int newwidth, int newheight)
{
    if(WindowsSwapChain)
        WindowsSwapChain->Resize(newwidth, newheight);
}
// ------------------------------------ //
void WindowRenderingResources::Invalidate()
{
    Owner = nullptr;
    WindowsSwapChain = nullptr;
}

void WindowRenderingResources::Present(bool vsync)
{
    if(WindowsSwapChain)
        WindowsSwapChain->Present(vsync ? 1 : 0);
}
