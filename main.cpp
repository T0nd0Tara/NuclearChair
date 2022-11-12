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
        for (int nType = 0; nType < magic_enum::enum_count<WT>(); nType++)
        {
            deleteVectorDecal(vWTDecals[nType], 1);
        }

        deleteVectorDecal(vBulletDecals);

        delete target.decal->sprite;
        delete target.decal;

        ConsoleCaptureStdOut(false);
    }
private:

    struct Target
    {
        olc::Decal* decal;
        olc::vf2d pos;
    };

    struct Bullet
    {
        int nDecal = -1;

        olc::vf2d pos;
        float fDir;
        bool bDamagePlayer  : 1 = false;
        bool bDamageEnemies : 1 = false;

        static std::vector<float> vBulletSpeed;
        static std::vector<int>   vBulletDamage;

        inline float getSpeed() const noexcept
        {
            if (nDecal < 0 || nDecal >= vBulletSpeed.size())
                return -1.0f;
            return vBulletSpeed[nDecal];
        }
        inline int getDamage() const noexcept
        {
            if (nDecal < 0 || nDecal >= vBulletDamage.size())
                return -1;
            return vBulletDamage[nDecal];
        }

        inline void move(float fElapsedTime)
        {
            if (nDecal == -1) return;
            pos += olc::vf2d{cosf(fDir), sinf(fDir)} * vBulletSpeed.at(nDecal) * fElapsedTime;
        }
    };



    olc::TransformedView tv;

    Map map;

    Mob& player = map.player;
    std::vector<Mob>& vMobs = map.vMobs;
    
    std::vector<std::vector<olc::Decal*>> vWTDecals;
    std::vector<olc::Decal*> vBulletDecals;

    // player moves fPlayerSpeed blocks in a second
    float fPlayerSpeed = 7.0f;

    Target target;

    std::vector<Bullet> vBullets;

    void handleControls()
    {
        const float& fElapsedTime = GetElapsedTime();

        if (GetKey(olc::MINUS).bPressed)  tv.ZoomAtScreenPos(0.5f, olc::vi2d{ScreenWidth(), ScreenHeight()} / 2);
        if (GetKey(olc::EQUALS).bPressed) tv.ZoomAtScreenPos(2.0f, olc::vi2d{ScreenWidth(), ScreenHeight()} / 2);

        // player controls
        if (player.nDecal == 1)
        {
            // move
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

            // shoot
            if (GetMouse(0).bPressed)
            {
                const olc::vf2d vDiff = target.pos - (player.pos + olc::vf2d{0.5f, 0.5f});
                const float fAngle = std::atan2(vDiff.y, vDiff.x);
                vBullets.emplace_back(1, player.pos + olc::vf2d{0.5f,0.5f}, fAngle, false, true);
            }


        }

    }

    void updateGame()
    {
        const float& fElapsedTime = GetElapsedTime();

        target.pos = tv.ScreenToWorld(GetMousePos()) / fBLOCK_SIZE;

        constexpr float fPlayerPerc = 0.2f;
        tv.SetWorldOffset(lerp(player.pos, target.pos, fPlayerPerc) * fBLOCK_SIZE - tv.GetWorldVisibleArea() / 2.0f);
        
        // update bullets
        for (int i=0; i<vBullets.size(); i++)
        {
            auto& b = vBullets[i];
            b.move(fElapsedTime);

            if (!map.get(b.pos.floor()).bBG)
            {
                vBullets.erase(vBullets.begin() + i);
                i--;
                continue;
            }

            bool bRemoveBullet = false;
            if (b.bDamageEnemies)
            {
                for (int nMob = 0; nMob < vMobs.size(); nMob++)
                {
                    auto& m = vMobs[nMob];
                    if (rectsOverlapping(m.pos * fBLOCK_SIZE, vBLOCK_SIZE, b.pos * fBLOCK_SIZE, vBULLET_SIZE))
                    {
                        m.nHp -= b.getDamage();
                        m.tint = pixelLerp(olc::RED, olc::WHITE, 0.3f);
                        bRemoveBullet = true;

                        // kill enemy
                        if (m.nHp <= 0)
                        {
                            vMobs.erase(vMobs.begin() + nMob);
                            nMob--;
                        }

                    }

                }
            }
            else if (b.bDamagePlayer)
            {
                if (rectsOverlapping(player.pos, vBLOCK_SIZE, b.pos, vBULLET_SIZE))
                {
                    player.nHp -= b.getDamage();
                    bRemoveBullet = true;

                    // kill player
                    if (player.nHp <= 0)
                    {
                        // TODO: kill player
                    }
                }               

            }
            if (bRemoveBullet)
            {
                vBullets.erase(vBullets.begin() + i);
                i--;
            }
        }

        // update mobs
        for (int i=0; i<vMobs.size(); i++)
        {
            auto& mob = vMobs[i];
            mob.updateTint(fElapsedTime);
        }

        
    }

    template <typename T>
    static inline T lerp(const T& a, const T& b, float t) 
    {
        return a + (b - a) * t;
    }
    

    static olc::Pixel pixelLerp(const olc::Pixel& a, const olc::Pixel& b, float t)
    {
        int32_t ar = a.r * a.r;
        int32_t ag = a.g * a.g;
        int32_t ab = a.b * a.b;
        int32_t aa = a.a * a.a;
        
        int32_t br = b.r * b.r;
        int32_t bg = b.g * b.g;
        int32_t bb = b.b * b.b;
        int32_t ba = b.a * b.a;
        
        int32_t r_ = ar + (br-ar)*t;
        int32_t g_ = ag + (bg-ag)*t;
        int32_t b_ = ab + (bb-ab)*t;
        int32_t a_ = aa + (ba-aa)*t;
        
        return olc::Pixel(std::sqrt(r_), std::sqrt(g_), std::sqrt(b_), std::sqrt(a_));
    }


    static void deleteVectorDecal(std::vector<olc::Decal*>& vec, int start = 0)
    {
        for (int i=start; i<vec.size(); i++)
        {
            delete vec[i]->sprite;
            delete vec[i];
        }
    }
    static inline bool pointInRect(olc::vf2d rectPos, olc::vf2d rectSize, olc::vf2d point)
    {
        return (rectPos.x <= point.x) && (point.x < rectPos.x + rectSize.x) &&
               (rectPos.y <= point.y) && (point.y < rectPos.y + rectSize.y);
    }
    static bool rectsOverlapping(olc::vf2d a_pos, olc::vf2d a_size, olc::vf2d b_pos, olc::vf2d b_size)
    {
        return pointInRect(a_pos, a_size, b_pos) ||
               pointInRect(a_pos, a_size, b_pos + olc::vf2d{b_size.x, 0.0f}) ||
               pointInRect(a_pos, a_size, b_pos + olc::vf2d{0.0f ,b_size.y}) ||
               pointInRect(a_pos, a_size, b_pos + b_size);
    }
public:
	bool OnUserCreate() override
	{
        ConsoleCaptureStdOut(true);
        tv = olc::TileTransformedView({ ScreenWidth(), ScreenHeight() }, { BLOCK_SIZE, BLOCK_SIZE });
        tv.SetZoom(0.5f, {0.0f,0.0f});


        // load decals
        // load world types
        for (int nType = 0; nType < magic_enum::enum_count<WT>(); nType++)
        {
            std::set<std::filesystem::path> files;
            std::string sDir = std::string(magic_enum::enum_name<WT>((WT)nType));
            std::transform(sDir.begin(), sDir.end(), sDir.begin(),
                [](unsigned char c){ return std::tolower(c); });

            for (const auto& file : std::filesystem::directory_iterator("world_types/" + sDir))
                files.insert(file.path());

            vWTDecals.emplace_back();
            vWTDecals[nType].push_back(nullptr);
            for (const auto & path : files){
                const std::string sPath = path;
                if (sPath.size() <= 4) continue;
                if (sPath.substr(sPath.size() - 4,4) != ".png") continue;
                vWTDecals[nType].push_back(new olc::Decal(new olc::Sprite(sPath)));
                if (nType == (int)WT::MOBS)
                {
                    Mob::vMobHp.push_back(100);

                    assert(vWTDecals[nType].back()->sprite->width  == MOB_FILE_SIZE.x);
                    assert(vWTDecals[nType].back()->sprite->height == MOB_FILE_SIZE.y);

                }
            }

        }

        // load bullets
        // TODO: make this expandable to more decals
        // TODO: add size assert
        std::set<std::filesystem::path> files;
        for (const auto& file : std::filesystem::directory_iterator("bullets/"))
            files.insert(file.path());

        for (const auto & path : files){
            const std::string sPath = path;
            if (sPath.size() <= 4) continue;
            if (sPath.substr(sPath.size() - 4,4) != ".png") continue;
            vBulletDecals.emplace_back(new olc::Decal(new olc::Sprite(sPath)));
            Bullet::vBulletSpeed.push_back(5.0f);
            Bullet::vBulletDamage.push_back(5.0f);

            assert(vBulletDecals.back()->sprite->width  == BULLET_FILE_SIZE.x);
            assert(vBulletDecals.back()->sprite->height == BULLET_FILE_SIZE.y);

        }

        // load target
        target.decal = new olc::Decal(new olc::Sprite("etc/target.png"));
        assert(target.decal->sprite->width  == TARGET_FILE_SIZE.x);
        assert(target.decal->sprite->height == TARGET_FILE_SIZE.y);

        // loading maps/01.bin
        OnConsoleCommand("load 01.bin");
        // std::ifstream f("maps/01.bin", std::ios::binary);
        // if (!f.is_open())
        // {
        //     std::cout << "Unable to open file `01.bin`. For some reason.\n";
        //     return true;
        // }
        //
        // f >> map;
        // f.close();

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
                tv.DrawDecal(olc::vf2d{(float)x, (float)y} * BLOCK_SIZE, vWTDecals[(int)WT::BLOCKS][block.nDecal], olc::vf2d{1.0f,1.0f} * BLOCK_SIZE, tint);
            }
        
        // draw mobs
        for (const auto& mob : vMobs)
        {
            olc::vi2d relSize = BLOCK_SIZE * olc::vi2d{1,1};
            if (!tv.IsRectVisible(mob.pos * BLOCK_SIZE, relSize)) continue;
            tv.DrawPartialDecal(mob.pos * BLOCK_SIZE, BLOCK_SIZE * relSize, vWTDecals[(int)WT::MOBS][mob.nDecal], olc::vf2d{0,0}, MOB_SIZE, mob.tint);
        }

        if (player.nDecal == 1)
        {
            // draw player
            tv.DrawPartialDecal(player.pos * BLOCK_SIZE, vWTDecals[(int)WT::MOBS][player.nDecal], olc::vf2d{0,0}, MOB_SIZE, BLOCK_SIZE * BLOCK_SIZE / MOB_SIZE, player.tint);
            // draw target
            tv.DrawDecal(target.pos * BLOCK_SIZE - olc::vi2d{BLOCK_SIZE, BLOCK_SIZE} / 2 , target.decal, TARGET_SIZE * fBLOCK_SIZE);
        }

        // draw bullets
        for (const auto& b: vBullets)
        {
            tv.DrawRotatedDecal(b.pos * fBLOCK_SIZE, vBulletDecals[b.nDecal], b.fDir, olc::vf2d{0.5f, 0.5f} * BULLET_SIZE, olc::vf2d{1.0f,1.0f} * BULLET_SIZE);
        }

        auto endTime = std::chrono::high_resolution_clock::now();
        float frameTime = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime).count();
        float fSleepFor = (1000.0f / MAX_FPS) - frameTime;
        // if (fSleepFor > 0.0f)
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

std::vector<float> NuclearChair::Bullet::vBulletSpeed{};
std::vector<int>   NuclearChair::Bullet::vBulletDamage{};

int main()
{
	NuclearChair demo;
	if (demo.Construct(256, 240, 4, 4))
		demo.Start();

	return 0;
}
