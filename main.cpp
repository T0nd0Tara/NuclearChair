#include "includes.hpp"

struct Target
{
    olc::Decal* decal;
    olc::vf2d pos;
};

class NuclearChair : public olc::PixelGameEngine
{
public:
	NuclearChair()
	{
		sAppName = "NuclearChair";
	}
    
    ~NuclearChair()
    {
        for (int nType = 0; nType < magic_enum::enum_count<WT>(); nType++)
        {
            for (int i=1; i<vDecals[nType].size(); i++)
            {
                delete vDecals[nType][i]->sprite;
                delete vDecals[nType][i];
            }

        }

        delete target.decal->sprite;
        delete target.decal;

        ConsoleCaptureStdOut(false);
    }
private:
    olc::TransformedView tv;

    Map map;

    Mob& player = map.player;
    std::vector<Mob>& vMobs = map.vMobs;
    
    std::vector<std::vector<olc::Decal*>> vDecals;

    // player moves fPlayerSpeed blocks in a second
    float fPlayerSpeed = 7.0f;

    Target target;

    void handleControls()
    {
        const float& fElapsedTime = GetElapsedTime();

        if (GetKey(olc::MINUS).bPressed)  tv.ZoomAtScreenPos(0.5f, olc::vi2d{ScreenWidth(), ScreenHeight()} / 2);
        if (GetKey(olc::EQUALS).bPressed) tv.ZoomAtScreenPos(2.0f, olc::vi2d{ScreenWidth(), ScreenHeight()} / 2);

        // player controls
        if (player.nDecal == 1)
        {
            olc::vf2d vMoveDir{0,0};
            if (GetKey(olc::W).bHeld) vMoveDir.y -= 1.0f;
            if (GetKey(olc::A).bHeld) vMoveDir.x -= 1.0f;
            if (GetKey(olc::S).bHeld) vMoveDir.y += 1.0f;
            if (GetKey(olc::D).bHeld) vMoveDir.x += 1.0f;

            float mag = vMoveDir.mag();
            if (mag > 0.0f)
            {
                olc::vf2d diffPos = fElapsedTime * fPlayerSpeed * vMoveDir / mag;
                auto newPos = [&](){ return player.pos + diffPos; };

                constexpr float fPadding = 0.15;
                constexpr float fOpPadding = 1.0f - fPadding;
                static_assert(fPadding > 0.0f);

                if (diffPos.x < 0.0f) // moving left
                {
                    if (!map.get(newPos().x, player.pos.y).bBG || !map.get(newPos().x, player.pos.y + fOpPadding).bBG)
                        diffPos.x = 0.0f; 
                }
                else if (diffPos.x > 0.0f) // moving right
                {
                    if (!map.get(newPos().x + 1.0f, player.pos.y).bBG || !map.get(newPos().x + 1.0f, player.pos.y + fOpPadding).bBG)
                        diffPos.x = 0.0f;
                }
                if (diffPos.y < 0.0f) // moving up
                {
                    if (!map.get(player.pos.x, newPos().y).bBG || !map.get(player.pos.x + fOpPadding, newPos().y).bBG)
                        diffPos.y = 0.0f;
                }
                else if (diffPos.y > 0.0f) // moving down
                {
                    if (!map.get(player.pos.x, newPos().y + 1.0f).bBG || !map.get(player.pos.x + fOpPadding, newPos().y + 1.0f).bBG)
                        diffPos.y = 0.0f;
                }
                player.pos = newPos();

            }
        }

    }

    void updateGame()
    {
        // const float& fElapsedTime = GetElapsedTime();

        target.pos = tv.ScreenToWorld(GetMousePos()) / fBLOCK_SIZE;

        constexpr float fPlayerPerc = 0.2f;
        tv.SetWorldOffset(lerp(player.pos, target.pos, fPlayerPerc) * fBLOCK_SIZE - olc::vf2d{(float)ScreenWidth(), (float)ScreenHeight()} );

        
    }
    template <typename T>
    inline static T lerp(const T& a, const T& b, float t)
    {
        return a + (b - a) * t;
    }
public:
	bool OnUserCreate() override
	{
        ConsoleCaptureStdOut(true);
        tv = olc::TileTransformedView({ ScreenWidth(), ScreenHeight() }, { BLOCK_SIZE, BLOCK_SIZE });
        tv.SetZoom(0.5f, {0.0f,0.0f});

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

        target.decal = new olc::Decal(new olc::Sprite("etc/target.png"));
        assert(target.decal->sprite->width  == TARGET_FILE_SIZE.x);
        assert(target.decal->sprite->height == TARGET_FILE_SIZE.y);

        // loading maps/01.bin
        std::ifstream f("maps/01.bin", std::ios::binary);
        if (!f.is_open())
        {
            std::cout << "Unable to open file `01.bin`. For some reason.\n";
            return true;
        }

        f >> map;
        f.close();

		return true;
	}

	bool OnUserUpdate(float fElapsedTime) override
	{
        auto startTime = std::chrono::high_resolution_clock::now();

        if (GetKey(olc::ESCAPE).bPressed) return false;
        if (GetKey(olc::TAB).bPressed) 
            ConsoleShow(olc::TAB, true);

        Clear(BG_COLOR);

        // Controls
        if (!IsConsoleShowing())
        {
            handleControls();
            updateGame();
        }

        // Drawing
        const olc::vf2d fTL = tv.GetWorldTL();
        const olc::vf2d fBR = tv.GetWorldBR();
        const olc::vi2d TL  = fTL.floor();
        const olc::vi2d BR  = fBR.ceil();


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
        
        // draw mobs
        for (const auto& mob : vMobs)
        {
            olc::vi2d relSize = BLOCK_SIZE * olc::vi2d{1,1};
            if (!tv.IsRectVisible(mob.pos * BLOCK_SIZE, relSize)) continue;
            tv.DrawPartialDecal(mob.pos * BLOCK_SIZE, BLOCK_SIZE * relSize, vDecals[(int)WT::MOBS][mob.nDecal], olc::vf2d{0,0}, MOB_SIZE);
        }

        // draw player
        if (player.nDecal == 1)
        {
            // tv.FillRectDecal(player.pos * BLOCK_SIZE, BLOCK_SIZE * BLOCK_SIZE / MOB_SIZE);
            tv.DrawPartialDecal(player.pos * BLOCK_SIZE, vDecals[(int)WT::MOBS][player.nDecal], olc::vf2d{0,0}, MOB_SIZE, BLOCK_SIZE * BLOCK_SIZE / MOB_SIZE);
            tv.DrawDecal(target.pos * BLOCK_SIZE - olc::vi2d{BLOCK_SIZE, BLOCK_SIZE} / 2 , target.decal, TARGET_SIZE * fBLOCK_SIZE);
        }

        auto endTime = std::chrono::high_resolution_clock::now();
        float frameTime = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime).count();
        float fSleepFor = (1000.0f / MAX_FPS) - frameTime;
        if (fSleepFor > 0.0f)
            std::this_thread::sleep_for(std::chrono::milliseconds((int)fSleepFor));
        
		return true;
	}

    bool OnConsoleCommand(const std::string& sText) override
    {
        std::stringstream ss(sText);
        std::string c1;
        ss >> c1;
        
        if (c1 == "load" || c1 == "laod")
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
            }
            return true;
        }

        std::cout << "Command '" << sText << "' not found.\n";

        return true;
    }
};


int main()
{
	NuclearChair demo;
	if (demo.Construct(256, 240, 4, 4))
		demo.Start();

	return 0;
}
