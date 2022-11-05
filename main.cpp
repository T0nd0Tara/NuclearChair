#include "includes.hpp"

class NuclearChair : public olc::PixelGameEngine
{
public:
	NuclearChair()
	{
		sAppName = "NuclearChair";
	}
    
    ~NuclearChair()
    {
        ConsoleCaptureStdOut(false);
    }
private:
    olc::TransformedView tv;

    Mob player;

public:
	bool OnUserCreate() override
	{
        ConsoleCaptureStdOut(true);
        tv = olc::TileTransformedView({ ScreenWidth(), ScreenHeight() }, { RESOLUTION, RESOLUTION });
		return true;
	}

	bool OnUserUpdate(float fElapsedTime) override
	{
        if (GetKey(olc::ESCAPE).bPressed) return false;
        if (GetKey(olc::TAB).bPressed) 
            ConsoleShow(olc::TAB, true);

		return true;
	}

    bool OnConsoleCommand(const std::string& sText) override
    {

        return true;
    }
};


int main()
{
	NuclearChair demo;
	if (demo.Construct(256, 240, RESOLUTION, RESOLUTION))
		demo.Start();

	return 0;
}
