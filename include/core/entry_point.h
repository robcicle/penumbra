#pragma once

extern penumbra::CApplication* penumbra::CreateApplication(
    ApplicationCommandLineArgs_t args);

int MainEntryPoint(int argc, char** argv)
{
    penumbra::CLog::Init();

    auto app = penumbra::CreateApplication({ argc, argv });
    app->Run();
    delete app;

    return 0;
}

#ifdef PENUMBRA_RELEASE

int APIENTRY WinMain(HINSTANCE hInst, HINSTANCE hInstPrev, PSTR cmdline, int cmdshow)
{
    return MainEntryPoint(__argc, __argv);
}

#else

int main(int argc, char** argv)
{
	return MainEntryPoint(argc, argv);
}

#endif