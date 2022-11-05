#include "includes.hpp"

#define PAN_SPEED 15.0f

class MapEditor : public olc::PixelGameEngine
{
public:
	MapEditor()
	{
		sAppName = "MapEditor";
	}
    
    ~MapEditor()
    {
        for (int i=1; i<vDBlocks.size(); i++)
        {
            delete vDBlocks[i]->sprite;
            delete vDBlocks[i];
        }
        ConsoleCaptureStdOut(false);
    }

private:
    olc::TransformedView tv;

    Map map;

    std::vector<olc::Decal*> vDBlocks;

    olc::vi2d vCurrCell{0,0};

    int nCurrDBlock = 0;

    bool bBGMode = false;
    bool bDrawGrid = true;
    

    void handleControls()
    {
        // Panning
        if (GetKey(olc::RIGHT).bHeld) tv.MoveWorldOffset(olc::vf2d{BLOCK_SIZE ,0} * PAN_SPEED * GetElapsedTime() / tv.GetWorldScale().x);
        if (GetKey(olc::LEFT).bHeld)  tv.MoveWorldOffset(olc::vf2d{-BLOCK_SIZE,0} * PAN_SPEED * GetElapsedTime() / tv.GetWorldScale().x);
        if (GetKey(olc::DOWN).bHeld)  tv.MoveWorldOffset(olc::vf2d{0, BLOCK_SIZE} * PAN_SPEED * GetElapsedTime() / tv.GetWorldScale().x);
        if (GetKey(olc::UP).bHeld)    tv.MoveWorldOffset(olc::vf2d{0,-BLOCK_SIZE} * PAN_SPEED * GetElapsedTime() / tv.GetWorldScale().x);
        
        if (GetMouse(0).bPressed) tv.StartPan(GetMousePos());
        if (GetMouse(0).bHeld) tv.UpdatePan(GetMousePos());
        if (GetMouse(0).bReleased) tv.EndPan(GetMousePos());

        if (GetKey(olc::MINUS).bPressed)  tv.ZoomAtScreenPos(0.5f, olc::vi2d{ScreenWidth(), ScreenHeight()} / 2);
        if (GetKey(olc::EQUALS).bPressed) tv.ZoomAtScreenPos(2.0f, olc::vi2d{ScreenWidth(), ScreenHeight()} / 2);

        // CurrCell Controls
        if (GetKey(olc::W).bPressed) vCurrCell.y -= 1;
        if (GetKey(olc::A).bPressed) vCurrCell.x -= 1;
        if (GetKey(olc::S).bPressed) vCurrCell.y += 1;
        if (GetKey(olc::D).bPressed) vCurrCell.x += 1;

        // Current Sprite
        if (GetKey(olc::Q).bPressed) nCurrDBlock--;
        if (GetKey(olc::E).bPressed) nCurrDBlock++;
        
        if (nCurrDBlock < 0) nCurrDBlock = vDBlocks.size() - 1;
        else nCurrDBlock = nCurrDBlock % vDBlocks.size();

        // BG Mode
        if (GetKey(olc::B).bPressed) bBGMode = !bBGMode;
    }
    
    void tvDrawRectDecal(olc::vf2d pos, olc::vf2d size, olc::Pixel col = olc::WHITE) 
    {
        tv.DrawLineDecal(pos, {pos.x + size.x, pos.y}, col);
        tv.DrawLineDecal({pos.x + size.x, pos.y}, pos + size, col);
        tv.DrawLineDecal({pos.x, pos.y + size.y}, pos + size, col);
        tv.DrawLineDecal(pos, {pos.x, pos.y + size.y}, col);
    }
public:
	bool OnUserCreate() override
	{
        ConsoleCaptureStdOut(true);
        tv = olc::TileTransformedView({ ScreenWidth(), ScreenHeight() }, { BLOCK_SIZE, BLOCK_SIZE });
        map.resize(ScreenWidth(), ScreenHeight());

        vDBlocks.push_back(nullptr);
        using std::filesystem::directory_iterator;
        for (const auto & file : directory_iterator("./Blocks")){
            std::string sPath = file.path();
            if (sPath.size() <= 4) continue;
            if (sPath.substr(sPath.size() - 4,4) != ".png") continue;
            vDBlocks.push_back(new olc::Decal(new olc::Sprite(sPath)));
        }
        
		return true;
	}

	bool OnUserUpdate(float fElapsedTime) override
	{
        if (GetKey(olc::ESCAPE).bPressed) return false;
        if (GetKey(olc::TAB).bPressed) 
            ConsoleShow(olc::TAB, true);

        Clear(olc::CYAN);

        // Controls
        if (!IsConsoleShowing())
        {   
            handleControls();
        }

        // Drawing
        olc::vf2d fTL = tv.GetWorldTL();
        olc::vf2d fBR = tv.GetWorldBR();
        olc::vi2d TL = fTL.floor();
        olc::vi2d BR = fBR.ceil();

        // Draw Grid
        if (bDrawGrid)
        {
            for (float x = float(TL.x / BLOCK_SIZE) * BLOCK_SIZE; x < BR.x; x+=BLOCK_SIZE)
               if (x != 0.0f)
                    tv.DrawLine({x, (float)TL.y}, {x, (float)BR.y}, olc::GREY);
            
            for (float y = float(TL.y / BLOCK_SIZE) * BLOCK_SIZE; y < BR.y; y+=BLOCK_SIZE)
               if (y != 0.0f)
                    tv.DrawLine({(float)TL.x, y}, {(float)BR.x, y}, olc::GREY);
            
            if (fTL.y < 0.0f && fBR.y > 0.0f)
                tv.DrawLine({(float)TL.x, 0.0f}, {(float)BR.x, 0.0f}, olc::RED);
            
            if (fTL.x < 0.0f && fBR.x > 0.0f)
                tv.DrawLine({0.0f, (float)TL.y}, {0.0f, (float)BR.y}, olc::RED);

        }

        // Draw Blocks
        for (int y = TL.y / BLOCK_SIZE; y <= BR.y / BLOCK_SIZE; y++)
            for (int x = TL.x / BLOCK_SIZE; x <= BR.x / BLOCK_SIZE; x++)
            {
                Block block = map.get(x,y);
                if (!block.bVisible) continue;
                if (block.nDecal == 0) continue;

                olc::Pixel tint = (block.bBG)? olc::GREY : olc::WHITE;
                tv.DrawDecal(olc::vf2d{(float)x, (float)y} * BLOCK_SIZE, vDBlocks[block.nDecal], olc::vf2d{1.0f,1.0f} * BLOCK_SIZE, tint);
            }

        if (vCurrCell.x < TL.x / BLOCK_SIZE) vCurrCell.x = TL.x / BLOCK_SIZE;
        if (vCurrCell.y < TL.y / BLOCK_SIZE) vCurrCell.y = TL.y / BLOCK_SIZE;
        if (vCurrCell.x > BR.x / BLOCK_SIZE) vCurrCell.x = BR.x / BLOCK_SIZE;
        if (vCurrCell.y > BR.y / BLOCK_SIZE) vCurrCell.y = BR.y / BLOCK_SIZE;

        // 2 times for contrust
        tvDrawRectDecal(olc::vf2d{(float)vCurrCell.x, (float)vCurrCell.y} * BLOCK_SIZE + olc::vf2d{0.1f,0.1f}, {(float)BLOCK_SIZE, BLOCK_SIZE}, olc::BLACK);
        tvDrawRectDecal(olc::vf2d{(float)vCurrCell.x, (float)vCurrCell.y} * BLOCK_SIZE, {(float)BLOCK_SIZE, BLOCK_SIZE});

        // Drawing BG Mode Label
        if (bBGMode) DrawStringDecal(olc::vf2d{1.0f,1.0f}, "BG MODE");

        // Drawing Preview Block
        olc::vi2d vBlockPos = olc::vi2d{ScreenWidth(), ScreenHeight()} - olc::vi2d{BLOCK_SIZE + 1, BLOCK_SIZE + 1};
        if (vDBlocks[nCurrDBlock])
            DrawDecal(vBlockPos, vDBlocks[nCurrDBlock]);
        else
            DrawRect(vBlockPos, olc::vi2d{BLOCK_SIZE - 1,BLOCK_SIZE - 1});
		return true;
	}

    bool OnConsoleCommand(const std::string& sText) override
    {
        std::stringstream ss(sText);
        std::string c1;
        ss >> c1;
        
        if (c1 == "grid")
        {
            std::string c2;
            ss >> c2;
            if (c2 == "off" || c2 == "0")
            {
                bDrawGrid = false;
                std::cout << "Turning off grid\n";
            }
            if (c2 == "on" || c2 == "1")
            {
                bDrawGrid = true;
                std::cout << "Turning on grid\n";
            }
            return true;
        }
        if (c1 == "save")
        {
            std::string c2;
            if (!(ss >> c2))
            {
                std::cout << "Didn't recive path... Include path!\n";
                return true;
            }

            map.shrink_to_fit();
            std::ofstream f(c2, std::ios::binary | std::fstream::trunc);
            if (!f.is_open())
            {
                std::cout << "Unable to open file `" << c2 << "`. For some reason.\n";
                return true;
            }
            f << map;
            f.close();
            
            std::cout << "Sucessfully saved `" << c2 << "`.\n";
            return true;

        }

        if (c1 == "load")
        {
            std::string c2;
            if (!(ss >> c2))
            {
                std::cout << "Didn't recive path... Include path!\n";
                return true;
            }
            map.clear();

            std::ifstream f(c2, std::ios::binary);
            if (!f.is_open())
            {
                std::cout << "Unable to open file `" << c2 << "`. For some reason.\n";
                return true;
            }
            
            f >> map;
            f.close();

            std::cout << "Sucessfully loaded `" << c2 << "`.\n";
            return true;
        }

        if (c1 == "clear" || c1 == "erase")
        {
            map.clear();
            return true;
        }

        std::cout << "Command '" << sText << "' not found.\n";

        return true;
    }
};


int main()
{
	MapEditor demo;
	if (demo.Construct(256, 240, 4, 4))
		demo.Start();

	return 0;
}
