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
	s3d::Vec2 playerPosition{ 100, GROUND_Y - 30.0 }; // Start on main ground line
	s3d::Vec2 playerVelocity{ 0.0, 0.0 };
	s3d::Camera2D camera{ s3d::Vec2{ s3d::Scene::CenterF() }, 1.0 }; // Centered camera initial

	// Dash mechanic state
	double dashTimer = 0.0; // How long current dash lasts
	double dashCooldownTimer = 0.0; // Time until next dash is available
	bool isDashing = false;

	// Jump mechanic state
	bool isJumpingForKeyHold = false; // True if jump key is held, allowing sustain
	double currentJumpSustainTime = 0.0; // How long jump has been sustained

	// Level Definition
	s3d::Array<s3d::RectF> levelObjects;
	// Platform 1
	levelObjects.push_back(s3d::RectF{ 200, GROUND_Y - 100, 200, 20 }); // x, y, width, height
	// Platform 2
	levelObjects.push_back(s3d::RectF{ 500, GROUND_Y - 180, 150, 20 });
	// A small wall on top of platform 1
	levelObjects.push_back(s3d::RectF{ 300, GROUND_Y - 100 - 50, 20, 50 }); // Sits on platform 1
	// Floating Platform
	levelObjects.push_back(s3d::RectF{ 800, GROUND_Y - 120, 100, 20 });

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

		// Determine if player is on ground (for jump initiation)
		// This check is based on player's current position and if their vertical velocity is near zero (implies landed)
		bool isOnGround = false;
		if (s3d::Math::Approximately(playerVelocity.y, 0.0, 0.1) ) // If y-velocity is practically zero
		{
			// Check against main ground with a small tolerance
			if (playerPosition.y >= GROUND_Y - 30.0 - 1.0) {
				isOnGround = true;
			}
			// Check against platforms if not on main ground
			if(!isOnGround) {
				s3d::Circle feetCircle{ playerPosition.movedBy(0, 1), 28 }; // Check slightly below current pos
				for (const auto& platform : levelObjects)
				{
					if (feetCircle.intersects(platform) && playerPosition.y < platform.top() + 1) // Player slightly above or on platform top
					{
						isOnGround = true;
						break;
					}
				}
			}
		}


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

		// Update positions based on velocity
		playerPosition.x += playerVelocity.x * deltaTime;
		playerPosition.y += playerVelocity.y * deltaTime;


		// Platform Collision Detection and Response
		// Strategy: Check current player circle against each platform.
		// To determine entry and side of collision, compare with player's position *before* this frame's movement.
		// Resolve by pushing player out and zeroing velocity on the collision axis.
		s3d::Circle playerCollisionCircle{ playerPosition, 30 };
		s3d::Vec2 previousPlayerPositionForCollision = playerPosition - playerVelocity * deltaTime; // Position before this frame's physics update

		for (const auto& platform : levelObjects)
		{
			if (playerCollisionCircle.intersects(platform))
			{
				s3d::Circle previousFrameCircle{ previousPlayerPositionForCollision, 30 };

				// Check vertical collision (landing on top or hitting bottom)
				if (playerVelocity.y > 0 && !previousFrameCircle.intersects(platform.topLine()) && playerCollisionCircle.intersects(platform.topLine(1.0))) // Moving down & was above
				{
					if (playerCollisionCircle.right().x > platform.left().x && playerCollisionCircle.left().x < platform.right().x) // Horizontal overlap
					{
						playerPosition.y = platform.top().y - 30;
						playerVelocity.y = 0;
					}
				}
				else if (playerVelocity.y < 0 && !previousFrameCircle.intersects(platform.bottomLine()) && playerCollisionCircle.intersects(platform.bottomLine(1.0))) // Moving up & was below
				{
					if (playerCollisionCircle.right().x > platform.left().x && playerCollisionCircle.left().x < platform.right().x) // Horizontal overlap
					{
						playerPosition.y = platform.bottom().y + 30;
						playerVelocity.y = 0;
					}
				}
				// Update collision circle for horizontal check after potential vertical correction
				playerCollisionCircle.setPos(playerPosition);

				// Check horizontal collision (hitting sides)
				// Important: only resolve horizontal if not primarily a vertical collision solved above
				// This simple check might still allow some corner clipping / incorrect resolution priority
				if (playerVelocity.x > 0 && !previousFrameCircle.intersects(platform.leftLine()) && playerCollisionCircle.intersects(platform.leftLine(1.0))) // Moving right & was to the left
				{
					if (playerCollisionCircle.bottom().y > platform.top().y && playerCollisionCircle.top().y < platform.bottom().y) // Vertical overlap
					{
						playerPosition.x = platform.left().x - 30;
						playerVelocity.x = 0;
					}
				}
				else if (playerVelocity.x < 0 && !previousFrameCircle.intersects(platform.rightLine()) && playerCollisionCircle.intersects(platform.rightLine(1.0))) // Moving left & was to the right
				{
					if (playerCollisionCircle.bottom().y > platform.top().y && playerCollisionCircle.top().y < platform.bottom().y) // Vertical overlap
					{
						playerPosition.x = platform.right().x + 30;
						playerVelocity.x = 0;
					}
				}
				playerCollisionCircle.setPos(playerPosition); // Update for next platform check
			}
		}

		// Main Ground Collision (Fallback)
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

			// Draw Level Objects
			for (const auto& obj : levelObjects)
			{
				obj.draw(s3d::Palette::DarkGreen); // Example color
			}

			// Draw Player (with animation placeholder)
			s3d::ColorF playerColor = s3d::Palette::Orange;
			// Re-evaluate finalIsOnGround based on current, possibly corrected, position and velocity
			bool finalIsOnGround = (s3d::Math::Approximately(playerVelocity.y, 0.0, 0.1) &&
									(playerPosition.y >= GROUND_Y - 30.0 - 1.0));
			if (!finalIsOnGround && s3d::Math::Approximately(playerVelocity.y, 0.0, 0.1)) {
				s3d::Circle feetCircle{ playerPosition.movedBy(0, 1), 28 };
				for (const auto& platform : levelObjects) {
					if (feetCircle.intersects(platform) && playerPosition.y < platform.top() +1) {
						finalIsOnGround = true;
						break;
					}
				}
			}

			if (isDashing)
			{
				playerColor = s3d::Palette::Yellow;
			}
			else if (playerVelocity.y < 0 && !finalIsOnGround) // Moving upwards and not on ground
			{
				playerColor = s3d::Palette::LightSkyBlue;
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
