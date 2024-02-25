#include <Windows.h>
#include <minitest.h>
int WINAPI WinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPSTR lpCmdLine, _In_ int nShowCmd)
{
    MINITEST_WIN32_RUN_TEST();
    MessageBoxW(NULL, L"Hello, Windows!", L"Hello", MB_OK);
    return 0;
} 

TEST_CASE("Win32")
{
    EXPECT_TRUE(true);
    EXPECT_FALSE(false);
}