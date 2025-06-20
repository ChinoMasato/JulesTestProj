# include <Siv3D.hpp> // Siv3D v0.6.16

const double GRAVITY = 1000.0; // Pixels per second per second
const double JUMP_VELOCITY = -500.0; // Negative for upward velocity
const double GROUND_Y = 500.0;
const double PLAYER_MOVE_SPEED = 200.0; // Horizontal speed

void Main()
{
	// 背景の色を設定する | Set the background color
	s3d::Scene::SetBackground(s3d::ColorF{ 0.6, 0.8, 0.7 });

	s3d::Vec2 playerPosition{ 400, GROUND_Y - 200 }; // Start above ground
	s3d::Vec2 playerVelocity{ 0.0, 0.0 };

	while (s3d::System::Update())
	{
		const double deltaTime = s3d::Scene::DeltaTime();

		// Horizontal Movement
		playerVelocity.x = 0.0;
		if (s3d::KeyA.pressed())
		{
			playerVelocity.x -= PLAYER_MOVE_SPEED;
		}
		if (s3d::KeyD.pressed())
		{
			playerVelocity.x += PLAYER_MOVE_SPEED;
		}
		playerPosition.x += playerVelocity.x * deltaTime;

		// Apply Gravity
		playerVelocity.y += GRAVITY * deltaTime;

		// Jumping (replaces old KeyW, removes KeyS)
		if (s3d::KeyW.pressed() && (playerPosition.y >= GROUND_Y - 30.0)) // 30 is radius
		{
			playerVelocity.y = JUMP_VELOCITY;
		}

		// Update Vertical Position
		playerPosition.y += playerVelocity.y * deltaTime;

		// Ground Collision
		if (playerPosition.y >= GROUND_Y - 30.0) // 30 is radius
		{
			playerPosition.y = GROUND_Y - 30.0;
			playerVelocity.y = 0.0; // Stop vertical movement
		}

		// Draw Ground
		s3d::Line(0, GROUND_Y, s3d::Scene::Width(), GROUND_Y).draw(2, s3d::Palette::Gray);

		// Draw Player
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
