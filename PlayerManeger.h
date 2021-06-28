#pragma once

#include"Boomerang/Boomerang.h"
#include"../LoadCSV/LoadCSV.h"

class PlayerManager {
public:
    PlayerManager::PlayerManager();
    PlayerManager::~PlayerManager();

    bool    Initialize();
    int     Update();
    void    Draw();

    ANIMATIONMODEL GetModel() { return _model; }
    MODEL GetCollision() { return _collision; }
    Boomerang GetBoomerang() { return _boomerang; }
    bool GetShootState() { return _shootstate; }
    int GetAnimState() { return _animstate; }
    void Shoot();
    void ChangeAnimation();
    void OnCollisionEnter();
    Vector3 PlayerGetPosition();
    enum AnimationState
    {
        WAIT,
        RUN,
        STANCE,
        DAMAGE,
        SHOOT,
        ALLTYPE
    };
#ifdef _DEBUG
    int AttackPattern();
    void    ChangeAttackPattern();
    void    BoomerangSizeUp();
    void    BoomerangSpeedUp();
    void    BoomerangDistanceUp();
#endif
private:
    void    KeyboardMove(KeyboardState);
    void    PadMove(GamePadState);
    void    Damage();

    void    PullStatus();

    void    FlyPoint();
    void    MovePlayerRotate(GamePadState);
    void    InvincibleManagement();

    MODEL           _collision;
    ANIMATIONMODEL  _model;

    //!�A�j���[�V����
    int             _animstate = 0;
    int             _oldanimstate = 0;
    float           _animation_count = 0.0f;

    //!�u�[������
    float           _power = 0.0f;
    bool            _shootstate = false;
    Boomerang       _boomerang;

    //!���G
    int   _invincibletime;//���G���Ԃ��J�E���g����
    bool  _invincibleflag;//�_���[�W�����炤��Ԃ��ǂ���
    bool  _invincible_countstart;
    float _blinking;
    bool  _blinking_state;

    //!csv�t�@�C����ǂݍ���ł��̒l������ϐ�
    int   _hp = 1;
    int   _max_invincibletime;
    float _frontdistance;
    float _sidedistance;
    float _playermove;

    Vector3 _player_position;

    //!�v���C���[�̋���
    float   _rotate_direction;
    Vector3 _frontvector;
    float   _rotation;
    float   _atan2_change;

    //!�v���C���[�̃m�b�N�o�b�N���鋗��
    float _knock_back;

    //!�u�[�������̐�����؂�ւ���
    int _attack_pattern;

    //!�u�[�������̐����ω��p�̐��l������
    float _boomerang_addspeed;
    float _boomerang_adddistance;

    std::vector<Vector3> padstick;




    //!�R�[�f�B���O�p
    bool NowMove(GamePadState);
    bool DoStance(GamePadState, KeyboardState);

    bool KeyShot(KeyboardState);
    bool PadShot(GamePadState);
    bool NotShot();
    bool NotStance();

    //struct BoomerangStatus
    //{
    //    Vector3 _start_position;
    //    Vector3 _control_position1;
    //    Vector3 _control_position2;
    //};
    //
    //BoomerangStatus _boomerang_status;
};