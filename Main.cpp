# include <Siv3D.hpp> // Siv3D v0.6.16

void Main()
{
	// 背景の色を設定する | Set the background color
	s3d::Scene::SetBackground(s3d::ColorF{ 0.6, 0.8, 0.7 });

	// 画像ファイルからテクスチャを作成する | Create a texture from an image file
	const s3d::Texture texture{ U"example/windmill.png" };

	// 絵文字からテクスチャを作成する | Create a texture from an emoji
	const s3d::Texture emoji{ U"🦖"_emoji };

	// 太文字のフォントを作成する | Create a bold font with MSDF method
	const s3d::Font font{ s3d::FontMethod::MSDF, 48, s3d::Typeface::Bold };

	// テキストに含まれる絵文字のためのフォントを作成し、font に追加する | Create a font for emojis in text and add it to font as a fallback
	const s3d::Font emojiFont{ 48, s3d::Typeface::ColorEmoji };
	font.addFallback(emojiFont);

	// ボタンを押した回数 | Number of button presses
	int32 count = 0;

	// チェックボックスの状態 | Checkbox state
	bool checked = false;

	// プレイヤーの移動スピード | Player's movement speed
	double speed = 200.0;

	// プレイヤーの X 座標 | Player's X position
	double playerPosX = 400;

	// プレイヤーが右を向いているか | Whether player is facing right
	bool isPlayerFacingRight = true;

	s3d::Vec2 playerPosition{ 400, 300 };

	while (s3d::System::Update())
	{
		if (s3d::KeyW.pressed())
		{
			playerPosition.y -= 5.0;
		}
		if (s3d::KeyS.pressed())
		{
			playerPosition.y += 5.0;
		}
		if (s3d::KeyA.pressed())
		{
			playerPosition.x -= 5.0;
		}
		if (s3d::KeyD.pressed())
		{
			playerPosition.x += 5.0;
		}

		// テクスチャを描く | Draw the texture
		texture.draw(20, 20);

		// テキストを描く | Draw text
		font(U"Hello, Siv3D!🎮").draw(64, s3d::Vec2{ 20, 340 }, s3d::ColorF{ 0.2, 0.4, 0.8 });

		// 指定した範囲内にテキストを描く | Draw text within a specified area
		font(U"Siv3D (シブスリーディー) は、ゲームやアプリを楽しく簡単な C++ コードで開発できるフレームワークです。")
			.draw(18, s3d::Rect{ 20, 430, 480, 200 }, s3d::Palette::Black);

		// 長方形を描く | Draw a rectangle
		s3d::Rect{ 540, 20, 80, 80 }.draw();

		// 角丸長方形を描く | Draw a rounded rectangle
		s3d::RoundRect{ 680, 20, 80, 200, 20 }.draw(s3d::ColorF{ 0.0, 0.4, 0.6 });

		// 円を描く | Draw a circle
		s3d::Circle{ 580, 180, 40 }.draw(s3d::Palette::Seagreen);

		// 矢印を描く | Draw an arrow
		s3d::Line{ 540, 330, 760, 260 }.drawArrow(8, s3d::SizeF{ 20, 20 }, s3d::ColorF{ 0.4 });

		// 半透明の円を描く | Draw a semi-transparent circle
		s3d::Circle{ s3d::Cursor::Pos(), 40 }.draw(s3d::ColorF{ 1.0, 0.0, 0.0, 0.5 });

		// ボタン | Button
		if (s3d::SimpleGUI::Button(U"count: {}"_fmt(count), s3d::Vec2{ 520, 370 }, 120, (checked == false)))
		{
			// カウントを増やす | Increase the count
			++count;
		}

		// チェックボックス | Checkbox
		s3d::SimpleGUI::CheckBox(checked, U"Lock \U000F033E", s3d::Vec2{ 660, 370 }, 120);

		// スライダー | Slider
		s3d::SimpleGUI::Slider(U"speed: {:.1f}"_fmt(speed), speed, 100, 400, s3d::Vec2{ 520, 420 }, 140, 120);

		// 左キーが押されていたら | If left key is pressed
		if (s3d::KeyLeft.pressed())
		{
			// プレイヤーが左に移動する | Player moves left
			playerPosX = Max((playerPosX - speed * s3d::Scene::DeltaTime()), 60.0);
			isPlayerFacingRight = false;
		}

		// 右キーが押されていたら | If right key is pressed
		if (s3d::KeyRight.pressed())
		{
			// プレイヤーが右に移動する | Player moves right
			playerPosX = Min((playerPosX + speed * s3d::Scene::DeltaTime()), 740.0);
			isPlayerFacingRight = true;
		}

		// プレイヤーを描く | Draw the player
		emoji.scaled(0.75).mirrored(isPlayerFacingRight).drawAt(playerPosX, 540);

		s3d::Circle(playerPosition, 30).draw(s3d::Palette::Orange);
	}
}

//
// - Debug ビルド: プログラムの最適化を減らす代わりに、エラーやクラッシュ時に詳細な情報を得られます。
//
// - Release ビルド: 最大限の最適化でビルドします。
//
// - [デバッグ] メニュー → [デバッグの開始] でプログラムを実行すると、[出力] ウィンドウに詳細なログが表示され、エラーの原因を探せます。
//
// - Visual Studio を更新した直後は、プログラムのリビルド（[ビルド]メニュー → [ソリューションのリビルド]）が必要な場合があります。
//
