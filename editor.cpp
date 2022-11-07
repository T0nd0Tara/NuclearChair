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
        for (int nType = 0; nType < magic_enum::enum_count<WT>(); nType++)
        {
            for (int i=1; i<vDecals[nType].size(); i++)
            {
                delete vDecals[nType][i]->sprite;
                delete vDecals[nType][i];
            }

        }
        ConsoleCaptureStdOut(false);
    }

private:
    olc::TransformedView tv;

    Map map;

    Mob& player = map.player;
    std::vector<Mob>& vMobs = map.vMobs;

    std::vector<std::vector<olc::Decal*>> vDecals;

    olc::vi2d vCurrCell{0,0};

    int nCurrDecal = 0;
    int nCurrType = 0;

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
        if (GetKey(olc::Q).bPressed) nCurrDecal--;
        if (GetKey(olc::E).bPressed) nCurrDecal++;
        
        if (nCurrDecal < 0) nCurrDecal = vDecals[nCurrType].size() - 1;
        else nCurrDecal = nCurrDecal % vDecals[nCurrType].size();

        // BG Mode
        if (GetKey(olc::B).bPressed) bBGMode = !bBGMode;

        if (GetKey(olc::ENTER).bHeld)
        {
            switch(nCurrType)
            {
            case (int)WT::BLOCKS:
                map.set(vCurrCell.x, vCurrCell.y, Block(nCurrDecal, true, bBGMode));
                break;
            case (int)WT::MOBS:
            {
                auto it = std::find_if(vMobs.begin(), vMobs.end(), [this](const auto& mob){ return vCurrCell == mob.pos; });
                if (it != vMobs.end())
                    vMobs.erase(it);

                if (nCurrDecal == 0) break;
                if (nCurrDecal == 1)
                    player = Mob(vCurrCell, nCurrDecal, Mob::mobHp(nCurrDecal)); 
                else
                    vMobs.emplace_back(vCurrCell, nCurrDecal, Mob::mobHp(nCurrDecal));
                break;
            }
            default:
                assert(false && "Unimplemented type try to set itself");
            }
            vCurrCell = vCurrCell.max({0,0});
        }

        // fill area
        if (GetKey(olc::F).bPressed && nCurrType == (int)WT::BLOCKS)
        {
            map.fillArea(vCurrCell, nCurrDecal, true, bBGMode);
        }

        // change world type
        if (GetKey(olc::SPACE).bPressed)
        {
            nCurrType++;
            nCurrType = nCurrType % magic_enum::enum_count<WT>();
        }
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
        tv.SetZoom(0.5f, {0.0f,0.0f});

        nCurrType = magic_enum::enum_integer(WT::BLOCKS);

        // load decals
        for (int nType = 0; nType < magic_enum::enum_count<WT>(); nType++)
        {
            std::set<std::filesystem::path> files;
            std::string sDir = std::string(magic_enum::enum_name<WT>((WT)nType));
            std::transform(sDir.begin(), sDir.end(), sDir.begin(),
                [](unsigned char c){ return std::tolower(c); });

            for (const auto& file : std::filesystem::directory_iterator("./" + sDir))
                files.insert(file.path());

            vDecals.emplace_back();
            vDecals[nType].push_back(nullptr);
            for (const auto & path : files){
                const std::string sPath = path;
                if (sPath.size() <= 4) continue;
                if (sPath.substr(sPath.size() - 4,4) != ".png") continue;
                vDecals[nType].push_back(new olc::Decal(new olc::Sprite(sPath)));
                if (nType == (int)WT::MOBS)
                {
                    assert(vDecals[nType].back()->sprite->width  == MOB_SPRITE_WIDTH);
                    assert(vDecals[nType].back()->sprite->height == MOB_SPRITE_HEIGHT);

                }
            }

        }
		return true;
	}

	bool OnUserUpdate(float fElapsedTime) override
	{
        if (GetKey(olc::ESCAPE).bPressed) return false;
        if (GetKey(olc::TAB).bPressed) 
            ConsoleShow(olc::TAB, true);

        Clear(BG_COLOR);

        // Controls
        if (!IsConsoleShowing())
        {
            handleControls();
        }

        // Drawing
        const olc::vf2d fTL = tv.GetWorldTL();
        const olc::vf2d fBR = tv.GetWorldBR();
        const olc::vi2d TL  = fTL.floor();
        const olc::vi2d BR  = fBR.ceil();

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

                olc::Pixel tint = (block.bBG)? olc::DARK_GREY : olc::WHITE;
                tv.DrawDecal(olc::vf2d{(float)x, (float)y} * BLOCK_SIZE, vDecals[(int)WT::BLOCKS][block.nDecal], olc::vf2d{1.0f,1.0f} * BLOCK_SIZE, tint);
            }
        
        // draw player
        if (player.nDecal == 1)
            tv.DrawPartialDecal(player.pos * BLOCK_SIZE, vDecals[(int)WT::MOBS][player.nDecal], olc::vf2d{0,0}, MOB_SIZE, BLOCK_SIZE * BLOCK_SIZE / MOB_SIZE);
        // draw mobs
        for (const auto& mob : vMobs)
        {
            olc::vi2d relSize = BLOCK_SIZE * olc::vi2d{1,1};
            if (!tv.IsRectVisible(mob.pos * BLOCK_SIZE, relSize)) continue;
            tv.DrawPartialDecal(mob.pos * BLOCK_SIZE, BLOCK_SIZE * relSize, vDecals[(int)WT::MOBS][mob.nDecal], olc::vf2d{0,0}, MOB_SIZE);
        }

        if (vCurrCell.x < TL.x / BLOCK_SIZE) vCurrCell.x = TL.x / BLOCK_SIZE;
        if (vCurrCell.y < TL.y / BLOCK_SIZE) vCurrCell.y = TL.y / BLOCK_SIZE;
        if (vCurrCell.x > BR.x / BLOCK_SIZE) vCurrCell.x = BR.x / BLOCK_SIZE;
        if (vCurrCell.y > BR.y / BLOCK_SIZE) vCurrCell.y = BR.y / BLOCK_SIZE;

        // 2 times for contrust
        tvDrawRectDecal(olc::vf2d{(float)vCurrCell.x, (float)vCurrCell.y} * BLOCK_SIZE + olc::vf2d{0.1f,0.1f}, {(float)BLOCK_SIZE, BLOCK_SIZE}, olc::BLACK);
        tvDrawRectDecal(olc::vf2d{(float)vCurrCell.x, (float)vCurrCell.y} * BLOCK_SIZE, {(float)BLOCK_SIZE, BLOCK_SIZE});

        // Drawing World Type
        DrawStringDecal(olc::vf2d{1.0f,1.0f}, std::string(magic_enum::enum_name<WT>((WT)nCurrType)), olc::WHITE, olc::vf2d{1.2f,1.2f});
        // Drawing BG Mode Label
        if (bBGMode) DrawStringDecal(olc::vf2d{1.0f,16.0f}, "BG MODE");

        // Drawing Preview Block
        int prevSize = BLOCK_SIZE / 2;
        olc::vi2d vBlockPos = olc::vi2d{ScreenWidth(), ScreenHeight()} - olc::vi2d{prevSize + 1, prevSize + 1};
        const std::array<olc::vf2d, 4> pos{vBlockPos, vBlockPos + olc::vi2d{0, prevSize},vBlockPos + olc::vi2d{prevSize, prevSize}, vBlockPos + olc::vi2d{prevSize, 0}};
        switch (nCurrType)
        {
        case (int)WT::BLOCKS:
            if (vDecals[nCurrType][nCurrDecal])
                DrawWarpedDecal(vDecals[nCurrType][nCurrDecal], pos);
            else
                DrawRect(vBlockPos, olc::vi2d{prevSize - 1,prevSize - 1});
            break;
        case (int)WT::MOBS:
            if (vDecals[nCurrType][nCurrDecal])
                DrawPartialWarpedDecal(vDecals[nCurrType][nCurrDecal], pos, olc::vf2d{0,0}, MOB_SIZE);
            else
                DrawRect(vBlockPos, olc::vi2d{prevSize - 1,prevSize - 1});
            break;
        default:
            assert(false && "Unimplemented type try to show itself.");
        }
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
            else if (c2 == "on" || c2 == "1")
            {
                bDrawGrid = true;
                std::cout << "Turning on grid\n";
            }
            else std::cout << "Unknown subcommand '" << c2 << "' for command 'grid'\n";
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
            std::ofstream f(MAP_DIR + c2, std::ios::binary | std::fstream::trunc);
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

            std::ifstream f(MAP_DIR + c2, std::ios::binary);
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
            std::string c2;
            ss >> c2;
            if (c2 == "all")
            {
                map.vMobs.clear();
                map.player.nDecal = 0;
            }
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
