#include"PlayerManeger.h"
#include"../Data/MyAlgorithm.h"
#include"../SceneManager/SceneManager.h"
#include <fstream>
PlayerManager::PlayerManager()
{
	_model = nullptr;
	//start_position, control_position1, control_position2, end_position
}

PlayerManager::~PlayerManager()
{
}

bool PlayerManager::Initialize()
{
	_model = GraphicsDevice.CreateAnimationModelFromFile(_T("MODEL/Player/hero_0527.X"));

	_model->SetScale(1.0f);
	_model->SetPosition(0, 0, 0);
	_model->SetRotation(0, 0, 0);

	if (_model == nullptr)
		return false;

    SimpleShape shape;
	shape.Type = Shape_Box;
	shape.Width  = 1;
	shape.Height = 1;
	shape.Depth  = 1;

	Material _collision_material;
	_collision_material.Diffuse = Color(1.0f, 1.0f, 1.0f);
	_collision_material.Ambient = Color(0.5f, 0.5f, 0.5f);
	_collision_material.Specular = Color(1.0f, 1.0f, 1.0f);
	_collision_material.Power = 1.0f;

	Material _player_material;
	_player_material.Diffuse = Color(1.0f, 1.0f, 1.0f);
	_player_material.Ambient = Color(0.5f, 0.5f, 0.5f);
	_player_material.Specular = Color(1.0f, 1.0f, 1.0f);
	_player_material.Power = 1.0f;

	_model->SetMaterial(_player_material);

	_collision = GraphicsDevice.CreateModelFromSimpleShape(shape);
	_collision->SetMaterial(_collision_material);
	_collision->SetScale(0.5f, 3.0f, 0.5f);

	_invincibletime = 0;
	_invincibleflag = false;
	_invincible_countstart = false;

	for (int i = 0; i < AnimationState::ALLTYPE; i++)
	{
		_model->SetTrackPosition(i, 0);
		_model->SetTrackEnable(i, FALSE);
	}

	_playermove = 0.0f;
	_max_invincibletime = 0;
	_frontdistance = 0.0f;
	_sidedistance = 0.0f;

	_rotate_direction = 0.0f;
	_frontvector      = Vector3_Zero;
	_rotation         = 0.0f;
	_atan2_change     = 0.0f;

	_attack_pattern   = 0;//０…サイズが大きくなる
	                      //１…飛距離が伸びる
	                      //２…移動速度が上がる

	_knock_back            = 0.01f;
	_blinking              = 1.0f;
	_blinking_state        = true;
	_boomerang_addspeed    = 0.01f;
	_boomerang_adddistance = 1.0f;

	_player_position = Vector3_Zero;

	InputDevice.CreateGamePad(1);

	PullStatus();

	return true;
}

int PlayerManager::Update()
{
	_player_position = _model->GetPosition();
	PlayerGetPosition();

	KeyboardState key = Keyboard->GetState();
	KeyboardBuffer key_buffer = Keyboard->GetBuffer();

	GamePadState pad = GamePad(0)->GetState();
	GamePadBuffer pad_buffer = GamePad(0)->GetBuffer();

	if (_animstate != AnimationState::DAMAGE) {
		KeyboardMove(key);
		PadMove(pad);
	}
	else {
		_invincible_countstart = true;
		_model->Move(-_model->GetFrontVector() * _knock_back);
		if (_animation_count >= 1.0f) {
			_animstate = AnimationState::WAIT;
		}
	}

	if (_invincible_countstart)
		InvincibleManagement();

	if (DoStance(pad,key)) {
		_animstate = AnimationState::STANCE;
	}

	if (pad.Y2 != 0.0f || pad.X3 != 0.0f)
	{
		padstick.push_back(Vector3(pad.Y2 * 0.0002, 0, -pad.X3 * 0.0002));
	}
	if (pad_buffer.IsPressed(GamePad_Button2) && !pad.Buttons[3] && _animstate != AnimationState::SHOOT)
	{
#ifdef DEBUG
		ChangeAttackPattern();
#endif
	}

	if ((key_buffer.IsReleased(Keys_Space) || pad_buffer.IsReleased(GamePad_Button4)) && !_shootstate)
	{
		_animstate = AnimationState::SHOOT;
	}

	if (_shootstate)
	{
		if (_boomerang.Update(_model->GetPosition(),pad) == 1)
		{
			_power = 0.0f;
			_boomerang_adddistance = 1.0f;
			_boomerang_addspeed    = 0.01f;
			padstick.erase(padstick.begin(), padstick.end());
 			_shootstate = false;
		}
	}

	_collision->SetPosition(_model->GetPosition() + Vector3(0.0f, 0.0f, 0.0f));

	_model->SetPosition(Clamp(_model->GetPosition().x,-9.0f,9.0f), 0.0f, Clamp(_model->GetPosition().z, -9.0f, 9.0f));
	return 0;
}

void PlayerManager::Draw()
{
	ChangeAnimation();
	_model->DrawAlpha(_blinking);

#ifdef DEBUG
	GraphicsDevice.BeginAlphaBlend();
	if (_animstate == AnimationState::DAMAGE)
	{
	}
	else
		//_collision->DrawAlpha(0.5f);

	GraphicsDevice.EndAlphaBlend();
#endif

	if (_shootstate)
		_boomerang.Draw();
}

void PlayerManager::PullStatus()
{
	//!データ読み込み
	LoadCSV::Instance().LoadStatus("csvFile/Player/PlayerStatus.csv");
	_playermove         = LoadCSV::Instance()._status.at("#動く速度");
	_max_invincibletime = LoadCSV::Instance()._status.at("#無敵時間");
	_frontdistance      = LoadCSV::Instance()._status.at("#ブーメランを飛ばす距離(前)");
	_sidedistance       = LoadCSV::Instance()._status.at("#ブーメランを飛ばす距離(横)");
}

void PlayerManager::KeyboardMove(KeyboardState key)
{
	auto old_pos = _model->GetPosition();
	if (key.IsKeyDown(Keys_W))
		_model->Move(0.0f, 0.0f, _playermove);

	if (key.IsKeyDown(Keys_A))
		_model->Move(-_playermove, 0.0f, 0.0f);

	if (key.IsKeyDown(Keys_S))
		_model->Move(0.0f, 0.0f, -_playermove);

	if (key.IsKeyDown(Keys_D))
		_model->Move(_playermove, 0.0f, 0.0f);

	if (key.IsKeyDown(Keys_Right)) {
		_model->Rotation(0.0f, 1.0f, 0.0f);
		_collision->Rotation(0.0f, 1.0f, 0.0f);
	}

	if (key.IsKeyDown(Keys_Left)) {
		_model->Rotation(0.0f, -1.0f, 0.0f);
		_collision->Rotation(0.0f, -1.0f, 0.0f);
	}

	if (_animstate == AnimationState::SHOOT || _animstate == AnimationState::STANCE)
	{
		return;
	}
	else if (_model->GetPosition() != old_pos)
	{
		_animstate = AnimationState::RUN;
	}
	else
	{
		_animstate = AnimationState::WAIT;
	}
}

void PlayerManager::PadMove(GamePadState pad)
{
	if (_animstate == AnimationState::SHOOT)
	{
		return;
	}
	auto old_pos = _model->GetPosition();

	if (_animstate != AnimationState::STANCE) {
		if (NowMove(pad)) {
			MovePlayerRotate(pad);
			_model->SetRotation(0.0f, _rotation, 0.0f);
			_model->Move(0.0f, 0.0f, _playermove);
		}
	}
	else {
		if (NowMove(pad)) {
			MovePlayerRotate(pad);
			_model->SetRotation(0.0f, _rotation, 0.0f);
		}
	}

	if (_animstate == AnimationState::SHOOT||_animstate == AnimationState::STANCE)
	{
		return;
	}
	else if (_model->GetPosition() != old_pos)
	{
		_animstate = AnimationState::RUN;
	}
	else
	{
		_animstate = AnimationState::WAIT;
	}
}

//! @fn プレイヤーモデルのアニメーション切り替え
void PlayerManager::ChangeAnimation()
{
	auto index = _oldanimstate;

	_animation_count += GameTimer.GetElapsedSecond() * 2;

	//! 全てのアニメーショントラックを停止
	for (int i = 0; i < AnimationState::ALLTYPE; ++i)
	{
		_model->SetTrackEnable(i, FALSE);
	}

	//! アニメーショントラックと状態が違う場合アニメーショントラックを更新
	if (_animstate != index)
	{
		_oldanimstate = _animstate;
		_animation_count = 0.0f;
	}

	if (_animstate == AnimationState::SHOOT && _animation_count >= 0.1f)
	{
		Shoot();
	}

	//! アニメーショントラックのアニメーションを指定した位置から再生
	_model->SetTrackEnable(_animstate, TRUE);

	_model->SetTrackPosition(_animstate, _animation_count);
}

void PlayerManager::Shoot()
{
	FlyPoint();

	_animstate = AnimationState::WAIT;
	_shootstate = true;
}

void PlayerManager::OnCollisionEnter()
{
	if (!_invincibleflag) {
		Damage();
		_animstate = AnimationState::DAMAGE;
		SceneManager::Instance().SetCombo(0);
	}
}

void  PlayerManager::Damage()
{
	_hp -= 1;
}

void PlayerManager::MovePlayerRotate(GamePadState pad)
{
	_rotate_direction = MathHelper_Atan2(pad.Y, pad.X) + 90.0f;
	Vector3 _target_foward(MathHelper_Cos(_rotate_direction), 0.0f, MathHelper_Sin(_rotate_direction));
	Vector3 _rotate_foward(MathHelper_Cos(_rotation), 0.0f, MathHelper_Sin(_rotation));
	_rotate_foward    = Vector3_Hermite(_rotate_foward, _target_foward, GameTimer.GetElapsedSecond() * 20);
	_rotation         = MathHelper_Atan2(_rotate_foward.z, _rotate_foward.x);
}

void PlayerManager::FlyPoint()
{
	Vector3 _start_position = _model->GetPosition() + _model->GetFrontVector();

	Vector3 _control_position1 = _model->GetPosition() + _model->GetFrontVector() * _frontdistance * _boomerang_adddistance
		+ _model->GetRightVector() * _sidedistance * _boomerang_adddistance;

	Vector3 _control_position2 = _model->GetPosition() + _model->GetFrontVector() * _frontdistance * _boomerang_adddistance
		+ (-_model->GetRightVector()) * _sidedistance * _boomerang_adddistance;

	_boomerang.Initialize(_start_position, _control_position1, _control_position2, _power, _boomerang_addspeed, padstick);
}

Vector3 PlayerManager::PlayerGetPosition()
{
	return _player_position;
}

void PlayerManager::InvincibleManagement()
{
	if (_invincibletime <= _max_invincibletime) {
		_invincibletime += 1;
		_invincibleflag = true;

		if (_blinking_state == 1 && _blinking >= 1.0f)
		{
			_blinking_state = 0;
		}

		if (_blinking_state == 0 && _blinking <= 0.3f)
		{
			_blinking_state = 1;
		}


		if (_blinking_state) {

			_blinking += 0.1f;
		}
		else {

			_blinking -= 0.1f;
		}

	}
	else
	{
		_blinking = 1.0f;
		_invincibletime = 0;
		_invincible_countstart = false;
		_invincibleflag = false;
	}
}

bool PlayerManager::NowMove(GamePadState pad)
{
	return pad.X != 0.0f || pad.Y != 0.0f && _animstate != AnimationState::DAMAGE;
}

bool PlayerManager::DoStance(GamePadState pad, KeyboardState key)
{
	return (KeyShot(key) || PadShot(pad)) && NotShot() && NotStance() && !_shootstate;
}

bool PlayerManager::KeyShot(KeyboardState key)
{
	return key.IsKeyDown(Keys_Space);
}

bool PlayerManager::PadShot(GamePadState pad)
{
	return pad.Buttons[3];
}

bool PlayerManager::NotShot()
{
	return _animstate != AnimationState::SHOOT;
}

bool PlayerManager::NotStance()
{
	return _animstate != AnimationState::STANCE;
}

#ifdef _DEBUG
void PlayerManager::ChangeAttackPattern()
{
	_attack_pattern += 1;
	if (_attack_pattern == 3)
		_attack_pattern = 0;
}

int PlayerManager::AttackPattern()
{
	return _attack_pattern;
}

void PlayerManager::BoomerangSizeUp()
{
	_power += 0.01;
	if (_power >= 2.0f)
		_power = 2.0f;
}

void PlayerManager::BoomerangDistanceUp()
{
	_boomerang_adddistance += 0.01f;
	if (_boomerang_adddistance >= 2.0f)
		_boomerang_adddistance = 2.0f;
}

void PlayerManager::BoomerangSpeedUp()
{
	_boomerang_addspeed += 0.001f;
	if (_boomerang_addspeed >= 0.03f)
		_boomerang_addspeed = 0.03f;
}
#endif