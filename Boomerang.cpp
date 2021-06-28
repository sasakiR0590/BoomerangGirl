#include"Boomerang.h"
#include"../../Data/MyAlgorithm.h"
#include"../../SceneManager/SceneManager.h"
Boomerang::Boomerang()
{
	_attack_power = 1;
	_combo = 0;
}

Boomerang::~Boomerang()
{

}

bool Boomerang::Initialize(Vector3 start, Vector3 control1, Vector3 control2, float power, float speed, std::vector<Vector3> stick)
{
	_model = GraphicsDevice.CreateModelFromFile(_T("MODEL/Boomerang/boomerang_v2.X"));
	_model->SetScale(1.0f + power);
	_model->SetPosition(0, 0, 0);
	_model->SetRotation(0, 0, 0);

	if (_model == nullptr)
		return false;

	SimpleShape shape;
	shape.Type = Shape_Box;
	shape.Width  = 1;
	shape.Height = 1;
	shape.Depth  = 1;

	Material mat;
	mat.Diffuse = Color(1.0f, 1.0f, 1.0f);
	mat.Ambient = Color(1.0f, 1.0f, 1.0f);
	mat.Specular = Color(1.0f, 1.0f, 1.0f);
	mat.Power = 1.0f;

	Material _boomerang_material;
	_boomerang_material.Diffuse = Color(1.0f, 1.0f, 1.0f);
	_boomerang_material.Ambient = Color(0.5f, 0.5f, 0.5f);
	_boomerang_material.Specular = Color(1.0f, 1.0f, 1.0f);
	_boomerang_material.Power = 1.0f;

	_collision = GraphicsDevice.CreateModelFromSimpleShape(shape);
	_collision->SetScale(0.1f + power);
	_collision->SetScale(_collision->GetScale() * 5);
	_collision->SetMaterial(mat);
	_model->SetMaterial(_boomerang_material);

	_addspeed    = speed;
	_speed       = 0.0f;
	_rotatespeed = 0.0f;

	_point[0] = start;
	if (stick.size() != 0) {
		_point[1] = control1 + *stick.rbegin();
		_point[2] = control2 + *stick.rbegin();
	}
	else {
		_point[1] = control1;
		_point[2] = control2;
	}
	_angle = Vector3_Zero;
	for (int i = 0; i < AFTERIMAGE; i++)
	{
		_oldpos[i] = Vector3_Zero;
	}
	_count = 0;

	return true;
}

int Boomerang::Update(Vector3 playerposition, GamePadState pad)
{
	_oldpos[_count] = _model->GetPosition();
	_count++;
	_count = IntWrap(_count, 0, AFTERIMAGE);
	if (_speed >= 0.5f && Vector3_Distance(_model->GetPosition(), playerposition) <= 1)
	{
		if (_oldpos[AFTERIMAGE - 1] == _model->GetPosition())
		{
			_model->SetPosition(Vector3_Zero);
			return 1;
		}
		return 0;
	}

	if (pad.Y2 != 0.0f || pad.X3 != 0.0f) {
		_point[1].x += pad.Y2 * 0.000005;
		_point[2].x += pad.Y2 * 0.000005;
		_point[1].z += pad.X3 * -0.000005;
		_point[2].z += pad.X3 * -0.000005;
	}
	_collision->SetPosition(_model->GetPosition() + Vector3(0.0f, 0.0f, 0.0f));
	_model->SetPosition(Move(playerposition));

	PowerManagement();
	return 0;
}

void Boomerang::Draw()
{
	auto pos = _model->GetPosition();
	_model->Draw();
	GraphicsDevice.BeginAlphaBlend();
	GraphicsDevice.SetRenderState(CullMode_None);
	for (int i = 0; i < AFTERIMAGE; i++)
	{
		if (_oldpos[i] != Vector3_Zero && i % 3 == 0)
		{
			_model->SetPosition(_oldpos[i]);
			_model->DrawAlpha(0.1f + i * 0.01f);
		}
	}
	GraphicsDevice.SetRenderState(CullMode_CullCounterClockwiseFace);
	GraphicsDevice.EndAlphaBlend();
	_model->SetPosition(pos);
#ifdef DEBUG
	GraphicsDevice.BeginAlphaBlend();
	_collision->DrawAlpha(0.5f);
#endif
}

Vector3 Boomerang::Move(Vector3 endpos)
{
	_speed += _addspeed;
	_rotatespeed += 50.0f;
	_model->SetRotation(0.0f, _rotatespeed, 0.0f);
	_collision->SetRotation(0.0f, _rotatespeed, 0.0f);
	Vector3 _bezier = Vector3_Bezier(_point[0], _point[1], _point[2], endpos, _speed);

	return _bezier;
}

void Boomerang::PowerManagement() {
	_combo = SceneManager::Instance().GetCombo();
	if (_combo >= 0 && _combo < 100)
		_attack_power = 1;

	else if (_combo >= 100 && _combo < 200)
		_attack_power = 2;

	else if (_combo >= 200 && _combo < 300)
		_attack_power = 3;

	else if (_combo >= 300)
		_attack_power = 4;
}

int Boomerang::PowerCheck()
{
	return _attack_power;
}