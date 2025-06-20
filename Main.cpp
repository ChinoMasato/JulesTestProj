# include <Siv3D.hpp> // Siv3D v0.6.16

const double GRAVITY = 1000.0; // Pixels per second per second
const double JUMP_VELOCITY = -500.0; // Negative for upward velocity
const double GROUND_Y = 500.0;
const double PLAYER_MOVE_SPEED = 200.0; // Horizontal speed
const double DASH_SPEED_MULTIPLIER = 2.5;
const double DASH_DURATION = 0.2; // seconds
const double DASH_COOLDOWN = 0.5; // seconds
const double JUMP_SUSTAIN_FORCE_REDUCTION_FACTOR = 0.5; // Reduce gravity by this factor while sustaining
const double MAX_JUMP_SUSTAIN_DURATION = 0.25; // Max time player can sustain jump by holding key

void Main()
{
	// 背景の色を設定する | Set the background color
	s3d::Scene::SetBackground(s3d::ColorF{ 0.6, 0.8, 0.7 });

	// Player and Camera state
	s3d::Vec2 playerPosition{ 400, GROUND_Y - 200 }; // Start above ground
	s3d::Vec2 playerVelocity{ 0.0, 0.0 };
	s3d::Camera2D camera{ s3d::Vec2{ s3d::Scene::CenterF() }, 1.0 }; // Centered camera initial

	// Dash mechanic state
	double dashTimer = 0.0; // How long current dash lasts
	double dashCooldownTimer = 0.0; // Time until next dash is available
	bool isDashing = false;

	// Jump mechanic state
	bool isJumpingForKeyHold = false; // True if jump key is held, allowing sustain
	double currentJumpSustainTime = 0.0; // How long jump has been sustained

	while (s3d::System::Update())
	{
		const double deltaTime = s3d::Scene::DeltaTime();

		// Update Dash Timers
		if (dashTimer > 0.0)
		{
			dashTimer -= deltaTime;
			if (dashTimer <= 0.0)
			{
				isDashing = false;
			}
		}
		if (dashCooldownTimer > 0.0)
		{
			dashCooldownTimer -= deltaTime;
		}

		// Dash Input Handling
		bool tryingToDash = s3d::KeyShift.pressed();

		if (tryingToDash && dashCooldownTimer <= 0.0 && !isDashing)
		{
			if (s3d::KeyA.pressed() || s3d::KeyD.pressed()) // Only dash if moving
			{
				isDashing = true;
				dashTimer = DASH_DURATION;
				// Cooldown starts after the current dash finishes plus the explicit cooldown period
				dashCooldownTimer = DASH_COOLDOWN + DASH_DURATION;
			}
		}

		// Horizontal Movement
		playerVelocity.x = 0.0;
		double actualMoveSpeed = PLAYER_MOVE_SPEED;

		if (isDashing)
		{
			actualMoveSpeed *= DASH_SPEED_MULTIPLIER;
		}

		if (s3d::KeyA.pressed())
		{
			playerVelocity.x = -actualMoveSpeed;
		}
		else if (s3d::KeyD.pressed())
		{
			playerVelocity.x = actualMoveSpeed;
		}
		playerPosition.x += playerVelocity.x * deltaTime;

		// Vertical movement logic (Jump and Gravity)
		bool isOnGround = (playerPosition.y >= GROUND_Y - 30.0); // Check based on current position for jump initiation

		// Jump initiation
		if (s3d::KeyW.down() && isOnGround)
		{
			playerVelocity.y = JUMP_VELOCITY;
			isJumpingForKeyHold = true; // Enable jump sustain
			currentJumpSustainTime = 0.0; // Reset sustain timer
		}

		// Jump sustain logic
		if (!s3d::KeyW.pressed() || playerVelocity.y >= 0) // Stop sustaining if key released or player is falling/on apex
		{
			isJumpingForKeyHold = false;
		}

		if (isJumpingForKeyHold && currentJumpSustainTime < MAX_JUMP_SUSTAIN_DURATION)
		{
			currentJumpSustainTime += deltaTime; // Increment sustain timer
		}
		else
		{
			isJumpingForKeyHold = false; // Stop sustaining if max duration reached
		}

		// Apply gravity (potentially modified by jump sustain)
		double effectiveGravity = GRAVITY;
		if (isJumpingForKeyHold) // If sustaining jump (moving up, key held, within duration)
		{
			// Reduce gravity's effect to allow variable jump height
			effectiveGravity *= (1.0 - JUMP_SUSTAIN_FORCE_REDUCTION_FACTOR);
		}
		playerVelocity.y += effectiveGravity * deltaTime;

		// Update Vertical Position
		playerPosition.y += playerVelocity.y * deltaTime;

		// Ground Collision
		if (playerPosition.y >= GROUND_Y - 30.0)
		{
			playerPosition.y = GROUND_Y - 30.0;
			playerVelocity.y = 0.0;
		}

		// Update camera
		// Camera follows player's X, Y is positioned to keep ground in lower third of screen
		camera.setCenter(s3d::Vec2{ playerPosition.x, GROUND_Y - (s3d::Scene::Height() / 3.0) });
		camera.update();

		{ // Start Transformer2D scope
			const auto t = camera.createTransformer();

			// Draw Ground (extended)
			s3d::Line(-10000, GROUND_Y, 10000, GROUND_Y).draw(2, s3d::Palette::Gray);

			// Draw Player (with animation placeholder)
			s3d::ColorF playerColor = s3d::Palette::Orange;
			bool finalIsOnGround = (playerPosition.y >= GROUND_Y - 30.0); // Re-evaluate for current frame state
			if (isDashing)
			{
				playerColor = s3d::Palette::Yellow;
			}
			else if (playerVelocity.y < 0 && !finalIsOnGround) // Moving upwards and not on ground
			{
				playerColor = s3d::Palette::Lightblue;
			}
			s3d::Circle(playerPosition, 30).draw(playerColor);
		} // Transformer2D scope ends
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
