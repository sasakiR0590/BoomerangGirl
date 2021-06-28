#pragma once

#include"../../../ESGLib.h"

class Boomerang {
public:
    Boomerang::Boomerang();
    Boomerang::~Boomerang();

    bool    Initialize(Vector3 start, Vector3 control1, Vector3 control2, float power, float speed,std::vector<Vector3> stick);
    int     Update(Vector3 playerposition,GamePadState pad);
    void    Draw();

    MODEL GetCollision() { return _collision; }

    int PowerCheck();

private:
    Vector3 Move(Vector3 endpos);
    void PowerManagement();

    Vector3 _position;
    Vector3 _angle;
    enum MyEnum
    {
        AFTERIMAGE = 15
    };
    Vector3 _oldpos[AFTERIMAGE];
    int _count = 0;
    //! �x�W�G�p�|�C���g
    Vector3 _point[4];

    MODEL   _collision;
    MODEL   _model;

    //!�X�s�[�h
    float   _speed;
    float   _addspeed;
    float   _rotatespeed;

    //�R���{
    int _combo;

    //�U����
    int _attack_power;
};