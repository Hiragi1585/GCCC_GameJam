//-----------------------------------------------
//
//  忙しい人のためのゲームひな形 for Siv3D v1.0.1
//
//  This file is part of the Siv3D Engine.
//
//  Copyright (C) 2008-2016 Ryo Suzuki
//
//  Licensed under the MIT License.
//
//-----------------------------------------------

# include <Siv3D.hpp>
# include <HamFramework.hpp>
# include <vector>


///////////////////////////////////////////////////////////////////////
//
//  ゲームの基本情報
//
namespace GameInfo
{
	// ゲームのタイトル
	const String Title = L"ギリギリで救え！！！";

	// ゲームのバージョン（空の文字列も OK）
	const String Version = L"Ver 1.14514";

	// ゲームの Web サイト（無い場合は空の文字列にする）
	const String WebURL = L"http://gcccweb.blog59.fc2.com/";

	// 結果ツイートの文章（TweetHead + score + TweetTail)
	const String TweetHead = L"ギリギリで救え！ をプレイしたよ。結果: ";

	const String TweetTail = L" Points";

	// ゲームの背景色
	const ColorF BackgroundColor = ColorF(0.4, 0.7, 0.5);

	// シーン切り替え時のフェードイン・アウトの色
	const ColorF FadeInColor = ColorF(1.0, 1.0, 1.0);

	// タイトルのフォントサイズ
	const int32 TitleFontSize = 72;

	// メニューのフォントサイズ
	const int32 MenuFontSize = 24;

	// メニュー項目「ゲームスタート」
	const String MenuGameStart = L"単位を拾いに行く";

	// メニュー項目「ゲーム説明」
//	const String MenuTutorial = L"ゲーム説明";

	// メニュー項目「スコア」
	const String MenuScore = L"スコア";

	// メニュー項目「クレジット」
	const String MenuCredit = L"スタッフクレジット";

	// メニュー項目「Web サイト」
	const String MenuWebURL = L"";

	// メニュー項目「終了」
	const String MenuExit = L"終了";

	// セーブデータの保存場所
	const FilePath SaveFilePath = L"Save/Score.dat";

	// スタッフロールのクレジット（項目は増減できる）
	const Array<std::pair<String, Array<String>>> Credits
	{
		{ L"ゲームデザイン",{ L"Hiragi-GKUTH" } },
		{ L"プログラム",{ L"Hiragi-GKUTH" } },
		{ L"ゲームアート",{ L"Hiragi-GKUTH" } },
		{ L"音楽",{ L"Hiragi-GKUTH" } },
		{ L"Special Thanks",{ L"You!" } },
	};
}

///////////////////////////////////////////////////////////////////////
//
//  タイトル画面の背景エフェクト
//
struct TitleBackGroundEffect : IEffect
{
	Texture tx_eff;
	Vec2 pos;
	Vec2 vel;
	double ang;
	const double fv = 5.0;

	TitleBackGroundEffect()
	{
		pos = Vec2(Random()*Window::Width(), Random()*Window::Height());
		vel = Vec2((Random()-0.5)*fv, (Random()*0.5)*fv);
		ang = Random()*3.14159 * 2;
		tx_eff = Texture(L"img/Tani.png");
	}

	bool update(double timeSec)
	{
		//Update
		vel.y -= 0.1;
		pos.y -= vel.y;
		pos.x -= vel.x;

		//Draw
		tx_eff.scale(0.7).rotate(ang).drawAt(pos,Color(255,255,255,(255-timeSec*120)));
		return timeSec < 2.0;
	}
};

///////////////////////////////////////////////////////////////////////
//
//  タイトル画面のメニュー選択時のエフェクト
//
struct MenuEffect : IEffect
{
	Rect m_rect;

	MenuEffect(const Rect& rect)
		: m_rect(rect) {}

	bool update(double timeSec)
	{
		const double e = EaseOut<Easing::Quad>(timeSec);
		RectF(m_rect).stretched(e * 20).shearedX(20).draw(ColorF(1.0-e,1.0-e,1.0,1.0-e));
		return timeSec < 1.0;
	}
};

///////////////////////////////////////////////////////////////////////
//
//  単位を修得、落とす時のエフェクト
//

struct TaniEffect : IEffect{
	int kind;
	String text;
	Vec2 pos;
	double scale;
	Texture img[5];

	TaniEffect(int argKind,Vec2 argPos)
	{
		kind = argKind;
		pos = argPos;
		pos.y -= 60;
		img[0] = Texture(L"img/effect/syu.png");
		img[1] = Texture(L"img/effect/yu.png");
		img[2] = Texture(L"img/effect/ryo.png");
		img[3] = Texture(L"img/effect/ka.png");
		img[4] = Texture(L"img/effect/huka.png");
	}

	bool init()
	{

	}

	bool update(double timeSec)
	{
		pos.y++;
		img[kind].rotate(Radians(10)).drawAt(pos,Color(255,255,255,255-timeSec*250));

		return timeSec < 1.0;
	}

};

///////////////////////////////////////////////////////////////////////
//
//  全てのシーンからアクセスできるデータ
//
struct GameData
{
	int32 lastScore = 0;
	int32 mode = 0;
};

using MyApp = SceneManager<String, GameData>;

///////////////////////////////////////////////////////////////////////
//
//  セーブ用スコアデータ
//
struct ScoreData
{
	int32 score;
	int32 mode;
	Date date;

	template <class Archive>
	void serialize(Archive& archive)
	{
		archive(mode, score, date);
	}
};

///////////////////////////////////////////////////////////////////////
//
//  デフォルトのハイスコア
//
const std::array<ScoreData, 5> defaultHighScores
{
	ScoreData{ 50,1, Date(2017,1,1) },
	ScoreData{ 40,1, Date(2017,1,1) },
	ScoreData{ 30,1, Date(2017,1,1) },
	ScoreData{ 20,1, Date(2017,1,1) },
	ScoreData{ 10,1, Date(2017,1,1) },
};
///////////////////////////////////////////////////////////////////////
//
//  タイトル画面
//
class Title : public MyApp::Scene
{
private:

	Effect m_effect;
	Stopwatch m_effectBackgroundStopwatch{ true };
	Stopwatch m_effectMenuItemStopwatch{ true };
	Array<Rect> m_menuBoxes;
	Array<Rect> m_modeBoxes;
	int cnt;
	Array<String> m_menuTexts =
	{
		GameInfo::MenuGameStart,
		GameInfo::MenuScore,
		GameInfo::MenuCredit,
		GameInfo::MenuExit,
	};
	Array<String> m_modeTexts =
	{
		L"Normal",
		L"Hard",
		L"Lunatic",
	};
	bool flag;

public:

	~Title()
	{
		Cursor::SetStyle(CursorStyle::Default);
	}

	void init() override
	{
		cnt = 0;
		if (GameInfo::WebURL.isEmpty)
		{
			m_menuTexts.erase(m_menuTexts.begin() + 3);
		}

		m_menuBoxes.resize(m_menuTexts.size());
		m_modeBoxes.resize(m_modeTexts.size());

		int32 boxWidth = 0;
		flag = false;

		for (const auto& text : m_menuTexts)
		{
			boxWidth = Max(boxWidth, FontAsset(L"Menu")(text).region().w);
		}

		for (auto i : step(m_menuBoxes.size()))
		{
			m_menuBoxes[i].set(120, 280 + i * 80, boxWidth + 80, 60);
		}

		for (auto i : step(m_modeBoxes.size()))
		{
			m_modeBoxes[i].set(120 + boxWidth + 120, 280 + (i - 1) * 60, boxWidth, 50);
		}
	}

	void update() override
	{
		cnt++;
		bool handCursor = false;
		for (auto i : step(m_menuBoxes.size()))
		{
			const Quad item = m_menuBoxes[i].shearedX(20);

			handCursor |= item.mouseOver;

			if (item.mouseOver)
			{
				if (i == 0) {
					flag = true; cnt = 0;
				}
				if (m_effectMenuItemStopwatch.elapsed() > 200ms)
				{
					m_effect.add<MenuEffect>(m_menuBoxes[i]);
					m_effectMenuItemStopwatch.restart();
				}
			}

			if (!(item.mouseOver && i == 0) && cnt > 60)
			{
				//flag = false;
			}

			if (item.leftClicked)
			{
				if (i == 0)
				{

				}
				else if (i == 1)
				{
					changeScene(L"Score");
				}
				else if (i == 2)
				{
					changeScene(L"Credit");
				}
				else if (!GameInfo::WebURL.isEmpty && i == 3)
				{
					Internet::LaunchWebBrowser(GameInfo::WebURL);
				}
				else
				{
					System::Exit();
				}

				break;
			}
		}

		if (flag)
		{
			for (auto i : step(m_modeBoxes.size()))
			{
				const Quad item = m_modeBoxes[i].shearedX(20);

				handCursor |= item.mouseOver;

				if (item.mouseOver && System::FrameCount() % 10 == 0)
				{
					m_effect.add<MenuEffect>(m_modeBoxes[i]);
				}

				if (item.leftClicked)
				{
					if (i == 0)
					{
						m_data->mode = 0;
					}
					else if (i == 1)
					{
						m_data->mode = 1;
					}
					else if (i == 2)
					{
						m_data->mode = 2;
					}
					changeScene(L"Game");
					break;
				}
			}
		}

		if (m_effectBackgroundStopwatch.elapsed() > 50ms)
		{
			m_effect.add<TitleBackGroundEffect>();

			m_effectBackgroundStopwatch.restart();
		}

		Cursor::SetStyle(handCursor ? CursorStyle::Hand : CursorStyle::Default);
	}

	void draw() const override
	{
		Graphics2D::SetBlendState(BlendState::Additive);

		m_effect.update();

		Graphics2D::SetBlendState(BlendState::Default);

		const double titleHeight = FontAsset(L"Title")(GameInfo::Title).region().h;

		FontAsset(L"Title")(GameInfo::Title).drawAt(Window::Center().x, titleHeight);

		for (auto i : step(m_menuBoxes.size()))
		{
			m_menuBoxes[i].shearedX(20).draw();

			FontAsset(L"Menu")(m_menuTexts[i]).drawAt(m_menuBoxes[i].center, Color(40));
		}
		if (flag)
		{
			for (auto i : step(m_modeBoxes.size()))
			{
				m_modeBoxes[i].shearedX(20).draw();
				FontAsset(L"Menu")(m_modeTexts[i]).drawAt(m_modeBoxes[i].center, Color(40));
			}
		}

		const Size versionSize = FontAsset(L"Version")(GameInfo::Version).region().size;

		FontAsset(L"Version")(GameInfo::Version).drawAt(Window::Size().moveBy(-versionSize));
	}
};


///////////////////////////////////////////////////////////////////////
//
//  ゲーム
//
class Game : public MyApp::Scene
{
private:
		//単位が持つ情報
	typedef struct {
		Vec2 pos;
		Vec2 vel;
		int cnt;
		int scr;
		int knd;
		bool flag;
		bool hit;
	}Tani;
		//ステージのパラメータ
	typedef struct {
		int cnt;
		int appear_tani;
		int dropped_tani;
		int m_score;
		int interval;
		int start;
		int end;
		double grav;
		int mode;	//1:Normal 2:Hard 3:Lunatic
		String StageName;
	}ModeParam;

	Font px32;
	Font px16;
	std::vector<Tani> tani;
	ModeParam modeparam;
	Texture tani_img;
	Effect m_effect;

public:

	void init() override{
		m_data->lastScore = 0;
		tani_img = Texture(L"img/Tani.png");
		px32 = Font(32);
		px16 = Font(16);
		init_stage();
	}

	void init_stage(){
		modeparam.appear_tani = 0;
		modeparam.dropped_tani = 0;
		modeparam.cnt = 0;
		modeparam.m_score = 0;
		modeparam.mode = m_data->mode;
		switch (modeparam.mode)
		{
			case 0: {
				modeparam.start = 60;
				modeparam.end = 30;
				modeparam.grav = 0.03;
				modeparam.StageName = L"Normal";
			}
			case 1: {
				modeparam.start = 45;
				modeparam.end = 15;
				modeparam.grav = 0.05;
				modeparam.StageName = L"Hard";
			}
			case 2: {
				modeparam.start = 25;
				modeparam.end = 2;
				modeparam.grav = 0.07;
				modeparam.StageName = L"Lunatic";
			}
		}
		modeparam.interval = modeparam.start;
	}

	void update() override
	{
		modeparam.cnt++;

			//単位の登録
		if ((int)(Random()*modeparam.interval) == 0)
		{
			Tani push;
			push.cnt = 0;
			push.pos = Vec2(Random()*Window::Width(), (Random()*Window::Height())*2/3);
			push.vel = Vec2(0.0f, 0.0f);
			push.scr = 100;
			push.hit = false;
			push.flag = false;
			modeparam.appear_tani++;
			tani.push_back(push);
		}
			//単位の処理
		for (auto n = tani.begin(); n != tani.end();)
		{
			n->cnt++;
				//経過時間によって動作を変更する。
			if (n->cnt > 70) n->flag = true;
			if (n->cnt > 120)
			{
				n->scr+=1-n->vel.y*2;
				n->vel.y -= modeparam.grav;
				n->pos.y -= n->vel.y;
			}
				//当たり判定をごにょごにょする
			const Rect col(n->pos.x-tani_img.width/2,n->pos.y-tani_img.height/2,tani_img.width,tani_img.height);
			const bool r = col.mouseOver;
			const bool c = col.leftClicked;
				//画面外に出る=単位を落とす
			if (n->pos.y - tani_img.height > Window::Height())
			{
				modeparam.m_score -= n->scr / 3;
				modeparam.dropped_tani++;
				m_effect.add<TaniEffect>(4,n->pos);
				n->hit = true;
			}
				//単位をクリックできたら消してスコア加算
			if (c)
			{
				n->hit = true;
				modeparam.m_score += n->scr;
				
			}

			if (n->hit)
				n = tani.erase(n);
			else
				n++;

		}
			//単位の発生間隔の処理
		if (modeparam.cnt > 200)
			if (modeparam.cnt % 50 == 0)
				modeparam.interval--;
		
	}

	void draw() const override
	{
		m_effect.update();
			//単位の描画
		for (auto n = tani.begin(); n != tani.end();)
		{
			if(n->flag)
				tani_img.drawAt(n->pos+RandomVec2(2.0));
			else
				tani_img.drawAt(n->pos);

			px16(n->scr).draw(n->pos);
			n++;
		}
		FontAsset(L"GameTime")(Profiler::FPS()).draw(0, 0);
	}
};

///////////////////////////////////////////////////////////////////////
//
//  結果画面
//
class Result : public MyApp::Scene
{
private:

	std::array<ScoreData, 5> m_highScores = defaultHighScores;
	const Circle titleButton = Circle(Window::Center().x - 300, Window::Height() * 0.7, 35);
	const Circle tweetButton = Circle(Window::Center().x + 300, Window::Height() * 0.7, 35);

public:

	~Result()
	{
		Cursor::SetStyle(CursorStyle::Default);
	}

	void init() override
	{
		if (FileSystem::Exists(GameInfo::SaveFilePath))
		{
			Deserializer<BinaryReader>{GameInfo::SaveFilePath}(m_highScores);
		}
		else
		{
			Serializer<BinaryWriter>{GameInfo::SaveFilePath}(m_highScores);
		}

		if (m_highScores.back().score <= m_data->lastScore)
		{
			m_highScores.back() = { m_data->lastScore, 0,Date::Today() };

			std::sort(m_highScores.begin(), m_highScores.end(), [](const ScoreData& a, const ScoreData& b)
			{
				return a.score > b.score ? true : a.score == b.score ? a.date > b.date : false;
			});

			Serializer<BinaryWriter>{GameInfo::SaveFilePath}(m_highScores);
		}
	}

	void update() override
	{
		if (titleButton.leftClicked || Input::KeyEscape.clicked)
		{
			changeScene(L"Title");
		}

		if (tweetButton.leftClicked)
		{
			const String tweetMessage = Format(GameInfo::TweetHead, m_data->lastScore, GameInfo::TweetTail);

			Twitter::OpenTweetWindow(tweetMessage);
		}

		const bool handCursor = titleButton.mouseOver || tweetButton.mouseOver;

		Cursor::SetStyle(handCursor ? CursorStyle::Hand : CursorStyle::Default);
	}

	void draw() const override
	{

		const double resultHeight = FontAsset(L"Result")(L"x", m_data->lastScore).region().h;

		FontAsset(L"Result")(L"x", m_data->lastScore).draw(Window::Center().x + 50, Window::Height() * 0.4 - resultHeight / 2);

		FontAsset(L"ResultButton")(L"タイトルへ").drawAt(titleButton.center.movedBy(0, 90));

		FontAsset(L"ResultButton")(L"結果をツイート").drawAt(tweetButton.center.movedBy(0, 90));
	}
};

///////////////////////////////////////////////////////////////////////
//
//  ハイスコア一覧
//
class Score : public MyApp::Scene
{
private:

	std::array<ScoreData, 5> m_highScores = defaultHighScores;

public:

	void init() override
	{
		if (FileSystem::Exists(GameInfo::SaveFilePath))
		{
			Deserializer<BinaryReader>{GameInfo::SaveFilePath}(m_highScores);
		}
		else
		{
			Serializer<BinaryWriter>{GameInfo::SaveFilePath}(m_highScores);
		}
	}

	void update() override
	{
		if ((Input::MouseL | Input::KeyEscape).clicked)
		{
			changeScene(L"Title");
		}
	}

	void draw() const override
	{
		const int32 h = FontAsset(L"ScoreList").height;

		for (auto i : step(m_highScores.size()))
		{
			const Rect rect = Rect(520, 100).setCenter(Window::Center().x, 120 + i * 120);

			rect.draw(ColorF(1.0, 0.2));

			FontAsset(L"ScoreList")(m_highScores[i].score)
				.draw(rect.pos + Point(42, (rect.h - h) / 2 + 2), Palette::Gray);

			FontAsset(L"ScoreList")(m_highScores[i].score)
				.draw(rect.pos + Point(40, (rect.h - h) / 2));

			const Size dateSize = FontAsset(L"ScoreListDate")(m_highScores[i].date).region().size;

			FontAsset(L"ScoreListDate")(m_highScores[i].date)
				.draw(rect.br.x - dateSize.x - 40, rect.center.y - dateSize.y / 2);

			for (auto k : step(5 - i))
			{
				const Point left(rect.center.movedBy(-rect.w / 2 - 40 - k * 50, 0));
				const Point right(rect.center.movedBy(rect.w / 2 + 40 + k * 50, 0));

				Rect(40).setCenter(left).rotated(45_deg).draw(AlphaF(1.0 - i * 0.1));
				Rect(40).setCenter(right).rotated(45_deg).draw(AlphaF(1.0 - i * 0.1));
			}
		}
	}
};

///////////////////////////////////////////////////////////////////////
//
//  スタッフクレジット
//
class Credit : public MyApp::Scene
{
private:

	Array<std::tuple<String, double, bool>> m_credits;

	double m_height = 0.0;

	Stopwatch m_stopwatch{ true };

	double yOffset() const
	{
		return Window::Height() + 60 - m_stopwatch.ms() / 20.0;
	}
public:

	void init()
	{
		double y = 0;

		for (const auto& credit : GameInfo::Credits)
		{
			m_credits.emplace_back(credit.first, y, true);
			y += 70;

			for (const auto& name : credit.second)
			{
				m_credits.emplace_back(name, y, false);
				y += 60;
			}

			y += 60;
		}

		m_height = y;
	}

	void update() override
	{
		if ((Input::MouseL | Input::KeyEscape).clicked)
		{
			changeScene(L"Title");
		}

		if ((m_height + yOffset()) < 0)
		{
			m_stopwatch.restart();
		}
	}

	void draw() const override
	{
		const double offset = yOffset();

		for (const auto& credit : m_credits)
		{
			FontAsset(std::get<bool>(credit) ? L"CreditBig" : L"CreditSmall")(std::get<String>(credit))
				.drawAt(Window::Center().x, std::get<double>(credit) + offset);
		}
	}
};

///////////////////////////////////////////////////////////////////////
//
//  メイン関数
//
void Main()
{
	Window::Resize(1280, 720);
	Window::SetTitle(GameInfo::Title);
	Graphics::SetBackground(GameInfo::BackgroundColor);
	System::SetExitEvent(WindowEvent::CloseButton);

	//////////////////////////////////////////////////////////////
	//
	//  使用するシーン
	//
	MyApp manager;
	manager.setFadeColor(GameInfo::FadeInColor);
	manager.add<Title>(L"Title");
	manager.add<Game>(L"Game");
	manager.add<Result>(L"Result");
	manager.add<Score>(L"Score");
	manager.add<Credit>(L"Credit");
	//////////////////////////////////////////////////////////////
	//
	//  使用するフォント
	//
	FontAsset::Register(L"Title", GameInfo::TitleFontSize, Typeface::Heavy, FontStyle::Outline);
	FontAsset(L"Title").changeOutlineStyle(TextOutlineStyle(Color(60), Color(255), GameInfo::TitleFontSize * 0.05));
	FontAsset::Register(L"Menu", GameInfo::MenuFontSize, Typeface::Bold);
	FontAsset::Register(L"Version", 14, Typeface::Regular);
	FontAsset::Register(L"CountDown", 72, Typeface::Bold);
	FontAsset::Register(L"Result", 80, Typeface::Bold);
	FontAsset::Register(L"ResultButton", 32, Typeface::Regular);
	FontAsset::Register(L"GameTime", 40, Typeface::Light);
	FontAsset::Register(L"ScoreList", 50, Typeface::Heavy);
	FontAsset::Register(L"ScoreListDate", 25, Typeface::Regular, FontStyle::Italic);
	FontAsset::Register(L"CreditBig", 32, Typeface::Bold);
	FontAsset::Register(L"CreditSmall", 28, Typeface::Regular);

	//////////////////////////////////////////////////////////////
	//
	//  メインループ
	//
	while (System::Update())
	{
		if (!manager.updateAndDraw())
		{
			break;
		}
	}
}
