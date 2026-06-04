#include "DxLib.h"
#include <stdlib.h>
#include <cmath>
#include <cstring>
#include <ctime>
#include <vector>
#include <string>
#include <algorithm>

#pragma region Constants Definition

#pragma region Constants
/// <summary>
/// 定数定義
/// </summary>
const int WIDTH = 960;
const int HEIGHT = 640;
const int RACKET_W = 120;
const int RACKET_H = 12;
const int BALL_R = 10;
const int INITIAL_BALL_VX = 7;
const int INITIAL_BALL_VY = 7;
const int INITIAL_RACKET_Y = HEIGHT - 50;
const int INITIAL_HP = 3;

// 敵関連の定数
const int ENEMY_W = 120;    // 敵の幅
const int ENEMY_H = 30;     // 敵の高さ
const int ENEMY_COLUMNS = 6;    // 一列の敵の数
const int CRITICAL_LINE_Y = HEIGHT - 50;   // ゲームオーバーライン
const float ENEMY_DOWN_SPEED = 0.2f;    // 降下速度
const int ENEMY_SPAWN_INTERVAL = 300;   // 生成スパン（60fps * 5）

// 弾関連の定数
const int SHOT_W = 8;   // 弾の幅
const int SHOT_H = 8;  // 弾の高さ
const int SHOT_SPEED_X = 5;     // 弾の横速度
const int SHOT_SPEED_Y = 5;     // 弾の縦速度
const int SHOT_LIFESPAN = 1200;   // 弾の寿命（60fps * 20s）
const int SHOT_COOLDOWN = 120;   // 発射間隔

// ステージ関連の定数
const int WAVES_PER_STAGE = 5;  // 1ステージ当たりの敵の列数
const int MAX_STAGES = 3;
const int STAGE_CLEAR_TIME = 180;   // ステージクリア演出の時間（3秒）

/// <summary>
/// カラー定義
/// </summary>
#define COLOR_RED       (unsigned int)0xff0000
#define COLOR_GREEN     (unsigned int)0x00ff00
#define COLOR_BLUE      (unsigned int)0x0080ff
#define COLOR_CYAN      (unsigned int)0x00ffff
#define COLOR_YELLOW    (unsigned int)0xffff00
#define COLOR_WHITE     (unsigned int)0xffffff
#define COLOR_DARKBLUE  (unsigned int)0x204080
#define COLOR_LIGHTBLUE (unsigned int)0x40c0ff
#define COLOR_PINK      (unsigned int)0xffa0a0
#pragma endregion

#pragma region Ball Class
/// <summary>
/// ボールクラス
/// </summary>
class Ball {
private:
    int x, y;
    int vx, vy;
    int r;

public:
    Ball(int startX, int startY, int radius, int initialVx, int initialVy)
        : x(startX), y(startY), r(radius), vx(initialVx), vy(initialVy) { }

    // 初期状態に戻す
    void reset(int startX, int startY, int initialVx, int initialVy)
    {
        x = startX;
        y = startY;
        vx = initialVx;
        vy = initialVy;
    }

    // 更新処理
    void update()
    {
        x += vx;
        y += vy;

        // 左右の壁との衝突判定
        if (x < r && vx < 0)
            vx = -vx;
        else if (x > WIDTH - r && vx > 0)
            vx = -vx;

        // 上の壁との衝突判定
        if (y < r && vy < 0)
            vy = -vy;
    }

    // 描画処理
    void draw() const
    {
        DrawCircle(x, y, r, COLOR_RED, TRUE);
    }

    int getX() const { return x; }
    int getY() const { return y; }
    int getR() const { return r; }
    int getVX() const { return vx; }
    int getVY() const { return vy; }
    void setVX(int newVx) { vx = newVx; }
    void setVY(int newVy) { vy = newVy; }
    void reverseVX() { vx = -vx; }   // X方向の速度を反転
    void reverseVY() { vy = -vy; }   // Y方向の速度を反転
};
#pragma endregion

#pragma region Shot Class
/// <summary>
/// 弾クラス
/// </summary>
class Shot
{
private:
    float x, y;
    int w, h;
    int vx, vy;
    int lifespan;   // 弾の寿命

public:
    Shot(float startX, float startY, int startVX, int startVY, int life)
        : x(startX), y(startY), w(SHOT_W), h(SHOT_H), vx(startVX), vy(startVY), lifespan(life) { }

    // 更新処理
    void update()
    {
        x += vx;
        y += vy;
        lifespan--;

        // 左右の壁との衝突判定
        if (x - w / 2 < 0 && vx < 0)
            vx = -vx;
        else if (x + w / 2 > WIDTH && vx > 0)
            vx = -vx;

        // 上下の壁との衝突判定
        if (y - h / 2 < 0 && vy < 0)
            vy = -vy;
        else if (y + h / 2 > HEIGHT && vy > 0)
            vy = -vy;
    }

    // 描画処理
    void draw() const
    {
        if (isAlive())
            DrawBox((int)x - w / 2, (int)y - h / 2, (int)x + w / 2, (int)y + h / 2, COLOR_YELLOW, TRUE);
    }

    // 生存確認
    bool isAlive() const
    {
        return lifespan > 0;  
    }

    // ヒット
    void hit()
    {
        lifespan = 0;   
    }
    
    // 衝突判定用に座標とサイズを返す
    int getX() const { return (int)x; }
    int getY() const { return (int)y; }
    int getW() const { return w; }
    int getH() const { return h; }
};
#pragma endregion

#pragma region Enemy Class
/// <summary>
/// 敵・ブロッククラス
/// </summary>
class Enemy
{
private:
    float x, y;
    int w, h;
    int health;
    unsigned int color;

public:
    Enemy(int startX, int startY, int hp)
        : x((float)startX), y((float)startY), w(ENEMY_W), h(ENEMY_H), health(hp) {
        color = (health == 2) ? GetColor(255, 100, 100) : GetColor(100, 255, 100);
    }

    // 降下
    void move(float speed)
    {
        y += speed;
    }

    void draw() const
    {
        DrawBox((int)x - w / 2, (int)y - h / 2, (int)x + w / 2, (int)y + h / 2, color, TRUE);
    }

    bool hit()
    {
        health--;
        if (health == 1)
            color = GetColor(100, 255, 100);
        return health <= 0;
    }

    int getX() const { return (int)x; }
    int getY() const { return (int)y; }
    int getW() const { return w; }
    int getH() const { return h; }
};
#pragma endregion

#pragma region Racket Class
/// <summary>
/// ラケットクラス
/// </summary>
class Racket
{
private:
    int x, y;
    int w, h;

public:
    Racket(int startX, int startY, int width, int height)
        : x(startX), y(startY), w(width), h(height) { }

    // 初期状態に戻す
    void reset(int startX)
    {
        x = startX;
    }

    // 更新処理
    void update()
    {
        // キー入力による移動
        if (CheckHitKey(KEY_INPUT_LEFT) == 1)
            x -= 10;
        if (CheckHitKey(KEY_INPUT_RIGHT) == 1)
            x += 10;

        // 画面端の制限
        if (x < w / 2)
            x = w / 2;
        if (x > WIDTH - w / 2)
            x = WIDTH - w / 2;
    }

    // 描画処理
    void draw() const
    {
        DrawBox(x - w / 2, y - h / 2, x + w / 2, y + h / 2, COLOR_BLUE, TRUE);
    }

    // ヒットチェック関数
    bool checkHit(Ball& ball, int& score, int& highScore, int seHandle)
    {
        int dx = ball.getX() - x;   // x軸方向の距離
        int dy = ball.getY() - y;   // y軸方向の距離

        // 衝突範囲：ラケットの中央から、x軸方向にラケット幅+ボール半径、y軸方向に上端から-20~0の範囲
        if (-w / 2 - BALL_R < dx && dx < w / 2 + BALL_R && -20 < dy && dy < 0)
        {
            if (ball.getVY() > 0)   // ボールが下降中（ラケットに向かっている）
            {
                // ボールのＹ方向速度を反転し、ランダムな要素を追加
                int currentSpeed = abs(ball.getVY());

                int newSpeed = currentSpeed;
                if (newSpeed < 1) newSpeed = 1;

                ball.setVY(-newSpeed);

                // スコア更新
                score += 100;
                if (score > highScore) highScore = score;

                // 効果音
                PlaySoundMem(seHandle, DX_PLAYTYPE_BACK);
                return true;
            }
        }
        return false;
    }

    int getX() const { return x; }
    int getY() const { return y; }
    int getW() const { return w; }
    int getH() const { return h; }
};
#pragma endregion

#pragma region GameScene Class
/// <summary>
/// ゲームシーン管理クラス
/// </summary>
class GameScene
{
public:
    enum GameState
    {
        STATE_TITLE,
        STATE_PLAY,
        STATE_STAGE_CLEAR,
        STATE_GAME_OVER,
        STATE_GAME_CLEAR,
        STATE_GALLERY
    };

private:
    GameState state;
    int timer;
    int score;
    int highScore;
    int hp;
    int enemySpawnTimer;
    bool isBallLaunched;    // ボールが発射されたか

    int currentStage;
    int wavesLeftInStage;
    int stageClearTimer;

    char keys[256];
    char oldKeys[256];

    Ball ball;
    Racket racket;
    std::vector<Enemy> enemies; // 敵のコンテナ
    std::vector<Shot> shots;    // 弾のコンテナ
    int shotCooldownTimer;      // 弾の発射クールダウン

    int imgLocked;      // 「？」ロック中の画像
    std::vector<int> imgDogGallery; // 犬の画像ハンドルを保持
    std::vector<bool> unlockedDogs; // 犬のアンロック状態
    int galleryPage;    // 図鑑の現在ページ

    // 画像・サウンドハンドル
    int imgBg;
    int bgm;
    int jingl;
    int se; // 敵ヒット音

public:
    GameScene()
        : state(STATE_TITLE), timer(0), score(0), highScore(1000), hp(INITIAL_HP),
        currentStage(1), wavesLeftInStage(0), stageClearTimer(0),
        shotCooldownTimer(0),
        ball(40, 80, BALL_R, INITIAL_BALL_VX, INITIAL_BALL_VY),
        racket(WIDTH / 2, INITIAL_RACKET_Y, RACKET_W, RACKET_H)
    {
        memset(oldKeys, 0, 256);

        // 図鑑用の変数
        galleryPage = 0;

        // 図鑑アセット
        imgLocked = LoadGraph("image/question.png");

        imgDogGallery.push_back(LoadGraph("image/dach.png"));
        imgDogGallery.push_back(LoadGraph("image/shiba.png"));
        imgDogGallery.push_back(LoadGraph("image/golden.png"));

        unlockedDogs.resize(MAX_STAGES, false);

        // リソース読み込み
        imgBg = LoadGraph("image/background.png");
        bgm = LoadSoundMem("sound/bgm.mp3");
        jingl = LoadSoundMem("sound/gameSCENE_OVER.mp3");
        se = LoadSoundMem("sound/hit.mp3");

        // 音量設定
        ChangeVolumeSoundMem(128, bgm);
        ChangeVolumeSoundMem(128, jingl);
    }

    ~GameScene() { }

    // ゲーム全体をリセット（TITLE ~ PLAY）
    void resetGame()
    {
        racket.reset(WIDTH / 2);
        ball.reset(racket.getX(),   // ラケットのX座標
            racket.getY() - racket.getH() / 2 - BALL_R - 1, // ラケットのY座標
            0, 0);  // 速度0
        score = 0;
        hp = INITIAL_HP;
        timer = 0;
        enemySpawnTimer = 0;
        enemies.clear();
        shots.clear();
        shotCooldownTimer = 0;

        currentStage = 1;
        wavesLeftInStage = WAVES_PER_STAGE;
        state = STATE_PLAY;
        isBallLaunched = false;
        PlaySoundMem(bgm, DX_PLAYTYPE_LOOP);
    }

    // プレイ中のゲーム要素をリセット（ミスした時）
    void resetPlay()
    {
        racket.reset(WIDTH / 2);
        ball.reset(racket.getX(),   // ラケットのX座標
            racket.getY() - racket.getH() / 2 - BALL_R / 1,     // ラケットのY座標
            0, 0);  // 速度0
        isBallLaunched = false;

        hp--;
        if (hp > 0)
            state = STATE_PLAY;
        else
        {
            state = STATE_GAME_OVER;
            timer = 0;
            StopSoundMem(bgm);
            PlaySoundMem(jingl, DX_PLAYTYPE_BACK);
        }
    }

    // 敵の列を生成
    void spawnEnemyRow()
    {
        // 画面幅から敵の配置開始位置を計算（中央揃え）
        int total_width = ENEMY_COLUMNS * ENEMY_W;
        int start_x = (WIDTH - total_width) / 2 + ENEMY_W / 2;

        for (int i = 0; i < ENEMY_COLUMNS; ++i)
        {
            int enemy_x = start_x + i * ENEMY_W;
            int health = (rand() % 100 < 20) ? 2 : 1;
            // 画面外上部に生成
            enemies.emplace_back(enemy_x, -ENEMY_H / 2, health);
        }
    }

    // ボールと敵の衝突判定と処理
    void checkEnemyCollision()
    {
        int ballX = ball.getX();
        int ballY = ball.getY();
        int ballR = ball.getR();

        for (auto it = enemies.begin(); it != enemies.end(); )
        {
            Enemy& enemy = *it;
            int enemyX = enemy.getX();
            int enemyY = enemy.getY();
            int enemyW = enemy.getW();
            int enemyH = enemy.getH();

            // 簡易的な矩形衝突判定
            bool collisionX = ballX + ballR > enemyX - enemyW / 2 && ballX - ballR < enemyX + enemyW / 2;
            bool collisionY = ballY + ballR > enemyY - enemyH / 2 && ballY - ballR < enemyY + enemyH / 2;

            if (collisionX && collisionY)
            {
                // ボール反射のロジック
                if ((ball.getVY() > 0 && ballY - ballR <= enemyY - enemyH / 2) ||   // 上面衝突
                    (ball.getVY() < 0 && ballY + ballR >= enemyY + enemyH / 2))     // 下面衝突
                    ball.reverseVY();
                else    // 側面衝突
                    ball.reverseVX();

                // 敵にダメージを与え、破壊されたかチェック
                if (enemy.hit())
                {
                    score += 500;   // 破壊ボーナス
                    it = enemies.erase(it); // 敵を削除
                    PlaySoundMem(se, DX_PLAYTYPE_BACK);
                }
                else
                {
                    score += 100;   // ヒットボーナス
                    PlaySoundMem(se, DX_PLAYTYPE_BACK);
                    ++it;
                }
            }
            else
            {
                ++it;
            }
        }
    }

    // 弾と敵の衝突判定
    void checkShotCollision()
    {
        // 全ての弾をチェック
        for (auto& shot : shots)
        {
            if (!shot.isAlive()) continue;  // 死んでいる弾はスキップ

            // 全ての敵をチェック
            for (auto it = enemies.begin(); it != enemies.end(); )
            {
                Enemy& enemy = *it;

                // 弾と敵の簡易的な矩形衝突判定
                int shotX = shot.getX();
                int shotY = shot.getY();
                int shotW = shot.getW();
                int shotH = shot.getH();

                int enemyX = enemy.getX();
                int enemyY = enemy.getY();
                int enemyW = enemy.getW();
                int enemyH = enemy.getH();

                bool collisionX = (shotX + shotW / 2 > enemyX - enemyW / 2) && (shotX - shotW / 2 < enemyX + enemyW / 2);
                bool collisionY = (shotY + shotH / 2 > enemyY - enemyH / 2) && (shotY - shotH / 2 < enemyY + enemyH / 2);

                if (collisionX && collisionY)
                {
                    shot.hit();

                    // 弾にダメージを与え、破壊されたかチェック
                    if (enemy.hit())
                    {
                        score += 500;
                        it = enemies.erase(it);
                        PlaySoundMem(se, DX_PLAYTYPE_BACK);
                    }
                    else
                    {
                        score += 100;
                        PlaySoundMem(se, DX_PLAYTYPE_BACK);
                        ++it;
                    }

                    break;
                }
                else
                {
                    ++it;   // 当たっていないので次の敵へ
                }
            }
        }
    }

    // 次のステージを開始
    void startNextStage()
    {
        currentStage++;

        // ステージが上がるごとに難易度アップ
        wavesLeftInStage = WAVES_PER_STAGE + (currentStage - 1) * 2;

        state = STATE_PLAY;

        // ボールとラケットを初期位置に戻す
        racket.reset(WIDTH / 2);
        ball.reset(racket.getX(),
            racket.getY() - racket.getH() / 2 - BALL_R - 1,
            0, 0);
        isBallLaunched = false;

        // 弾と敵をクリア
        shots.clear();
        enemies.clear();

        // タイマーリセット
        enemySpawnTimer = 0;
        shotCooldownTimer = 0;

        PlaySoundMem(bgm, DX_PLAYTYPE_LOOP);
    }

    // 更新処理
    void update()
    {
        memcpy(oldKeys, keys, 256);
        GetHitKeyStateAll(keys);

        timer++;
        if (shotCooldownTimer > 0) shotCooldownTimer--;

        switch (state)
        {
        case STATE_TITLE:
            if (isKeyTriggered(KEY_INPUT_SPACE) == 1)
            {
                resetGame();
            }
            if (isKeyTriggered(KEY_INPUT_G))
            {
                galleryPage = 0;    // ギャラリーを開く時は1ページ目から
                state = STATE_GALLERY;
            }
            break;

        case STATE_PLAY:
            enemySpawnTimer++;

            // 敵の出現
            if (enemySpawnTimer >= ENEMY_SPAWN_INTERVAL)
            {
                if (wavesLeftInStage > 0)
                {
                    spawnEnemyRow();
                    enemySpawnTimer = 0;
                    wavesLeftInStage--;
                }
            }

            // 敵の移動と敗北判定
            for (auto& enemy : enemies)
            {
                enemy.move(ENEMY_DOWN_SPEED);
                if (enemy.getY() + enemy.getH() / 2 >= CRITICAL_LINE_Y)
                {
                    // 敵がクリティカルラインを超えたらゲームオーバー
                    state = STATE_GAME_OVER;
                    timer = 0;
                    StopSoundMem(bgm);
                    PlaySoundMem(jingl, DX_PLAYTYPE_BACK);
                    return;
                }
            }

            racket.update();
            
            if (!isBallLaunched)
            {
                // 発射前：ボールをラケットの中央に追従
                ball.reset(racket.getX(),
                    racket.getY() - racket.getH() / 2 - BALL_R - 1,
                    0, 0);

                // 上矢印キーが押されたら発射
                if (isKeyTriggered(KEY_INPUT_UP) == 1)
                {
                    ball.setVX(INITIAL_BALL_VX);    // 初期速度X
                    ball.setVY(-INITIAL_BALL_VY);   // 初期速度Y
                    isBallLaunched = true;
                }
            }
            else
            {
                // 発射後：ボールとラケットの処理
                ball.update();
                racket.checkHit(ball, score, highScore, se);

                // ボールと敵の衝突処理
                checkEnemyCollision();

                // 弾の発射
                if (shotCooldownTimer <= 0)
                {
                    int startVX = (rand() % 2 == 0) ? SHOT_SPEED_X : -SHOT_SPEED_X;

                    // ラケットの中央上部から発射
                    shots.emplace_back(
                        (float)racket.getX(),
                        (float)racket.getY() - racket.getH() / 2,
                        startVX,
                        -SHOT_SPEED_Y,
                        SHOT_LIFESPAN
                    );
                    shotCooldownTimer = SHOT_COOLDOWN;
                }

                // 弾の更新
                for (auto& shot : shots)
                    shot.update();

                // 弾と敵の衝突判定
                checkShotCollision();

                // 寿命が尽きた弾をリストから削除
                shots.erase(
                    std::remove_if(shots.begin(), shots.end(),
                        [](const Shot& s) { return !s.isAlive(); }
                    ),
                    shots.end()
                );

                // ボールが下端を通過したか
                if (ball.getY() > HEIGHT)
                {
                    resetPlay();
                }
            }

            // ステージクリア判定
            // 残りウェーブ0、画面上の敵が0、敵の出現タイマーが待機状態になってから一定時間
            if (wavesLeftInStage <= 0 && enemies.empty() && enemySpawnTimer > 60)
            {
                StopSoundMem(bgm);

                state = STATE_STAGE_CLEAR;
                stageClearTimer = STAGE_CLEAR_TIME;
            }
            break;

        case STATE_STAGE_CLEAR:
            stageClearTimer--;
            if (stageClearTimer <= 0)
            {
                // 犬をアンロック
                if (currentStage - 1 < (int)unlockedDogs.size())
                {
                    unlockedDogs[currentStage - 1] = true;
                }

                if (currentStage >= MAX_STAGES)
                {
                    state = STATE_GAME_CLEAR;
                    timer = 0;
                }
                else
                {
                    startNextStage();
                }
            }
            break;

        case STATE_GAME_CLEAR:
            if (isKeyTriggered(KEY_INPUT_SPACE) == 1)
                state = STATE_TITLE;
            break;

        case STATE_GAME_OVER:
            if (isKeyTriggered(KEY_INPUT_SPACE) == 1)
                resetGame();    // リセットしてSCENE_PLAYへ
            if (isKeyTriggered(KEY_INPUT_T) == 1)
                state = STATE_TITLE;    // タイトルへ
            break;

        case STATE_GALLERY:
            // 左右キーでページめくり
            if (isKeyTriggered(KEY_INPUT_LEFT))
            {
                galleryPage--;
                if (galleryPage < 0) galleryPage = 0;
            }
            if (isKeyTriggered(KEY_INPUT_RIGHT))
            {
                galleryPage++;
                if (galleryPage >= MAX_STAGES) galleryPage = MAX_STAGES - 1;
            }

            // Tキーでタイトルに戻る
            if (isKeyTriggered(KEY_INPUT_T)) state = STATE_TITLE;

            break;
        }
    }

    // 描画処理
    void draw() const
    {
        SetDrawBlendMode(DX_BLENDMODE_ALPHA, 125);
        DrawGraph(0, 0, imgBg, TRUE);
        SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);

        switch (state)
        {
        case STATE_TITLE:
            drawTitleScreen();
            break;

        case STATE_GALLERY:
            drawGalleryScreen();
            break;

        case STATE_PLAY:
            // 敵の描画
            for (const auto& enemy : enemies)
                enemy.draw();

            for (const auto& shot : shots)
                shot.draw();

            // クリティカルラインの描画
            DrawLine(0, CRITICAL_LINE_Y, WIDTH, CRITICAL_LINE_Y, GetColor(255, 0, 0), 2);
            ball.draw();
            racket.draw();

            // 共通情報（スコア、HP）の描画
            drawUI();
            break;

        case STATE_STAGE_CLEAR:
            ball.draw();
            racket.draw();

            if (stageClearTimer % 60 < 30)
            {
                const char* text = "STAGE CLEAR";
                SetFontSize(50);
                int w = GetDrawStringWidth(text, strlen(text));
                DrawString(WIDTH / 2 - w / 2, HEIGHT / 3, text, COLOR_YELLOW);
            }

            // 共通情報（スコア、HP）の描画
            drawUI();
            break;

        case STATE_GAME_CLEAR:
        {
            const char* text = "GAME CLEAR!";
            SetFontSize(50);
            int w = GetDrawStringWidth(text, strlen(text));
            DrawString(WIDTH / 2 - w / 2, HEIGHT / 3, text, COLOR_GREEN);

            if (timer % 60 < 30)
            {
                const char* start_text = "Press SPACE to Title";
                SetFontSize(30);
                int start_width = GetDrawStringWidth(start_text, strlen(start_text));
                DrawString(WIDTH / 2 - start_width / 2, HEIGHT * 2 / 3, start_text, COLOR_CYAN);
            }
        }
            break;

        case STATE_GAME_OVER:
            // 敵の描画
            for (const auto& enemy : enemies)
                //enemy.draw(imgEnemyFace);
            // 弾の描画
            for (const auto& shot : shots)
                shot.draw();

            ball.draw();
            racket.draw();
            drawGameOverScreen();

            // 共通情報（スコア、HP）の描画
            drawUI();
            break;
        }
    }

private:
    void drawGalleryScreen() const
    {
        // 背景
        DrawBox(0, 0, WIDTH, HEIGHT, GetColor(20, 20, 40), TRUE);

        int imgHandle = -1;

        // どの画像を表示するか
        if (galleryPage < (int)unlockedDogs.size() && unlockedDogs[galleryPage])
            imgHandle = imgDogGallery[galleryPage];
        else
            imgHandle = imgLocked;

        // 画像を画面にフィット
        int imgW, imgH;
        GetGraphSize(imgHandle, &imgW, &imgH);  // 元の画像サイズを取得

        if (imgW > 0 && imgH > 0)
        {
            // 横幅の比率と縦幅の比率を計算→小さい方を採用
            float ratioX = (float)WIDTH / imgW;
            float ratioY = (float)HEIGHT / imgH;
            float ratio = (ratioX < ratioY) ? ratioX : ratioY;

            // 比率をかけた後の、新しい描画サイズ
            int newW = (int)(imgW * ratio);
            int newH = (int)(imgH * ratio);

            // 中央に配置するための描画開始位置
            int drawX = (WIDTH - newW) / 2;
            int drawY = (HEIGHT - newH) / 2;

            // 拡大縮小して描画
            DrawExtendGraph(drawX, drawY, drawX + newW, drawY + newH, imgHandle, TRUE);
        }
        
        // UI
        SetFontSize(30);
        DrawFormatString(10, 10, COLOR_WHITE, "DOG GALLERY ( %d / %d )", galleryPage + 1, MAX_STAGES);
        DrawString(10, 50, "Use LEFT/RIGHT arrows", COLOR_WHITE);
        DrawString(10, 90, "Press T to Title", COLOR_WHITE);
    }

    // キーが「押された瞬間」だけtrueを返す
    bool isKeyTriggered(int keyCode)
    {
        return (keys[keyCode] == 1) && (oldKeys[keyCode] == 0);
    }

    // タイトル画面の描画
    void drawTitleScreen() const
    {
        const char* title_text = "BARK OUT";
        SetFontSize(50);
        int title_width = GetDrawStringWidth(title_text, strlen(title_text));
        DrawString(WIDTH / 2 - title_width / 2, HEIGHT / 3, title_text, COLOR_GREEN);

        if (timer % 60 < 30)
        {
            const char* start_text = "Press SPACE to start.";
            SetFontSize(30);
            int start_width = GetDrawStringWidth(start_text, strlen(start_text));
            DrawString(WIDTH / 2 - start_width / 2, HEIGHT * 2 / 3, start_text, COLOR_CYAN);

            const char* gallery_text = "Press G for Dog Gallery.";
            SetFontSize(30);
            int gallery_width = GetDrawStringWidth(gallery_text, strlen(gallery_text));
            DrawString(WIDTH / 2 - gallery_width / 2, HEIGHT * 2 / 3 + 50, gallery_text, COLOR_YELLOW);
        }
    }

    // ゲームオーバー画面の描画
    void drawGameOverScreen() const
    {
        const char* over_text = "GAME OVER";
        SetFontSize(60);
        int over_width = GetDrawStringWidth(over_text, strlen(over_text));
        DrawString(WIDTH / 2 - over_width / 2, HEIGHT / 3, over_text, COLOR_RED);

        if (timer % 60 < 30)
        {
            const char* start_text = "Press SPACE to try again.";
            SetFontSize(30);
            int start_width = GetDrawStringWidth(start_text, strlen(start_text));
            DrawString(WIDTH / 2 - start_width / 2, HEIGHT * 2 / 3, start_text, COLOR_CYAN);

            const char* title_key_text = "Press T to Title";
            SetFontSize(30);
            int title_key_width = GetDrawStringWidth(title_key_text, strlen(title_key_text));
            DrawString(WIDTH / 2 - title_key_width / 2, HEIGHT * 2 / 3 + 50, title_key_text, COLOR_YELLOW);
        }
    }

    // UIの描画（スコア、HPなど）
    void drawUI() const
    {
        SetFontSize(30);
        DrawFormatString(10, 10, COLOR_WHITE, "SCORE %d", score);
        DrawFormatString(WIDTH - 200, 10, COLOR_YELLOW, "HI-SC %d", highScore);
        DrawFormatString(10, 50, COLOR_CYAN, "HP %d", hp > 0 ? hp : 0);
        DrawFormatString(WIDTH / 2 - 70, 10, COLOR_GREEN, "STAGE %d", currentStage);

        // 残りウェーブ数はプレイ中のみ表示
        if (state == STATE_PLAY)
            DrawFormatString(WIDTH / 2 - 70, 50, COLOR_PINK, "HORDE %d", wavesLeftInStage);
    }
};
#pragma endregion

#pragma region WinMain (Main Entry Point)
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    SetWindowText("バークアウト"); // ウィンドウのタイトル
    SetGraphMode(WIDTH, HEIGHT, 32); // ウィンドウの大きさとカラービット数の指定
    ChangeWindowMode(TRUE); // ウィンドウモードで起動
    if (DxLib_Init() == -1) return -1; // ライブラリ初期化 エラーが起きたら終了
    
    SetBackgroundColor(0, 0, 0); // 背景色の指定
    SetDrawScreen(DX_SCREEN_BACK); // 描画面を裏画面にする

    srand((unsigned int)time(NULL));

    // ゲームシーンのインスタンスを作成
    GameScene game;

    while (ProcessMessage() == 0 && CheckHitKey(KEY_INPUT_ESCAPE) == 0) // メインループ
    {
        ClearDrawScreen(); // 画面をクリアする

        game.update();  // ゲーム状態の更新
        game.draw();    // 描画

        ScreenFlip();   // 裏画面の内容を表画面に反映
        WaitTimer(16);  // 16ms待つ
    }

    DxLib_End(); // ＤＸライブラリ使用の終了処理
    return 0; // ソフトの終了
}
#pragma endregion